
//
//  pthread_sched.c
//
//  Author:
//       Antonio Natale <inferdevil97@gmail.com>
//
//  Copyright (c) 2014 WareX
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdint.h>
#include <pthread.h>

#include "pthread_internal.h"

PUBLIC int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param) {
	errno = ENOSYS;
	return 1;
}

PUBLIC int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param) {
	errno = ENOSYS;
	return 1;
}

PUBLIC int pthread_setconcurrency(int level) {
	errno = ENOSYS;
	return 1;
}

PUBLIC int pthread_getconcurrency(void) {
	errno = ENOSYS;
	return 1;
}