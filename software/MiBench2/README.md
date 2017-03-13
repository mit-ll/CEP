# MiBench2
[MiBench](http://vhosts.eecs.umich.edu/mibench/) ported for IoT devices.

All benchmarks include [barebench.h](barebench.h).  This file contains the `main()` used in building every benchmark and determines the number of benchmark trials and what happens when a benchmark attempts to print to the screen.

### Prerequisites

You will need a cross-compiler for your target platform. The default target is the ARM-Cortex-M0+.  The easiest way to get a working cross-compiler for the Cortex-M0+ is to download the prebuilt binaries from [Launchpad.net](https://launchpad.net/gcc-arm-embedded).

Whatever toolchain you go with, update the paths and commands in the global [make file](Makefile.mk).

### Building

`cd` to the appropriate benchmark directory.

`make`


Running `make` produces several useful files:
   `main.elf` an ELF executable suitable for loading to a board or simulator using GDB
   `main.bin` a raw binary suitable for loading directly in to the memory of a board or a simulator
   `main.lst` assembly listing


Running `make clean` will remove all files produced during compilation.


To build all benchmarks and move the resulting bin files to the repo's top-level directory, run [buildAll.sh](buildAll.sh).

### Porting

[memmap](memmap) contains the memory map used by the linker to place program sections.  Edit this file to change the size of memory or the location/size of individual program sections (e.g., stack and heap).

[vectors.s](vectors.s) contains the exception jump table and the execution entry point `_start` and exit point `exit`.  You may need to edit this file if you target a different instruction set than the ARMv6-M.

[putget.s](putget.s) contains low level functions written in assembly.  Edit the `putchar` function in this file to change the behavior of all C-level printing operations.

[supportFuncs.c](supportFuncs.c) contains functions needed to port newlib to our target platform.


