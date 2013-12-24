Yet another Smalltalk VM
========================

… is Smalltalk Virtual Machine. It supports Smalltalk as described in Bluebook
and partially ANSI Smalltalk with support for class definiton syntax.

… is written in C and contains bytecode compiler (used only for compiled code
representation - not interpreted), JIT (currently only x86-64 is supported),
generational GC with moving GC on new space and mark & sweep on old space.

… is tested on x86-64 Linux, Os X and FreeBSD.


Usage
-----

For building VM you need: Clang or GCC and Cmake.

```sh
# withing VM root directory
cmake .
make all
./st -b smalltalk # compiles Smalltalk kernel and writes to ./snapshot
```
