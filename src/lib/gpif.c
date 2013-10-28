/*
* Copyright (c) 2008, Björn Rehm (bjoern@shugaa.de)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#include "gpif.h"

/* ######################################################################### */
/*                            TODO / Notes                                   */
/* ######################################################################### */

/* ######################################################################### */
/*                            Types & Defines                                */
/* ######################################################################### */

/* ######################################################################### */
/*                           Private interface (Module)                      */
/* ######################################################################### */

static ssize_t prv_gpif_read(int fd, void* vptr, ssize_t len);
static ssize_t prv_gpif_write(int fd, const void* ptr, size_t len);

/* ######################################################################### */
/*                           Implementation                                  */
/* ######################################################################### */

int gpif_init(gpif_session_t *session, char *const argv[])
{
    int rc;
    pid_t pid;
    int pipe_togp[2];
    int pipe_fromgp[2];

    if (!session)
        return EGPIFINVAL;

    if (!argv)
        return EGPIFINVAL;

    rc = pipe (pipe_togp);
    if (rc < 0)
        return EGPIFERR;

    rc = pipe (pipe_fromgp);
    if (rc < 0)
        return EGPIFERR;

    pid = fork();
    if (pid < 0) {
        return EGPIFERR;
    }

    /* Child */
    if (pid == 0) {

        /* STDIN / STDOUT will automatically be closed by dup2() if necessary */
        dup2(pipe_togp[0], STDIN_FILENO);
        dup2(pipe_fromgp[1], STDOUT_FILENO);
       
        /* These file descriptors are now unneeded */
        close(pipe_togp[0]);
        close(pipe_togp[1]);
        close(pipe_fromgp[0]);
        close(pipe_fromgp[1]);

        execvp(argv[0], argv);
        perror("execvp");
        return 1;
    }

    /* Parent. TODO: Make sure everything is OK in the child. */

    /* Close unused pipe ends */
    close(pipe_togp[0]);
    close(pipe_fromgp[1]);

    /* Make the read end non blocking, if there is nothing to read then so be
     * it. */
    rc = fcntl(pipe_fromgp[0], F_SETFL, O_NONBLOCK);
    if (rc < 0)
        return EGPIFERR;

    session->gp_pid = pid;
    session->gp_read = pipe_fromgp[0];
    session->gp_write = pipe_togp[1];

    return EGPIFOK;
}

int gpif_close(gpif_session_t *session)
{
    int rc;
    size_t count;
    int exit_status;

    if (!session)
        return EGPIFINVAL;

    /* Send quit command. The leading newline is for safety in case some garbage
     * ended up on the command line */
    count = strlen("\nquit\n");
    rc = gpif_write(session, "\nquit\n", &count);
    if (rc != EGPIFOK)
        return EGPIFERR;

    rc = waitpid(session->gp_pid, &exit_status, 0);
    if (rc < 0)
        return EGPIFERR;

    if(!WIFEXITED(exit_status))
        return EGPIFERR;

    close(session->gp_read);
    close(session->gp_write);

    return EGPIFOK;
}

int gpif_write(gpif_session_t *session, const char *buf, size_t *count)
{
    ssize_t written;

    if (!session)
        return EGPIFINVAL;
    if (!buf)
        return EGPIFINVAL;
    if (!count)
        return EGPIFINVAL;
    
    written = prv_gpif_write(session->gp_write, buf, *count);
    if (written != *count) {

        if (written < 0)
            written = 0;

        *count = written;
        return EGPIFERR;
    }

    return EGPIFOK;
}

int gpif_read(gpif_session_t *session, void *buf, size_t *count)
{
    ssize_t bytes_read;

    if (!session)
        return EGPIFINVAL;
    if (!buf)
        return EGPIFINVAL;
    if (!count)
        return EGPIFINVAL;

    bytes_read = prv_gpif_read(session->gp_read, buf, *count);
    if (bytes_read < 0) {
        *count = 0;
        return EGPIFERR;
    }
    
    *count = bytes_read;
    return EGPIFOK;
}

ssize_t prv_gpif_read(int fd, void* vptr, ssize_t len)
{
    ssize_t n, rc;
    char *ptr;
    fd_set waitforread;

    ptr = vptr;
    n = 0;

    while(n < len) {
        if((rc = read(fd, ptr, len-n)) > 0) {
            n+=rc;
            ptr=&((char*)vptr)[n];

            /* Text mode, break on newline */
            if (((char*)vptr)[n-1] == '\n')
                return n;

            if (n == len)
                return n;
        }
        else {
            if (errno == EINTR)
                ;
            else if (errno == EAGAIN) {

                FD_ZERO(&waitforread); 
                FD_SET(fd, &waitforread);
                if (select(fd+1, &waitforread, NULL, NULL, NULL) < 0) {
                    perror("select");
                    return -1;
                }     

            }
            else {
                if (rc != 0)
                    perror("read");

                return -1;
            }
        }
    }

    return n;
}

ssize_t prv_gpif_write(int fd, const void* ptr, size_t len) 
{
    size_t nleft;
    ssize_t nwritten;
    fd_set waitforwrite;
    unsigned char *byteptr = (unsigned char*)ptr;

    nleft = len;
    while(nleft > 0) {
        if((nwritten = write(fd, (const void*)byteptr, nleft)) <= 0) {

            if(errno == EINTR)
                nwritten = 0;
            else if(errno == EAGAIN) {

                nwritten = 0;
                
                FD_ZERO(&waitforwrite); 
                FD_SET(fd, &waitforwrite);
                if (select(fd+1, NULL, &waitforwrite, NULL, NULL) < 0) {
                    perror("select");
                    return -1;
                } 
            }
            else {
                perror("write");
                return -1;
            }
        }
        nleft -= nwritten;
        byteptr += nwritten;
    }

    return len;
}

