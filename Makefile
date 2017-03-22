.PHONY: all clean
.EXPORT_ALL_VARIABLES:

ARCH		:= i386
TARGET		:= i686-cross



include build/Makefile.tools
include build/Makefile.sources
include build/Makefile.flags


all:					\
	$(KERNEL_OUTPUT)	\
	KERNEL_MODULES		\
	$(KERNEL_ISO)
	@$(VMM)

$(KERNEL_OUTPUT): $(KERNEL_OBJECTS) LIBRARIES
	@echo "  LD     " $@
	@$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJECTS) $(LIBS)
	@echo "  OBJCPY " $(KERNEL_SYM)
	@$(OBJCPY) --only-keep-debug $@ $(KERNEL_SYM)
	@echo "  ZIP    " $@.gz
	@$(ZIP) $@
	@echo "  MV     " $@
	@$(MV) $@.gz $@

KERNEL_MODULES: LIBRARIES
	@$(foreach dir, $(KERNEL_MODULES_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) CC=$(CC);)
	@echo "multiboot /$(KERNEL_NAME)" > bin/boot/grub/grub.cfg
	@$(foreach mod, $(KERNEL_MODULES), echo module /$(subst bin/,,$(mod)) >> bin/boot/grub/grub.cfg; )
	@echo "boot" >> bin/boot/grub/grub.cfg
	
APPS: LIBRARIES
	@$(foreach dir, $(APPS_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) CC=$(CC) CXX=$(CXX) AR=$(AR);)

LIBRARIES:
	@$(foreach dir, $(LIBS_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) CC=$(CC) CXX=$(CXX) AR=$(AR);)

$(KERNEL_ISO): $(KERNEL_OUTPUT) KERNEL_MODULES APPS LIBRARIES
	@echo "  ISO    " $@
	@grub-mkrescue -o $@ bin

.c.o:
	@echo "  CC     " $@
	@$(CC) $(CFLAGS) -o $@ $<

.s.o:
	@echo "  AS     " $@
	@$(AS) $(AFLAGS) -o $@ $<

.asm.o:
	@echo "  ASM    " $@
	@$(ASM) $(NFLAGS) -o $@ $<
	
clean_modules:
	@$(foreach dir, $(KERNEL_MODULES_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) clean;)

clean_apps:
	@$(foreach dir, $(APPS_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) clean;)

clean_libs:
	@$(foreach dir, $(LIBS_MAKE), cd $(PWD)/$(dir) && $(MAKE) -s ROOT=$(PWD) clean;)
	
clean_kernel:
	@$(RM) $(KERNEL_OBJECTS) $(KERNEL_ISO) $(KERNEL_OUTPUT) $(KERNEL_SYM)
	
clean: clean_modules clean_apps clean_kernel clean_libs
	@$(RM) -r *.o *.map

debug:
	@$(MAKE) -s DEBUG=yes

