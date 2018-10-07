C4-Lisp
=======

An experiment on the [C4](https://github.com/rswier/c4) self-hosting C virtual machine.

This might better be known as a Scheme interpreter, but the author isn't sure on the correct terminology.

It features an expanded C4 interpreter, with the ability to load from dynamic libraries for additional platform support.


Goals
-----

This project aims to provide a virtual machine that can run multiple lisp interpreters in a single process, switching
between those processes in a pre-emptive tasking fashion.

The Lisp interpreter is based on the classic [90(++)-line scheme interpreter](https://github.com/anthay/Lisp90).

As with [Elispidae](https://github.com/andrakis/elispidae), the project will eventually use C++14's shared pointers and contain an extended type system.


Accomplished Goals
------------------

* A tail recurse Scheme/Lisp interpreter. Uses strings for most values, except for Lists and Procs.

* Standard test library (used in above 90(++)-line scheme interperter) passes successfuly.

* Separatation of C4 from all Lisp/Scheme related code

* Support for microprocesses in C4 (can task switch between them.)

* Support for per-microprocess custom platform libraries

The Scheme/Lisp interpreter can run code from command line, load a file containing code, or load code from a <code>.c</code> source file (C4 or compiled source.)

Care is taken to ensure code runs the same in both the C4 interpreter and in native compiled mode.
