# Copyright (C) 2020 Massachusetts Institute of Technology #


This directory contains everything you need to run unit transaction-level simulation for all cores that are wrapped under tileLink.
It contains one common testbench and it is used to run simulation for any core. See Makefile for details

## Pre-Requisites ##

Any Modelsim or Questa version installed that supports SystemVerilog and Verilog.

## Preprare Stimulus for simulation ##

Since the command sequences (stimulus that are captured from chip level simulation) are in C-header format, they need to be converted to system verilog format for this unit level simulation, respectively. This below step only need to be done once whenever there is a change in the orginal tests or command sequences are re-captured.

```
make vectors
```
When done, check sv_vectors directory to verify all the stimulus files are created there.

## To Run single core simulation ##

Just type  

```
make CORE=<aes|md5|des3|sha256|gps|dft|idft|fir|iir|rsa>

```

**NOTE**: Make without any argument will use **aes** core as default.

This will compile and run the simulation for the selected core.

## To Run all cores simulation ##

```
make clean	<-- only if you want to
make runAll

```
**NOTE**: Each core will have its own log file captured in <core>.log and wave file <core>.wlf, respectively.




