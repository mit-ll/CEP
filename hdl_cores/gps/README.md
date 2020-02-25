<p align="center">
   v2.0
   <br>
   Copyright 2019 Massachusetts Institute of Technology
</p>

<br>

# GPS code generator

Generates three codes used for GPS signal correlation (from the perspective of a space vehicle): C/A-code, P-code, and a secure form of P-code that we call L-code.  The combination of the P-code generator and the L-code is meant to serve as a surrogate for the M-code generator found in modern secure GPS systems.  The goal of this core is to provide reference implementations for items that a GPS designer would like to protect from an untrusted manufacturer; it is not a fully-functional GPS unit.

## Protected secrets:
* Length of P-code generator
* Position of taps for P-code generator
* Initial value used by P-code generator
* Symmetric key used for creating L-code

## References
* https://www.mitre.org/publications/technical-papers/overview-of-the-gps-m-code-signal
* https://natronics.github.io/blag/2014/gps-prn/
* https://github.com/kristianpaul/gnsssdr/blob/master/ARM_FPGA_PROJECTS/Xilinx_ISE_14.1_namuru_project_for_async_memory_bus/code_gen.v
* http://www.ohio.edu/people/starzykj/network/research/Papers/Recent%20conferences/Pcode_Generator_FPGA_FPLA2002.pdf
* https://etd.ohiolink.edu/!etd.send_file?accession=ohiou1177700503&disposition=inline
* https://www.navcen.uscg.gov/pubs/gps/icd200/ICD200Cw1234.pdf
* https://www.google.com/patents/US5068872
* http://www.google.com.pg/patents/US5202694
* https://www.google.com/patents/US6567042
