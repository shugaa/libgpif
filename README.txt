libgpif (GnuPlot InterFace) is a bare metal simple as hell C Interface to
gnuplot. All it does (at the moment) is set up a gnuplot session as a child
process which you can then send commands to and read data from. 

Maybe sometime there will be abstractions to gnuplot's command set but imho this
simplistic approach offers the highest degree of flexibility. 

If you're looking for something more sophisticated you might want to have a look
at http://ndevilla.free.fr/gnuplot/.

This library has been written with Linux as a target operating system in mind
but it'll probably be easy to port to something else which adheres to the POSIX
standard (mostly in terms of process handling).

For install instructions have a look at INSTALL.txt.

API documentation in HTML format can be found in the doc directory. Also,
test/gpif_testcase.c should give you a pretty good impression of how to use the
library.
