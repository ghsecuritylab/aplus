/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2019 Antonino Natale
 * 
 * 
 * This file is part of aPlus.
 * 
 * aPlus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aPlus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _APLUS_VFS_H
#define _APLUS_VFS_H

#include <aplus.h>
#include <aplus/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/dirent.h>

typedef struct inode inode_t;
typedef struct vfs_cache vfs_cache_t;



struct vfs_cache {

    size_t capacity;
    size_t count;

    spinlock_t lock;

    inode_t** inodes;
    ktime_t* times;

};


struct inode_ops {

    int (*open)(inode_t*);
    int (*close)(inode_t*);
    int (*read)(inode_t*, void*, off_t, size_t);
    int (*write)(inode_t*, const void*, off_t, size_t);

    inode_t* (*finddir) (inode_t*, const char*);
    inode_t* (*mknod) (inode_t*, const char*, mode_t);
    int (*readdir) (inode_t*, struct dirent*, off_t, size_t);
    int (*unlink) (inode_t*, const char*);

    int (*ioctl) (inode_t*, int, void*);
    int (*fsync) (inode_t*);

};


struct inode {
    char name[MAXNAMLEN];

    struct stat st;
    struct inode_ops ops;
    
    spinlock_t lock;

    union {
        struct {
            int flags;
            const char* type;
            inode_t* dev;

            struct statvfs st;
            struct inode_ops ops;
            struct vfs_cache* cache;
        } mount;

        struct inode* link;
    };



    void* userdata;
    void* fsinfo;

    struct inode* parent;
    struct inode* root;
};



void vfs_init(void);

// os/kernel/fs/vfs.c
int vfs_mount(inode_t* dev, inode_t* dir, const char __user * fs, int flags, const char __user * args);
int vfs_open(inode_t* inode);
int vfs_close(inode_t* inode);
int vfs_read(inode_t* inode, void* buf, off_t off, size_t size);
int vfs_write(inode_t* inode, const void* buf, off_t off, size_t size);
int vfs_readdir(inode_t* inode, struct dirent* ent, off_t off, size_t count);
int vfs_unlink(inode_t* inode, const char* name);
int vfs_ioctl(inode_t* inode, int req, void* arg);
int vfs_fsync(inode_t* inode);

inode_t* vfs_finddir(inode_t* inode, const char __user * name);
inode_t* vfs_mknod(inode_t* inode, const char __user * name, mode_t mode);


// os/kernel/fs/cache.c
void vfs_cache_create(vfs_cache_t**, int capacity);
void vfs_cache_destroy(vfs_cache_t**);
inode_t* vfs_cache_get_inode(vfs_cache_t*, ino_t);



extern inode_t* vfs_root;
extern inode_t* vfs_dev;


#define TMPFS_ID                    0xDEAD1000
#define EXT2_ID                     0xDEAD1001
#define VFAT_ID                     0xDEAD1002

#endif