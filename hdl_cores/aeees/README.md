[//]: # (Copyright 2020 Massachusetts Institute of Technology)

<p align="center">
   Copyright 2020 Massachusetts Institute of Technology
</p>

## AEEES-WB - Advanced Egregiously Extended Encryption Standard, White-Box Edition

The aeees.py script generates extended variants of the AES-128 core with the option of
"baking in the key" if desired.  

Executing `aeees.py -h` will print a complete list of script options.

Example usage:
```
    ./aeees.py -k 12345 -r 100 -t 500 -v    <- Generate a 100-round variant of AES-128 with the
                                               key fixed at "12345".  Generate 500 test vectors.
                                               Run in verbose mode

    ./aeees.py -nk -r 200 -t 250            <- Generate a 200-round variant of AES-128 with a
                                               key schedule.  Generate 250 test vectors.                                                 
```

Generated outputs from the first run example:
```
    aeees_roundfn.vi    - Utility functions for the AES rounds 
    aeees_rounds.v      - Individual definitions of all (100) AES rounds    
    aeees_box_XXX.vi    - Generated AES S-Box for round XXX with "baked-in" key
    aeees.v             - Top level AEEES module
    aeees_tb.v          - AEEES module testbench
    aeees_stimulus.v    - Stimulus file for the AEEES testbench
    Makefile            - Simulation makefile - just run "make"
    vsim.do             - Simulation TCL script
```

Generated outputs from the second run example:
```
    aeees_rounds.v      - Individual definitions of all (250) AES rounds    
    aeees_table.v       - AES S-Box definition
    aeees.v             - Top level AEEES module
    aeees_stimulus.csv  - Stimulus file for the AEEES testbench
    aeees_tb.sv         - AEEES module testbench
    Makefile            - Simulation makefile - just run "make"
    vsim.do             - Simulation TCL script
```

All generated files are implicitly included in .gitignore and are thus not tracked by git.

Caution should be taken when running the aeees.py script with the `--clean` switch.  

The following files will be deleted with `--clean`:
```
    aeees.v
    aeees_rounds.v
    eees_table.v
    aeees_stimulus.csv
    eees_tb.sv
    Makefile
    vsim.do
    aeees_roundfn.vi
    aeees_box_*.vi
```

#### Return to the root CEP [README](../../README.md)

