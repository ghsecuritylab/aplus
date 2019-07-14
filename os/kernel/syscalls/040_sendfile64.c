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
 * Name:        sendfile64
 * Description: transfer data between file descriptors
 * URL:         http://man7.org/linux/man-pages/man2/sendfile64.2.html
 *
 * Input Parameters:
 *  0: 0x28
 *  1: int out_fd
 *  2: int in_fd
 *  3: loff_t __user * offset
 *  4: size_t count
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(40, sendfile64,
long sys_sendfile64 (int out_fd, int in_fd, off_t __user * offset, size_t count) {
    return -ENOSYS;
});
