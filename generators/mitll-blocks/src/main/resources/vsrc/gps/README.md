[//]: # (Copyright 2022 Massachusets Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

<p align="center">
   Copyright 2022 Massachusets Institute of Technology
</p>

# GPS code generator
Generates three codes used for GPS signal correlation (from the perspective of a space vehicle): C/A-code, P-code, and a secure form of P-code that we call L-code.  The combination of the P-code generator and the L-code is meant to serve as a surrogate for the M-code generator found in modern secure GPS systems.  The goal of this core is to provide reference implementations for items that a GPS designer would like to protect from an untrusted manufacturer; it is not a fully-functional GPS unit.

## Functionality:
While this block generates accurate P-code and C/A-code sequences, it does not implement the GPS spec exactly. Notable differences:
* "L-code" is a proxy for the M-code, and in this implementation just encrypts with AES in 128 bit blocks.
  * Note that the default AES key is currently 0x00000... This can be customized as needed in gps.scala
* Generates batches of 128 bits of the P-code and 13 bits of C/A-code, which roughly approximates the 10:1 ratio of a real GPS device. Likewise real GPS systems have precise timing specifications which are not followed here.

## References
* https://www.gps.gov/technical/icwg/IS-GPS-200K.pdf
* https://www.mitre.org/publications/technical-papers/overview-of-the-gps-m-code-signal
* https://natronics.github.io/blag/2014/gps-prn/
* https://github.com/kristianpaul/gnsssdr/blob/master/ARM_FPGA_PROJECTS/Xilinx_ISE_14.1_namuru_project_for_async_memory_bus/code_gen.v
* http://www.ohio.edu/people/starzykj/network/research/Papers/Recent%20conferences/Pcode_Generator_FPGA_FPLA2002.pdf
* https://etd.ohiolink.edu/!etd.send_file?accession=ohiou1177700503&disposition=inline
* https://www.navcen.uscg.gov/pubs/gps/icd200/ICD200Cw1234.pdf
* https://patents.google.com/patent/US5068872
* https://patents.google.com/patent/US5202694
* https://patents.google.com/patent/US6567042

#### Return to the root CEP [README](../../README.md)