This is my entry for the Intel Threading Challenge, P2:A4: Maximal Paths.


Pre-requisites
==============

- Threading Building Blocks (TBB) 3.0 or higher.
- The Intel C++ compiler, version 11.1 or higher.
- 64-bit Linux.


To build it
===========

- Set up the environment variables for the Intel compiler and TBB. For example:

    source /opt/intel/Compiler/11.1/073/bin/iccvars.sh intel64
    source /opt/intel/tbb/3.0/bin/tbbvars.sh intel64

  (you may need to adjust those paths to match your own installations).

  It's important to run the tbbvars script *after* the iccvars script. If you
  don't, the build will find the wrong version of TBB and you'll get compiler
  errors.

- Run make

That should be it. The executable it generates is called 'maximalpath'.


To run it
=========

  ./maximalpath <graph.txt> <nodes.txt>

(as in the problem description).

The output includes a print out of the time taken. This is the wall-clock
execution time for the entire program, inclduing all I/O and all computation.

