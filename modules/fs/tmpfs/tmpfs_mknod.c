#include <xdev.h>
#include <xdev/debug.h>
#include <xdev/vfs.h>
#include <xdev/task.h>
#include <xdev/mm.h>
#include <xdev/timer.h>
#include <libc.h>

#include "tmpfs.h"


struct inode* tmpfs_mknod(struct inode* inode, char* name, mode_t mode) {
	inode_t* child = (inode_t*) kmalloc(sizeof(inode_t), GFP_KERNEL);
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
	child->mtime = timer_gettime();
	
	child->parent = inode;
	child->link = NULL;

	child->childs = NULL;

	if(S_ISDIR(mode)) {
		child->mknod = tmpfs_mknod;
		child->unlink = tmpfs_unlink;
	} else {
		child->read = tmpfs_read;
		child->write = tmpfs_write;
	}


	child->finddir = NULL;
	child->rename = NULL;
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