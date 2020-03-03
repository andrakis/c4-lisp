Lisp on C4
==========

Directory Structure
-------------------

platform/             Interpreter platforms
  lisp/               Lisp interpreter
    bin/              Contains scripts to build c4_src (below)
    c_src/            Individual .c files intended for compilation (not C4)
    c4_src/           Destination for bundled .c4 file containing contents of above
    Makefile          Used to build the c4_src contents
