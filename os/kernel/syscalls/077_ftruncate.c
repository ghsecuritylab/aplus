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


#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/smp.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        ftruncate
 * Description: truncate a file to a specified length
 * URL:         http://man7.org/linux/man-pages/man2/ftruncate.2.html
 *
 * Input Parameters:
 *  0: 0x4d
 *  1: unsigned int fd
 *  2: unsigned long length
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(77, ftruncate,
long sys_ftruncate (unsigned int fd, unsigned long length) {

    if(unlikely(fd < 0))
        return -EBADF;

    if(unlikely(fd > TASK_NFD)) /* TODO: Add Network Support */
        return -EBADF;

    if(unlikely(!current_task->fd[fd].inode))
        return -EBADF;


    int e = 0;

    __lock(&current_task->fd[fd].lock, {
        e = vfs_truncate(current_task->fd[fd].inode, length);
    });


    if(e < 0)
        return -errno;
    
    return e;

});
