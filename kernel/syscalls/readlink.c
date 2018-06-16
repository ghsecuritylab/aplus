/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2018 Antonino Natale
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


#include <aplus.h>
#include <aplus/vfs.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/syscall.h>
#include <aplus/network.h>
#include <aplus/debug.h>
#include <libc.h>

SYSCALL(85, readlink,
ssize_t sys_readlink(const char* filename, char* pt, size_t size) {
    if(unlikely(!filename || !pt || !size)) {
        errno = EINVAL;
        return -1;
    }
    
    
    int fd = sys_open(filename, O_RDONLY | O_NOFOLLOW, 0);
    if(fd < 0)
        return -1;

    inode_t* inode = current_task->fd[fd].inode;
    sys_close(fd);
    
    
    if(unlikely(!inode)) {
        errno = EBADF;
        return -1;
    }

    if(unlikely(!(S_ISLNK(inode->mode)) || !inode->link)) {
        errno = EINVAL;
        return -1;
    }



    int j = 0;
    inode_t* tmp = inode->link;


    #define ADD(x) {            \
        if(j > size) {          \
            errno = ERANGE;     \
            return -1;          \
        }                       \
                                \
        pt[j++] = (char) (x);   \
        pt[j] = '\0';           \
    }


    if(tmp == current_task->root) {
        ADD('/');
        return 1;
    }

    for(; tmp; tmp = tmp->parent) {
        if(tmp == current_task->root)
            break;

        for(int i = strlen(tmp->name); i > 0; i--)
            ADD(tmp->name[i - 1]);
        
        ADD('/');
    }


    char* end = pt;
    char* begin = pt;

    for(int i = 0; i < strlen(pt) - 1; i++)
        end++;

    for(int i = 0; i < strlen(pt) / 2; i++) {
        *begin ^= *end;
        *end ^= *begin;
        *begin ^= *end;

        begin++;
        end--;
    }

    return strlen(pt);
});
