# Copyright (C) 2020 Massachusetts Institute of Technology #


This directory contains everything you need to run unit level simulation for the **gps** core (DUT)

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

1. Vector file (gps_stimulus.txt) is in HEX format to be used by $readmemh, and every signal (input/output) is captured and placed in individual column separated by underscore (_), respectively. Since not every captured signal's width is divisible by 4 (HEX character=4 bits), bit filler are used to pad these signals to number of bits that divisible by 4. For example, syn_rst_in is 1 bit and it occupies one column in the stimulus file, therefore, 3'b000 are padded to make 4 bits when captured. The testbench is already setup to extract those filler bits and throw away, so the user doesn't need to worry about them as long as the testbench is used as is.

2. This module requires reset to be applied at beginning of every transaction, therefore, it is also part of the stimulus.

3. This module has 2 clocks: fast and slow clock. Slow clock is derived from fast clock and their phase relationship are established when module reset is de-asserted. Therefore, if logic locking is implemented, care should be taken regard clock relationship following unlocking of the core.

As of 05/20/20: Note#2 & 3 are no longer needed.

