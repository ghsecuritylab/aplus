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
 * Name:        perf_event_open
 * Description: set up performance monitoring
 * URL:         http://man7.org/linux/man-pages/man2/perf_event_open.2.html
 *
 * Input Parameters:
 *  0: 0x12a
 *  1: struct perf_event_attr __user * attr_uptr
 *  2: pid_t pid
 *  3: int cpu
 *  4: int group_fd
 *  5: unsigned long flags
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

struct perf_event_attr;

SYSCALL(298, perf_event_open,
long sys_perf_event_open (struct perf_event_attr __user * attr_uptr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return -ENOSYS;
});
