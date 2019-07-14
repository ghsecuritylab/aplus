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
#include <aplus/smp.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        access
 * Description: check user's permissions for a file
 * URL:         http://man7.org/linux/man-pages/man2/access.2.html
 *
 * Input Parameters:
 *  0: 0x15
 *  1: const char __user * filename
 *  2: int mode
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(21, access,
long sys_access (const char __user * filename, int mode) {
    
    if(unlikely(!ptr_check(filename, R_OK)))
        return -EFAULT;


    int e;

    struct stat st;
    if((e = sys_newstat(filename, &st)) < 0)
        return e;



    if(st.st_uid == current_task->uid)
        if(!(
            (mode & R_OK ? st.st_mode & S_IRUSR : 1) &&
            (mode & W_OK ? st.st_mode & S_IWUSR : 1) &&
            (mode & X_OK ? st.st_mode & S_IXUSR : 1) &&
        )) return -EACCES;

    else if(st.st_gid == current_task->gid)
        if(!(
            (mode & R_OK ? st.st_mode & S_IRGRP : 1) &&
            (mode & W_OK ? st.st_mode & S_IWGRP : 1) &&
            (mode & X_OK ? st.st_mode & S_IXGRP : 1) &&
        )) return -EACCES;
    
    else
        if(!(
            (mode & R_OK ? st.st_mode & S_IROTH : 1) &&
            (mode & W_OK ? st.st_mode & S_IWOTH : 1) &&
            (mode & X_OK ? st.st_mode & S_IXOTH : 1) &&
        )) return -EACCES;


    return 0;

});
