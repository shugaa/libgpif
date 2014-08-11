#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
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

static ssize_t fdread(int fd, void* vptr, ssize_t len);
static ssize_t fdwrite(int fd, const void* ptr, size_t len);

/* ######################################################################### */
/*                           Implementation                                  */
/* ######################################################################### */

int gpif_init(gpif_session *session, char *const argv[])
{
    int rc;
    pid_t pid;
    int pipe_togp[2] = {-1,-1};
    int pipe_fromgp[2] = {-1,-1};

    /* Basic parameter checking */
    if (session == NULL)
        return EGPIFINVAL;
    if (argv == NULL)
        return EGPIFINVAL;

    /* Create read and write pipes for interacting with GNUPlot */
    for(;;)
    {
        rc = pipe (pipe_togp);
        if (rc < 0)
        {
            /* Pipe error */
            break;
        };

        rc = pipe (pipe_fromgp);
        if (rc < 0)
        {
            /* Pipe error */
            break;
        }

        pid = fork();
        if (pid < 0) 
        {
            /* Fork error */
            rc = -1;
            break;
        }

        rc = 0;
        break;
    }
   
    /* Something went horribly wrong, don't bother continuing */
    if (rc != 0)
    {
        close(pipe_togp[0]);
        close(pipe_togp[1]);
        close(pipe_fromgp[0]);
        close(pipe_fromgp[1]);

        return EGPIFERR;
    }

    /* Child */
    if (pid == 0) 
    {
        /* Make STDIN and STDOUT refer to the respective pipe ends */
        dup2(pipe_togp[0], STDIN_FILENO);
        dup2(pipe_fromgp[1], STDOUT_FILENO);

        /* These file descriptors are now unneeded */
        close(pipe_togp[0]);
        close(pipe_togp[1]);
        close(pipe_fromgp[0]);
        close(pipe_fromgp[1]);

        /* Exec child process with gnuplot. Will only return on error */
        execvp(argv[0], argv);
        perror("execvp");
        return EGPIFERR;
    }

    /* Parent. */

    /* Close unused pipe ends */
    close(pipe_togp[0]);
    close(pipe_fromgp[1]);

    /* Make the read end non blocking, if there is nothing to read then so be
     * it. */
    for (;;)
    {
        rc = fcntl(pipe_fromgp[0], F_SETFL, O_NONBLOCK);
        if (rc < 0)
            break;

        break;
    }
   
    /* Error */
    if (rc < 0)
    {
        /* This is not the most sophisticated exit strategy but it'll probably
         * do. */
        kill(pid, SIGKILL);
        close(pipe_togp[1]);
        close(pipe_fromgp[0]);
        return EGPIFERR;
    }

    /* Everything OK so far, initialize session */
    session->gp_pid = pid;
    session->gp_read = pipe_fromgp[0];
    session->gp_write = pipe_togp[1];

    return EGPIFOK;
}

int gpif_close(gpif_session *session)
{
    int rc;
    size_t count;
    int exit_status;
    char quitcmd[] = "\nquit\n";

    if (session == NULL)
        return EGPIFINVAL;

    /* Send quit command. The leading newline is for safety in case some garbage
     * ended up on the command line */
    count = strlen(quitcmd);
    rc = gpif_write(session, quitcmd, &count);
    if (rc != EGPIFOK)
        return EGPIFERR;

    /* Wait for the GNUPlot child to exti */
    rc = waitpid(session->gp_pid, &exit_status, 0);
    if (rc < 0)
        return EGPIFERR;

    if(!WIFEXITED(exit_status))
        return EGPIFERR;

    /* Close open file descriptors */
    close(session->gp_read);
    close(session->gp_write);

    return EGPIFOK;
}

int gpif_write(gpif_session *session, const char *buf, size_t *count)
{
    ssize_t written;

    if (session == NULL)
        return EGPIFINVAL;
    if (buf == NULL)
        return EGPIFINVAL;
    if (count == NULL)
        return EGPIFINVAL;
    
    written = fdwrite(session->gp_write, buf, *count);
    if (written != *count) 
    {
        if (written < 0)
            written = 0;

        *count = written;
        return EGPIFERR;
    }

    return EGPIFOK;
}

int gpif_read(gpif_session *session, void *buf, size_t *count)
{
    ssize_t bytes_read;

    if (session == NULL)
        return EGPIFINVAL;
    if (buf == NULL)
        return EGPIFINVAL;
    if (count == NULL)
        return EGPIFINVAL;

    bytes_read = fdread(session->gp_read, buf, *count);
    if (bytes_read < 0) 
    {
        *count = 0;
        return EGPIFERR;
    }
    
    *count = bytes_read;
    return EGPIFOK;
}

ssize_t fdread(int fd, void* vptr, ssize_t len)
{
    ssize_t n, rc;
    char *ptr;
    fd_set waitforread;

    ptr = vptr;
    n = 0;

    while(n < len) 
    {
        if((rc = read(fd, ptr, len-n)) > 0) 
        {
            n+=rc;
            ptr=&((char*)vptr)[n];

            /* Text mode, break on newline */
            if (((char*)vptr)[n-1] == '\n')
                break;

            if (n == len)
                break;
        }
        else {
            /* Just try again */
            if ((rc < 0) && (errno == EINTR))
            {
                continue;
            }
            /* Wait for fd ready and try again */
            if ((rc < 0) && (errno == EAGAIN))
            {
                FD_ZERO(&waitforread); 
                FD_SET(fd, &waitforread);
                if (select(fd+1, &waitforread, NULL, NULL, NULL) < 0) 
                {
                    perror("select");
                    n = -1;
                    break;
                }
                continue;
            }
        
            /* Some fatal error */
            if (rc != 0)
            {
                perror("read");
                n = -1;
                break;
            }

            /* Nothing to read */
            break;
        }
    }

    return n;
}

ssize_t fdwrite(int fd, const void* ptr, size_t len) 
{
    size_t nleft;
    ssize_t nwritten;
    fd_set waitforwrite;
    unsigned char *byteptr = (unsigned char*)ptr;

    nleft = len;
    while(nleft > 0) 
    {
        if((nwritten = write(fd, (const void*)byteptr, nleft)) <= 0) 
        {
            if(errno == EINTR)
            {
                continue;
            }
            if(errno == EAGAIN) 
            {
                FD_ZERO(&waitforwrite); 
                FD_SET(fd, &waitforwrite);
                if (select(fd+1, NULL, &waitforwrite, NULL, NULL) < 0) 
                {
                    perror("select");
                    len = -1;
                    break;
                }
                continue;
            }
            
            /* Some other fatal error */
            perror("write");
            len = -1;
            break;
        }

        nleft -= nwritten;
        byteptr += nwritten;
    }

    return len;
}

