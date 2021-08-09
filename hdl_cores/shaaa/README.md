[//]: # (Copyright 2021 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

<p align="center">
   Copyright 2021 Massachusetts Institute of Technology
</p>

## SHAAA - Secure Hash Algorithm, Arbitrarily Augmented

The shaaa_verilog_generator.py script generates randomized variants of SHA2 algorithms.

Executing `shaaa_verilog_generator.py -h` will print a complete list of script options.

Example usage:
```
    ./shaaa_verilog_gen.py --random256 -o gen_256_a
        ^   Generate a 256 bit output SHAAA module, save in the gen_256_a folder. All 
            constants are randomly selected, and the IO are the same size as standard 
            SHA256.
    
    ./shaaa_verilog_gen.py --config config.json 
        ^   Generate a SHAAA module based on the specification in config.json

```

Generated outputs:
```
    .gitignore           - Automatically ignore the entire output directory. Only
                           generated if it doesn't already exist.
    config.json          - Copy of the configuration used to generate the verilog, 
                           can be used as the --config input.
    shaaa_k_constants.v  - K constants
    shaaa_stimulus.v     - Stimulus for shaaa_tb.sv
    shaaa_tb.sv          - Main testbench, not LLKI enabled.
    shaaa_w_mem.v        - Rolling W memory
    shaaa.v              - Top level SHAAA module
```


### SHAAA Configuration
SHAAA is based on the SHA2 family, and supports the following configuration parameters:
| Name                  | Range                         | Description                              |
|-----------------------|-------------------------------|------------------------------------------|
| WORDSIZE              | 32,64                         | Size of each word, in bits               |
| BLOCKSIZE             | 512,1024 (Must be 16*WORDSIZE)| Size of each hash block, in bits         |
| DIGESTBITS            | 1 <= DIGESTBITS <= 8*WORDSIZE | Cropped output size. 256 for SHA256, etc.|
| ITERATIONS            | >=1                           | Number of iterations for each block. Randomizer picks value in range [64, 127] |
| H0                    | [WORD]x8                      | Initial H register values.               |
| K                     | [WORD]xITERATIONS             | K constants to use for each round.       |
| W_SUMS                | [1<=z<=16]xRANDOM             | Random sampling of integers from 1 to 16. Used as the "sum" terms for W generation. The randomizer selects 2-4 values, but the number used is arbitrary. |
| P_SIGMA_LOW_<0,1>     | 1<=z<=WORDSIZE                | Select which W words are used as parameters in SIGMA_LOW functions. |
| SIGMA_<LOW,CAP>_<0,1> | [1<=z<=WORDSIZE]x3            | Tuple of 3 ints (max WORDSIZE) selecting which W words are used for SIGMA functions. |


#### Return to the root CEP [README](../../README.md)
