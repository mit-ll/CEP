[//]: # (Copyright 2021 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

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




