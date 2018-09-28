C4-Lisp
=======

An experiment on the [C4](https://github.com/rswier/c4) self-hosting C virtual machine.

This might better be known as a Scheme interpreter, but the author isn't sure on the correct terminology.

New opcodes are added:

* SYS1, SYS2, SYS3, SYS4: Calls functions in internal.cpp, which implement the behind-the-scenes details of the language.

* FDSZ: gets filesize from a file descriptor

A number of other enhancements have been added:

* Support for 64bit compilation

* Memory allocated is properly freed

* Start of the Lisp-like interpreter


Goals
-----

This project aims to provide a virtual machine that can run multiple lisp interpreters in a single process, switching
between those processes in a pre-emptive tasking fashion.

The Lisp interpreter is based on the classic [90(++)-line scheme interpreter](https://github.com/anthay/Lisp90).

As with [Elispidae](https://github.com/andrakis/elispidae), the project will eventually use C++14's shared pointers and contain an extended type system.
