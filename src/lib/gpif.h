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

/** @file gpif.h
 *
 * @brief Public library interface
 *
 * */

/** @mainpage A bare metal C interface to gnuplot
 *
 * Installation instructions can be found in INSTALL.txt in the software
 * package's root directory.
 *
 * Feedback to bjoern@shugaa.de is always highly appreciated.
 */

#ifndef _GPIF_H
#define _GPIF_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* ######################################################################### */
/*                            TODO / Notes                                   */
/* ######################################################################### */

/* ######################################################################### */
/*                            Types & Defines                                */
/* ######################################################################### */

#define EGPIFERR     0   /* General error */
#define EGPIFOK      1   /* Success */
#define EGPIFINVAL   2   /* Invalid argument */

typedef struct {
    /* PID of the gnuplot process */
    pid_t gp_pid;
    
    /* read and write file descriptors which are gnuplot's STDIN and STDOUT
     * respectively */
    int gp_read;
    int gp_write;
} gpif_session_t;

/* ######################################################################### */
/*                            Public interface                               */
/* ######################################################################### */

/** Create a gnuplot session.
 *
 * A call to this function will fork/exec a gnuplot child process, set up pipes
 * to it's STDIN/STDOUT accoringly and populate the supplied session with the
 * necesary information.
 *
 * @param session     Session to be initialized
 * @param argv        First item: name of the gnuplot binary, will usually be 'gnuplot'
 *                    Any successive items: gnuplot arguments
 *                    The last item must always be NULL
 *
 * @return EGPIFOK    No errors occured
 * @return EGPIFINVAL An invalid argument has been passed
 * @return EGPIFERR   Session could not be created
 */
int gpif_init(gpif_session_t *session, char *const argv[]);

/** Destroy a gnuplot session
 *
 * This will automatically send a 'quit' command to gnuplot and do some cleanup.
 *
 * @param session     Session to be destroyed
 *
 * @return EGPIFOK    No errors occured
 * @return EGPIFINVAL An invalid argument has been passed
 * @return EGPIFERR   Session could not be destroyed
 */
int gpif_close(gpif_session_t *session);

/** Write to gnuplot's STDIN 
 * 
 * count will be modified by this function to reflect the number of bytes
 * actually written.
*
 * @param session     gnuplot session to be used
 * @param buf         Write buffer
 * @param count       Number of bytes to write / Number of bytes written (in / out)
 *
 * @return EGPIFOK    No errors occured
 * @return EGPIFINVAL An invalid argument has been passed
 * @return EGPIFERR   Unable to write
 */
int gpif_write(gpif_session_t *session, const char *buf, size_t *count);

/** Read from gnuplot's STDOUT
 *  
 * Try to read up to count bytes from gnuplot's STDOUT. Count will be
 * manipulated by this function call to reflect the actual number of bytes read.
 *
 * @param session     gnuplot session to be used
 * @param buf         Read buffer
 * @param count       Number of bytes to read / Number of bytes read (in / out)
 *
 * @return EGPIFOK    No errors occured
 * @return EGPIFINVAL An invalid argument has been passed
 * @return EGPIFERR   Unable to read
 */
int gpif_read(gpif_session_t *session, void *buf, size_t *count);


#endif /* _GPIF_H */
