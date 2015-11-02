#include <xdev.h>
#include <xdev/debug.h>
#include <xdev/module.h>
#include <xdev/vfs.h>
#include <xdev/ipc.h>
#include <xdev/intr.h>
#include <xdev/timer.h>
#include <xdev/mm.h>
#include <libc.h>

MODULE_NAME("i386/atapi");
MODULE_DEPS("");
MODULE_AUTHOR("WareX");
MODULE_LICENSE("GPL");


#if defined(__i386__)
#include <arch/i386/i386.h>
#include <arch/i386/pci.h>
#include "ata.h"

static uintptr_t ata_pci = 0;

struct ata_device {
	int io_base;
	int control;
	int slave;
	ata_identify_t identify;
	
	struct {
		uintptr_t offset;
		uint16_t bytes;
		uint16_t last;
	} *dma_prdt;

	uintptr_t dma_prdt_phys;
	uint8_t* dma_start;
	uintptr_t dma_start_phys;
	uint32_t bar4;

	mutex_t lock;
	uint8_t cache[ATA_SECTOR_SIZE];
};


static struct ata_device ata_primary_master = {
	.io_base = 0x1F0,
	.control = 0x3F6,
	.slave = 0
};

static struct ata_device ata_secondary_master = {
	.io_base = 0x1F0,
	.control = 0x3F6,
	.slave = 1
};
static struct ata_device ata_primary_slave = {
	.io_base = 0x170,
	.control = 0x376,
	.slave = 0
};
static struct ata_device ata_secondary_slave = {
	.io_base = 0x170,
	.control = 0x376,
	.slave = 1
};


static void find_ata_pci(uint32_t device, uint16_t vendorid, uint16_t deviceid, void* arg) {
	if((vendorid == 0x8086) && (deviceid & 0x7010))
		*((uintptr_t*) arg) = device;
}


static uint64_t ata_max_offset(struct ata_device* dev) {
	if(dev->identify.sectors_48)
		return dev->identify.sectors_48 * ATA_SECTOR_SIZE;

	return dev->identify.sectors_28 * ATA_SECTOR_SIZE;
}

static void ata_io_wait(struct ata_device* dev) {
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
}

static int ata_status_wait(struct ata_device* dev, int timeout) {
	int e, i = 0;
	if(timeout > 0)
		while((e = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout))
			i++;
	else
		while((e = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY);

	return e;
}

static int ata_wait(struct ata_device* dev, int advanced) {
	ata_io_wait(dev);
	int e = ata_status_wait(dev, -1);

	if(advanced) {
		e = inb(dev->io_base + ATA_REG_STATUS);

		if(e & ATA_SR_ERR)
			return E_ERR;

		if(e & ATA_SR_DF)
			return E_ERR;

		if(!(e & ATA_SR_DRQ))
			return E_ERR;
	}

	return E_OK;
}


static void ata_soft_reset(struct ata_device* dev) {
	outb(dev->control, 0x04);
	ata_io_wait(dev);
	outb(dev->control, 0x00);
}




static void irq_handler_1(void* unused) {
	(void) unused;

	inb(ata_primary_master.io_base + ATA_REG_STATUS);
}

static void irq_handler_2(void* unused) {
	(void) unused;

	inb(ata_secondary_master.io_base + ATA_REG_STATUS);
}


static void ata_device_init(struct ata_device* dev) {
	outb(dev->io_base + 1, 1);
	outb(dev->control, 0);
	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
	
	ata_io_wait(dev);

	outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	ata_io_wait(dev);
	int e = inb(dev->io_base + ATA_REG_COMMAND);
	ata_wait(dev, 0);



	uint16_t* buf = (uint16_t*) &dev->identify;

	int i;
	for(i = 0; i < 256; i++)
		buf[i] = inw(dev->io_base);
	
	
	char* ptr = (char*) &dev->identify.model;
	for(i = 0; i < 39; i += 2) {
		register char t = ptr[i + 1];
		ptr[i + 1] = ptr[i];
		ptr[i] = t;
	}

#if 0
	kprintf(LOG, "[ATA] Device Name:  %s\n", dev->identify.model);
	kprintf(LOG, "[ATA] Sectors (48): %lld\n", dev->identify.sectors_48);
	kprintf(LOG, "[ATA] Sectors (28): %d\n", dev->identify.sectors_28);
#endif

	dev->dma_prdt = (void*) kvalloc(8, GFP_KERNEL);
	dev->dma_start = (void*) kvalloc(4096, GFP_KERNEL);

	dev->dma_prdt_phys = V2P(dev->dma_prdt);
	dev->dma_start_phys = V2P(dev->dma_start);

	dev->dma_prdt[0].offset = dev->dma_start_phys;
	dev->dma_prdt[0].bytes = ATA_SECTOR_SIZE;
	dev->dma_prdt[0].last = 0x8000;

	
	uint16_t cmd = pci_read_field(ata_pci, PCI_COMMAND, 4);
	if(!(cmd & (1 << 2)))
		pci_write_field(ata_pci, PCI_COMMAND, 4, cmd | (1 << 2));
	
	dev->bar4 = pci_read_field(ata_pci, PCI_BAR4, 4);

	if(dev->bar4 & 1)
		dev->bar4 &= 0xFFFFFFFC;


	memset(dev->cache, 0, ATA_SECTOR_SIZE);
}


static void ata_device_read_sector(struct ata_device* dev, uint32_t lba, void* buf) {
	uint16_t bus = dev->io_base;
	uint8_t slave = dev->slave;

	outb(bus + ATA_REG_CONTROL, 0x02);
	
	ata_wait(dev, 0);
	outb(bus + ATA_REG_HDDEVSEL, 0xE0 | (slave << 4) | (lba & 0x0F000000) >> 24);
	ata_wait(dev, 0);

	outb(bus + ATA_REG_FEATURES, 0x00);
	outb(bus + ATA_REG_SECCOUNT0, 0x01);
	outb(bus + ATA_REG_LBA0, lba & 0xFF);
	outb(bus + ATA_REG_LBA1, (lba & 0xFF00) >> 8);
	outb(bus + ATA_REG_LBA2, (lba & 0xFF0000) >> 16);
	outb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	ata_wait(dev, 0);
	
	insw(bus, buf, ATA_SECTOR_SIZE / 2);
	
	outb(bus + 0x07, ATA_CMD_CACHE_FLUSH);	
	ata_wait(dev, 0);
}


static void ata_device_write_sector(struct ata_device* dev, uint32_t lba, void* buf) {
	uint16_t bus = dev->io_base;
	uint8_t slave = dev->slave;

	outb(bus + ATA_REG_CONTROL, 0x02);
	
	ata_wait(dev, 0);
	outb(bus + ATA_REG_HDDEVSEL, 0xE0 | (slave << 4) | (lba & 0x0F000000) >> 24);
	ata_wait(dev, 0);

	outb(bus + ATA_REG_FEATURES, 0x00);
	outb(bus + ATA_REG_SECCOUNT0, 0x01);
	outb(bus + ATA_REG_LBA0, lba & 0xFF);
	outb(bus + ATA_REG_LBA1, (lba & 0xFF00) >> 8);
	outb(bus + ATA_REG_LBA2, (lba & 0xFF0000) >> 16);
	outb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

	ata_wait(dev, 0);
	
	outsw(bus, buf, ATA_SECTOR_SIZE / 2);
	
	outb(bus + 0x07, ATA_CMD_CACHE_FLUSH);	
	ata_wait(dev, 0);
}


static int ata_read(inode_t* ino, void* buffer, size_t size) {
	if(ino->position > ino->size)
		return 0;

	if(ino->position + size > ino->size)
		size = ino->size - ino->position;

	if(!size)
		return 0;

	struct ata_device* dev = (struct ata_device*) ino->userdata;

	long sb = ino->position / ATA_SECTOR_SIZE;
	long eb = (ino->position + size - 1) / ATA_SECTOR_SIZE;
	off64_t xoff = 0;

	if(ino->position % ATA_SECTOR_SIZE) {
		long p = ATA_SECTOR_SIZE - (ino->position % ATA_SECTOR_SIZE);

		mutex_lock(&dev->lock);
		ata_device_read_sector(dev, sb, dev->cache);

		memcpy(buffer, (void*) ((uintptr_t) dev->cache + ((uintptr_t) ino->position % ATA_SECTOR_SIZE)), p);
		xoff += p;
		sb++;

		mutex_unlock(&dev->lock);
	}

	if(((ino->position + size) % ATA_SECTOR_SIZE) && (sb <= eb)) {
		long p = (ino->position + size) % ATA_SECTOR_SIZE;

		mutex_lock(&dev->lock);
		ata_device_read_sector(dev, eb, dev->cache);

		memcpy((void*) ((uintptr_t) buffer + size - p), dev->cache, p);
		eb--;

		mutex_unlock(&dev->lock);
	}

	while(sb <= eb) {
		ata_device_read_sector(dev, sb, (void*) ((uintptr_t) buffer + (uintptr_t) xoff));
		xoff += ATA_SECTOR_SIZE;
		sb++;
	}

	ino->position += size;
	return size;
}

static int ata_write(inode_t* ino, void* buffer, size_t size) {
	if(ino->position > ino->size)
		return 0;

	if((ino->position + size) > ino->size)
		size = ino->size - ino->position;

	if(!size)
		return 0;


	struct ata_device* dev = (struct ata_device*) ino->userdata;

	long sb = ino->position / ATA_SECTOR_SIZE;
	long eb = (ino->position + size - 1) / ATA_SECTOR_SIZE;
	off64_t xoff = 0;


	if(ino->position % ATA_SECTOR_SIZE) {
		long p = ATA_SECTOR_SIZE - (ino->position % ATA_SECTOR_SIZE);


		mutex_lock(&dev->lock);

		ata_device_read_sector(dev, sb, dev->cache);
		memcpy((void*) ((uintptr_t) dev->cache + ((uintptr_t) ino->position % ATA_SECTOR_SIZE)), buffer, p);
		ata_device_write_sector(dev, sb, dev->cache);

		xoff += p;
		sb++;

		mutex_unlock(&dev->lock);
	}

	if(((ino->position + size) % ATA_SECTOR_SIZE) && (sb <= eb)) {
		long p = (ino->position + size) % ATA_SECTOR_SIZE;


		mutex_lock(&dev->lock);

		ata_device_read_sector(dev, eb, dev->cache);
		memcpy(dev->cache, (void*) ((uintptr_t) buffer + size - p), p);
		ata_device_write_sector(dev, eb, dev->cache);

		eb--;
		mutex_unlock(&dev->lock);
	}

	while(sb <= eb) {
		ata_device_write_sector(dev, sb, (void*) ((uintptr_t) buffer + (uintptr_t) xoff));
			
		xoff += ATA_SECTOR_SIZE;
		sb++;
	}

	ino->position += size;
	return size;
}



static int ata_device_detect(struct ata_device* dev) {
	ata_soft_reset(dev);
	ata_io_wait(dev);

	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);

	ata_io_wait(dev);
	ata_status_wait(dev, 10000);

	uint8_t cl = inb(dev->io_base + ATA_REG_LBA1);
	uint8_t ch = inb(dev->io_base + ATA_REG_LBA2);
	uint8_t c = 0;

	if(cl == 0xFF && ch == 0xFF)
		return 0;

	if(
		(cl == 0x00 && ch == 0x00) ||
		(cl == 0x3C && ch == 0xC3)
	) {

		mutex_init(&dev->lock, MTX_KIND_DEFAULT);
		ata_device_init(dev);

		inode_t* inode = vfs_mkdev("hd", c++, S_IFBLK | 0666);
		inode->size = ata_max_offset(dev);
		inode->read = ata_read;
		inode->write = ata_write;
		inode->userdata = (void*) dev;
	}
}

int init(void) {
	
	pci_scan(&find_ata_pci, -1, &ata_pci);

	irq_enable(ATA_IRQ_PRIMARY, irq_handler_1);
	irq_enable(ATA_IRQ_SECONDARY, irq_handler_2);

	ata_device_detect(&ata_primary_master);
	ata_device_detect(&ata_secondary_master);
	ata_device_detect(&ata_primary_slave);
	ata_device_detect(&ata_secondary_slave);

	return E_OK;
}

#else

int init(void) {
	return E_OK;
}

#endif


int dnit(void) {
	return E_OK;
}