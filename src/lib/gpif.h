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

#ifndef GPIF_H
#define GPIF_H

#include <sys/types.h>

/* ######################################################################### */
/*                            TODO / Notes                                   */
/* ######################################################################### */

/* ######################################################################### */
/*                            Types & Defines                                */
/* ######################################################################### */

#define EGPIFOK      (1)    /* Success */
#define EGPIFERR     (-1)   /* General error */
#define EGPIFINVAL   (-2)   /* Invalid argument */

typedef struct {
    /* PID of the gnuplot process */
    pid_t gp_pid;
    
    /* read and write file descriptors which are gnuplot's STDIN and STDOUT
     * respectively */
    int gp_read;
    int gp_write;
} gpif_session;

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
int gpif_init(gpif_session *session, char *const argv[]);

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
int gpif_close(gpif_session *session);

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
int gpif_write(gpif_session *session, const char *buf, size_t *count);

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
int gpif_read(gpif_session *session, void *buf, size_t *count);

#endif /* GPIF_H */

