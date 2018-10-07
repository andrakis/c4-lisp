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


Extensions to C4
----------------

* Support for <code>/* C style comments */</code>

* Additional escape codes; negative number support for enums; other various quality-of-life improvements and more support for the C standard.

* Microprocess support

   * C4 can now interpret multiple processes, done in a preemptive (cycle count) fashion.
  
   * Each process has its own data segment, symbol table, and stack.
  
   * Multiple processes can be run concurrently (using the <code>-r file.c</code> flag), and their respective data is freed upon completion of their execution.
  
* The concept of a "runtime platform" and "system calls" has been introduced.
  
   * A runtime platform is a set of functions loaded from a shared library object at runtime.
   
   * A runtime platform implements a set of system calls that provide added functionality to the C runtime provided by the C4 interpreter.
   
   * System calls take a `signal`, an integer that signifies the instruction to be run.
   
   * `syscall1` takes no arguments except for the signal.
   
   * `syscall2` through `syscall4` each take an additional argument, each of which is an integer.
   
   * The `syscall` interface may convert any argument to a different data type (or struct/class pointer) to perform its actions.
   
   * Any data allocated from a `syscall` must be deallocated by another `syscall`. That is, data allocated in the runtime platform must be deallocated from it also. This is a restriction of shared object memory allocation.
   
   * All of these interfaces can be seen in action in the following files:
   
     * [scheme.c](platform/scheme/scheme.c) - uses `syscall` functions to implement a Scheme interpreter. Can be compiled or run with the interpreter.
    
      * [syscalls.c](platform/scheme/syscalls.c) - exported `syscall` interface, containing the functions that are loaded by the platform runtime.
    
      * [scheme_internal.cpp](platform/scheme/scheme_internal.cpp) - provides basic Scheme types, parser, functions for receiving/returning and manipulating Scheme objects, and the handlers for the `syscall` functions. Contains some additional reference information (including the original C++ `eval()` loop upon which `scheme.c` is based on), and is the interface between C4 and the Scheme interpreter. Usually returns pointers (cast to integers) and re-casts them to the appropriate data type as needed.
    
   * Passing incorrect data to the Scheme platform syscalls results in undefined behaviour, as there is very little runtime error checking. Care must be taken to adhere to the `SYSCALL` enums, which contain documentation as to appropriate calling behaviour.
