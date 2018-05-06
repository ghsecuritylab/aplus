#include <aplus.h>
#include <aplus/vfs.h>
#include <aplus/task.h>
#include <aplus/mm.h>
#include <aplus/ipc.h>
#include <aplus/timer.h>
#include <aplus/debug.h>
#include <libc.h>


static inode_t __vfs_root;
inode_t* vfs_root = &__vfs_root;

inode_t* devfs = NULL;
list(fsys_t*, fsys_queue);
 
int vfs_open(struct inode* inode) {
    if(likely(inode->open))
        return inode->open(inode);

    return 0;
}

int vfs_close(struct inode* inode) {
    if(likely(inode->close))
        return inode->close(inode);

    return 0;
}

int vfs_read(struct inode* inode, void* ptr, off_t pos, size_t len) {
    if(likely(inode->read))
#if CONFIG_IOSCHED
        return iosched_read(inode, ptr, pos, len);
#else
        return inode->read(inode, ptr, pos, len);
#endif

    errno = ENOSYS;
    return 0;
}

int vfs_write(struct inode* inode, void* ptr, off_t pos, size_t len) {
    if(likely(inode->write))
    #if CONFIG_IOSCHED
        return iosched_write(inode, ptr, pos, len);
    #else
        return inode->write(inode, ptr, pos, len);
    #endif

    errno = ENOSYS;
    return 0;
}



struct inode* vfs_finddir(struct inode* inode, char* name) {

    if(name[0] == '.' && name[1] == '\0')
        return inode;

    if(name[0] == '.' && name[1] == '.' && name[2] == '\0') {
        if(likely(inode != current_task->root))
            return inode->parent;
        
        return NULL;
    }


    if(inode->childs) {
        struct inode_childs* tmp;
        for(tmp = inode->childs; tmp; tmp = tmp->next)
            if(strcmp(tmp->inode->name, name) == 0)
                return tmp->inode;
    }

    if(unlikely(!inode->finddir))
        return NULL;



    inode_t* child = inode->finddir(inode, name);
    if(unlikely(!child))
        return NULL;




    struct inode_childs* cx = (struct inode_childs*) kmalloc(sizeof(struct inode_childs), GFP_KERNEL);
    cx->inode = child;
    cx->next = inode->childs;
    inode->childs = cx;

    return child;
}

struct inode* vfs_mknod(struct inode* inode, char* name, mode_t mode) {

    if(unlikely(vfs_finddir(inode, name))) {
        errno = EEXIST;
        return NULL;
    }

    inode_t* child;

    if(likely(inode->mknod)) {
        if(likely(child = inode->mknod(inode, name, mode))) {
            struct inode_childs* cx = (struct inode_childs*) kmalloc(sizeof(struct inode_childs), GFP_KERNEL);
            cx->inode = child;
            cx->next = inode->childs;
            inode->childs = cx;

            return child;
        }
        
        return NULL;
    }

    child = (inode_t*) kmalloc(sizeof(inode_t), GFP_KERNEL);
    memset(child, 0, sizeof(inode_t));

    child->name = strdup(name);
    child->ino = vfs_inode();

    child->mode = mode & ~current_task->umask;

    child->dev =
    child->rdev =
    child->nlink = 0;
    child->uid = current_task->uid;
    child->gid = current_task->gid;
    child->size = 0;

    child->atime = 
    child->ctime = 
    child->mtime = timer_gettimestamp();
    
    child->parent = inode;
    child->mtinfo = inode->mtinfo;
    child->link = NULL;

    child->childs = NULL;

    child->read = NULL;
    child->write = NULL;
    child->finddir = NULL;
    child->mknod = NULL;
    child->rename = NULL;
    child->unlink = NULL;
    child->chown = NULL;
    child->chmod = NULL;
    child->ioctl = NULL;
    child->open = NULL;
    child->close = NULL;


    struct inode_childs* cx = (struct inode_childs*) kmalloc(sizeof(struct inode_childs), GFP_KERNEL);
    cx->inode = child;
    cx->next = inode->childs;
    inode->childs = cx;

    return child;
}



int vfs_unlink(struct inode* inode, char* name) {
    int r = E_OK;    
    if(likely(inode->unlink))
        r = inode->unlink(inode, name);
    
    if(unlikely(r == E_ERR))
        return r;
        
    if(unlikely(!inode->childs))
        return r;

    struct inode_childs* t0 = inode->childs->next;
    struct inode_childs* t1 = inode->childs;
    for(; t0; t1 = t0, t0 = t0->next) {
        if(strcmp(t0->inode->name, name) != 0)
            continue;

        t1->next = t0->next;

        kfree((void*) t0->inode->name);
        kfree((void*) t0->inode);
        kfree((void*) t0);

        return 0;
    }

    if(strcmp(inode->childs->inode->name, name) != 0)
        return -1;

    t0 = inode->childs;    
    inode->childs = inode->childs->next;
    
    kfree(t0);
    return 0;
}

int vfs_rename(struct inode* inode, char* newname) {
    if(likely(inode->rename))
        if(unlikely(inode->rename(inode, newname) != E_OK))
            return -1;


    inode->name = strdup(newname);
    inode->dirty++;

    return 0;
}

int vfs_chown(struct inode* inode, uid_t owner, gid_t group) {
    if(likely(inode->chown))
        if(unlikely(inode->chown(inode, owner, group) != E_OK))
            return -1;


    inode->uid = owner;
    inode->gid = group;
    inode->dirty++;

    return 0;
}

int vfs_chmod(struct inode* inode, mode_t mode) {
    if(likely(inode->chmod))
        if(unlikely(inode->chmod(inode, mode) != E_OK))
            return -1;


    inode->mode = mode;
    inode->dirty++;

    return 0;
}


int vfs_ioctl(struct inode* inode, int req, void* buf) {
    if(likely(inode->ioctl))
        return inode->ioctl(inode, req, buf);

    return -1;
}


int vfs_fsync(struct inode* inode) {
    if(likely(inode->fsync))
        if(inode->fsync(inode) != E_OK)
            return -1;
    

    inode->dirty = 0;
    return 0;
}


fsys_t* vfs_fsys_find(const char* name) {
    list_each(fsys_queue, tmp)
        if(strcmp(tmp->name, name) == 0)
            return tmp;

    return NULL;
}

int vfs_fsys_register(const char* name, int (*mount) (struct inode*, struct inode*, struct mountinfo*)) {
    if(vfs_fsys_find(name))
        return -1;

    fsys_t* fsys = (fsys_t*) kmalloc(sizeof(fsys_t), GFP_KERNEL);
    fsys->name = strdup(name);
    fsys->mount = mount;
    list_push(fsys_queue, fsys);

    return 0;
}


ino64_t vfs_inode() {
    static ino64_t ino = UINT32_MAX;
    return ino++;
}

int vfs_init(void) {
    vfs_root->name = "/";
    vfs_root->ino = 0;

    vfs_root->mode = 0777 | S_IFDIR;

    vfs_root->dev =
    vfs_root->rdev =
    vfs_root->nlink =
    vfs_root->uid =
    vfs_root->gid = 
    vfs_root->size = 0;

    vfs_root->atime = 
    vfs_root->ctime = 
    vfs_root->mtime = timer_gettimestamp();
    
    vfs_root->parent =
    vfs_root->link = NULL;

    vfs_root->childs = NULL;

    vfs_root->read = NULL;
    vfs_root->write = NULL;
    vfs_root->finddir = NULL;
    vfs_root->mknod = NULL;
    vfs_root->rename = NULL;
    vfs_root->unlink = NULL;
    vfs_root->chown = NULL;
    vfs_root->chmod = NULL;
    vfs_root->ioctl = NULL;
    vfs_root->open = NULL;
    vfs_root->close = NULL;
    vfs_root->fsync = NULL;

    
    kernel_task->cwd =
    kernel_task->root = vfs_root;


    devfs = vfs_mknod(vfs_root, "dev", S_IFDIR | 0666);
    return 0;
}


inode_t* vfs_mkdev(const char* name, dev_t rdev, mode_t mode) {
    char buf[strlen(name) + 2];
    if(rdev != -1)
        sprintf(buf, "%s%d", name, rdev);
    else
        strcpy(buf, name);


    return vfs_mknod(devfs, buf, mode);
} 


EXPORT(vfs_open);
EXPORT(vfs_close);
EXPORT(vfs_read);
EXPORT(vfs_write);
EXPORT(vfs_finddir);
EXPORT(vfs_mknod);
EXPORT(vfs_rename);
EXPORT(vfs_unlink);
EXPORT(vfs_chown);
EXPORT(vfs_chmod);
EXPORT(vfs_ioctl);
EXPORT(vfs_fsys_register);
EXPORT(vfs_fsys_find);
EXPORT(vfs_inode);
EXPORT(vfs_mkdev);

EXPORT(fsys_queue);
EXPORT(devfs);