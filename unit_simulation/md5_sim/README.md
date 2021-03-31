# Copyright 2021 Massachusetts Institute of Technology #


This directory contains everything you need to run unit level simulation for the **md5** core (DUT)

## Pre-Requisites ##

Any Modelsim or Questa version installed that supports SystemVerilog and Verilog.

make utility

## To Run simulation ##

Just type  

```
make
```

This will compile and run the simulation where stimulus will be applied to the DUT's input and outputs are checked every cycle.

**NOTES**:

1. Vector file (md5_stimulus.txt) is in HEX format to be used by $readmemh, and every signal (input/output) is captured and placed in individual column separated by underscore (_), respectively. Since not every captured signal's width is divisible by 4 (HEX character=4 bits), bit filler are used to pad these signals to number of bits that divisible by 4. For example, rst is 1 bit and it occupies one column in the stimulus file, therefore, 3'b000 are padded to make 4 bits when captured. The testbench is already setup to extract those filler bits and throw away, so the user doesn't need to worry about them as long as the testbench is used as is.

2. This module requires reset to be applied at beginning of every transaction, therefore, it is also part of the stimulus.

As of 05/20/20: Note#2 is no longer needed.