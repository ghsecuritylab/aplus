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
 * Name:        nanosleep
 * Description: high-resolution sleep
 * URL:         http://man7.org/linux/man-pages/man2/nanosleep.2.html
 *
 * Input Parameters:
 *  0: 0x23
 *  1: struct timespec __user * rqtp
 *  2: struct timespec __user * rmtp
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(35, nanosleep,
long sys_nanosleep (struct timespec __user * rqtp, struct timespec __user * rmtp) {
    return -ENOSYS;
});
