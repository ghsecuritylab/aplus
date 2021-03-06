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
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/wait.h>

#include <aplus/base.h>
#include <aplus/utils/list.h>

#include "sh.h"

static struct termios ios;

void sh_reset_tty() {
    pid_t pgrp = getpgrp();
    ioctl(STDIN_FILENO, TIOCSPGRP, &pgrp);
    ioctl(STDIN_FILENO, TCGETA, &ios);
    ios.c_lflag &= ~ISIG;
    ioctl(STDIN_FILENO, TCSETA, &ios);
}

void sh_prepare_tty(pid_t pgrp) {
    ioctl(STDIN_FILENO, TIOCSPGRP, &pgrp);
    ioctl(STDIN_FILENO, TCGETA, &ios);
    ios.c_lflag |= ISIG;
    ioctl(STDIN_FILENO, TCSETA, &ios);
}

static int run(int argc, char** argv, int in, int out) {
    int i;
    for(i = 0; sh_commands[i].cmd; i++) {
        if(strcmp(sh_commands[i].cmd, argv[0]) != 0)
            continue;

        /*
        int old_in = dup(STDIN_FILENO);
        int old_out = dup(STDOUT_FILENO);
        int old_err = dup(STDERR_FILENO);

        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);
        dup2(out, STDERR_FILENO);
        */

        int e = sh_commands[i].fn(argc, argv);

        /*
        dup2(old_in, STDIN_FILENO);
        dup2(old_out, STDOUT_FILENO);
        dup2(old_err, STDERR_FILENO);

        close(old_in);
        close(old_out);
        close(old_err);
        */
        
        return e;
    }

    pid_t e = fork();
    switch(e) {
        case -1:
            perror("sh: fork()");
            return -1;
        
        case 0: {
            setpgrp();
            sh_prepare_tty(getpgrp());

            //dup2(in, STDIN_FILENO);
            //dup2(out, STDOUT_FILENO);
            //dup2(out, STDERR_FILENO);

            execvp(argv[0], argv);
            perror(argv[0]);
            exit(100);
        }

        default: {
            int err, r;
            do {
                err = waitpid(e, &r, 0);
            } while((err == -1 && errno == EINTR));
            
            
            if(err != -1 && WIFSTOPPED(r))
                sh_jobs_new(err, argv[0], 1);


            sh_reset_tty();
            return r;
        }
    }

    return -1;
}




int sh_exec(char* command) {
    if(!command || strlen(command) == 0)
        return 0;


    static list(char*, args);


    char* p;
    char varbuf[BUFSIZ];
    char garbage[BUFSIZ];


    #define clear_args() {  \
        list_each(args, v)  \
            free(v);        \
        list_clear(args);   \
    }


    while(isblank(*command))
        command++;



    memset(varbuf, 0, sizeof(varbuf));
    memset(garbage, 0, sizeof(garbage));

    do {
        switch(*command) {
            case '\"':
            case '\'':
                if(!(p = strchr(command + 1, *command))) {
                    fprintf(stderr, "sh: syntax-error: expected %c at end of line\n", *command);
                    return -1;
                }

                strncat(garbage, command + 1, p - command - 1);
                command = p + 1;
                break;
            case '$':
                command++;
                while(isalnum(*command))
                    strncat(varbuf, command++, 1);

                if(getenv(varbuf))
                    strcat(garbage, getenv(varbuf));

                memset(varbuf, 0, sizeof(varbuf));
                break;
            case '*':
                *command = '\032';
            default:
                strncat(garbage, command++, 1);
                break;
        }
    } while(*command);


    for(p = strtok(garbage, " "); p; p = strtok(NULL, " ")) {
        char* s;
        if(!(s = strchr(p, '\032')))
            list_push(args, strdup(p));
        else {
            do {
                if(*s == '\032')
                    *s = '*';
            } while(*++s);

            glob_t gl;
            glob(p, GLOB_NOSORT, NULL, &gl);

            if(!gl.gl_pathc)
                list_push(args, strdup(p));
            else {
                int i;
                for(i = 0; i < gl.gl_pathc; i++)
                    list_push(args, strdup(gl.gl_pathv[i]));
            } 

            globfree(&gl);
        }
    }
    


    char** argv = (char**) calloc(sizeof(char*), list_length(args) + 1);
    if(!argv) {
        fprintf(stderr, "sh: out of memory!");
        return -1;
    }

    int argc = 0;
    list_each(args, v) {
        argv[argc] = (char*) calloc(1, strlen(v) + 1);
        if(!argv[argc]) {
            fprintf(stderr, "sh: out of memory!");
            return -1;
        }

        strcpy(argv[argc++], v);
    }
    

    int e = run(argc, argv, STDIN_FILENO, STDOUT_FILENO);



    while(argc >= 0)
        free(argv[argc--]);
    
    free(argv);
    clear_args();

    return e;
}