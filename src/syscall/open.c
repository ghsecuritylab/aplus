#include <aplus.h>
#include <aplus/syscall.h>
#include <aplus/fs.h>
#include <aplus/task.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <dirent.h>

#include <stdio.h>

extern task_t* current_task;
extern inode_t* vfs_root;


static char* dupstr(char* s) {
	char* p = (char*) kmalloc(strlen(s) + 1);
	strcpy(p, s);

	return p;
}


static inode_t* ino_open(char* filename, int flags, mode_t mode) {
	inode_t* cwd = NULL;

	if(filename[0] == '/')
		cwd = vfs_root;
	else
	 	cwd = current_task->cwd;
	
	if(!cwd) {
		if(!vfs_root) {
			kprintf("sys_open: no root found for cwd.");
		
			errno = ENOENT;
			return NULL;
		}
		
		cwd = vfs_root;
	}
	
	
	if(filename[0] == '/')
		filename++;
		
	if(filename[0] == 0)
		return cwd;
		
	
	char* s = filename;
	char* p = s;
	
	while(*s) {
		if((p = strchr(s, '/'))) {
			*p++ = 0;
		
			cwd = (inode_t*) fs_finddir(cwd, s);
			if(!cwd) {
				errno = ENOENT;
				return NULL;
			}

			while(S_ISLNK(cwd->mode))
				if(cwd->link)
					cwd = cwd->link;
				else
					break;

			s = p;
		} else
			break;
	}

	if(*s == 0) {
		errno = ENOENT;
		return NULL;
	}

	inode_t* ent = (inode_t*) fs_finddir(cwd, s);

	if(flags & O_EXCL) {
		if(ent) {
			errno = EEXIST;
			return NULL;
		}
	}


	if(flags & O_CREAT)
		if(!ent)
			ent = (inode_t*) fs_creat(cwd, s, mode);
		

	if(!ent) {
		errno = ENOENT;
		return NULL;
	}

	if(!(flags & O_NOFOLLOW)) {
		while(S_ISLNK(ent->mode)) {
			if(ent == ent->link) {
				errno = ELOOP;
				return NULL;
			}			

			if(ent->link)
				ent = ent->link;
			else
				break;
		}
	}

	if(flags & O_DIRECTORY) {
		if(!(S_ISDIR(ent->mode))) {
			errno = ENOTDIR;
			return NULL;
		}
	}

	return ent;
}

int sys_open(char* filename, int flags, mode_t mode) {
	if(!current_task)
		return -1;

	char* p = dupstr(filename);
	inode_t* ino = ino_open(p, flags, mode);
	kfree(p);

	if(!ino)
		return -1;
	
	
	return schedule_append_fd(current_task, ino);
}

SYSCALL(sys_open, 10);
