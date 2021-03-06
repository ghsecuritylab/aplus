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
 * Name:        wait4
 * Description: wait for process to change state, BSD style
 * URL:         http://man7.org/linux/man-pages/man2/wait4.2.html
 *
 * Input Parameters:
 *  0: 0x3d
 *  1: pid_t pid
 *  2: int __user * stat_addr
 *  3: int options
 *  4: struct rusage __user * ru
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

struct rusage;

SYSCALL(61, wait4,
long sys_wait4 (pid_t pid, int __user * stat_addr, int options, struct rusage __user * ru) {
    return -ENOSYS;
});
