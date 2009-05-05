/*
* Copyright (c) 2009, Björn Rehm (bjoern@shugaa.de)
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*  * Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*  * Neither the name of the author nor the names of its contributors may be
*    used to endorse or promote products derived from this software without
*    specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
ssize_t prv_gpif_read(int fd, void* vptr, ssize_t len);
ssize_t prv_gpif_write(int fd, const void* ptr, size_t len);

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

    /* Parent. 
     * TODO: We need to make sure the exec step in the child succeeded */

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
    if (read < 0) {
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

    nleft = len;
    while(nleft > 0) {
        if((nwritten = write(fd, ptr, nleft)) <= 0) {

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
        ptr += nwritten;
    }

    return len;
}

