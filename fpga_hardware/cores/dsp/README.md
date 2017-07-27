# CEP-DSP: Core blocks

The core dsp blocks (DFT/IDFT/IIR/FIR) were generated using hardware generators provided by the [spiral project](http://www.spiral.net/). Due to licensing we could not include the sources files for the core dsp blocks we used. To rebuild and included the core files for follow these steps:

## Websites:
* DFT/iDFT: [Spiral Project DFT Gen](http://www.spiral.net/hardware/dftgen.html)
* IIR/FIR: [Spiral Project FFT Gen](http://www.spiral.net/hardware/filter.html)

## Generator Settings:
* Note: Modifications to the default generator setting will be noted as `modifed`

* DFT/iDFT:

Core(s)  |DFT |IDFT
---------|---:|----:
**Problem Specification**||
Transform Size|64|64
Direction|Forward|`Inverse`
Data type|Fixed Point|Fixed Point
||16 bits|16 bits
||unscaled|unscaled
**Parameters Controling Implementation**||
Architecture|fully streaming|fully streaming
radix|2|2
streaming width|2|2
data ordering|Natural in / natural out|Natural in / natural out
BRAM budget|1000|1000
Permutation Method|`DATE'09 [4] (patent Free)`|`DATE'09 [4] (patent Free)`

* IIR/FIR

Core(s)  |IIR |FIR
---------|---:|---:
Example Filter| 6| 6
Filter Taps a_k|1.000000e+000|`1`
||8.274631e-016|`0`
||1.479294e+000|`0`
||1.089388e-015|`0`
||7.013122e-001|`0`
||4.861197e-016|`0`
||1.262132e-001|`0`
||9.512539e-017|`0` 
||7.898376e-003|`0` 
||-4.496047e-018|`0` 
||1.152699e-004|`0` 
||-6.398766e-021|`0` 
Filter Taps b_k|1.618571e-003|1.618571e-003
||1.780428e-002|1.780428e-002
||8.902138e-002|8.902138e-002
||2.670642e-001|2.670642e-001
||5.341283e-001|5.341283e-001
||7.477796e-001|7.477796e-001
||7.477796e-001|7.477796e-001
||5.341283e-001|5.341283e-001
||2.670642e-001|2.670642e-001
||8.902138e-002|8.902138e-002
||1.780428e-002|1.780428e-002
||1.618571e-003|1.618571e-003
Fractional bits|8|8
Bitwidth|32|32
Module Name|acm_filter|acm_filter
Input Data|inData|inData
Register Input|Yes|Yes
Output Data|outData|outData
Register Output|Yes|Yes
Clock Name|clk|clk
reset|negedge|negedge
Filter Form|I|I
Debug Output|`Off`|`Off`

## Additional Modifications:
* DFT
  * Location: ./CEP/fpga_hardware/cores/dsp/dft/
  * Name: filter_dft.v
  * testbench: Included in the generator source is a testbench that should be extracted and stored as tb_filter_dft.v
* iDFT
  * Location: ./CEP/fpga_hardware/cores/dsp/idft/
  * Name: filter_idft.v
  * testbench: Included in the generator source is a testbench that should be extracted and stored as tb_filter_idft.v
* IIR
  * Location: ./CEP/fpga_hardware/cores/dsp/iir/
  * Name: filter_iir.v
* FIR
  * Location: ./CEP/fpga_hardware/cores/dsp/fir/
  * Name: filter_fir.v


## Note:
All testing was done with the default values. Modify the gernerators at your own descreation, however additional modifcations to the wishbone bus interface may be nessary.

