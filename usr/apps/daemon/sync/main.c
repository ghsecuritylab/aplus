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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <aplus/base.h>
#include <aplus/sysconfig.h>

static void show_usage(int argc, char** argv) {
    printf(
        "Use: sync\n"
        "Synchronize cached writes to persistent storage.\n\n"
        "       --daemon                run as daemon [syncd]\n"
        "       --help                  show this help\n"
        "       --version               print version info and exit\n"
    );
    
    exit(0);
}

static void show_version(int argc, char** argv) {
    printf(
        "%s (aPlus sysutils) 0.1\n"
        "Copyright (c) %s Antonino Natale.\n"
        "Built with gcc %s (%s)\n",
        
        argv[0], COMMIT, __DATE__ + 7, __VERSION__, __TIMESTAMP__
    );
    
    exit(0);
}


static void atsig_handler(int sig) {
    fprintf(stderr, "syncd: service stopped\n");
    exit(sig);
}



int main(int argc, char** argv) {
    
    static struct option long_options[] = {
        { "daemon", no_argument, NULL, 'd'},
        { "help", no_argument, NULL, 'h'},
        { "version", no_argument, NULL, 'v'},
        { NULL, 0, NULL, 0 }
    };
    
    
    int daemon = 0;
    int c, idx;
    while((c = getopt_long(argc, argv, "d", long_options, &idx)) != -1) {
        switch(c) {
            case 'd':
                daemon++;
                break;
            case 'v':
                show_version(argc, argv);
                break;
            case 'h':
            case '?':
                show_usage(argc, argv);
                break;
            default:
                abort();
        }
    }
  
    nice(19);
    signal(SIGTERM, atsig_handler);
    signal(SIGQUIT, atsig_handler);

    if(!daemon)
        sync();
    else {
        if(strcmp((const char*) sysconfig("daemons.syncd.enabled", "false"), "true") != 0) {
            fprintf(stderr, "syncd: daemon disabled by /etc/config\n");
            return 0;
        }

        if(strcmp(argv[0], "[syncd]") != 0)
            execl("/proc/self/exe", "[syncd]", "--daemon", NULL);
        
        setsid();
        chdir("/");

        int fd = open("/dev/log", O_WRONLY);
        if(fd >= 0) {
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
        }


        int s = (int) sysconfig("daemons.syncd.timeout", 10);
        fprintf(stderr, "syncd: running as daemon every %d seconds\n", s);

        for(;; sleep(s))
            sync();
    }

    return 0;
}