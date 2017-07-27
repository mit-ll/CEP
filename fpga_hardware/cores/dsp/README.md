# CEP-DSP: Core blocks

The core DSP blocks (DFT/IDFT/IIR/FIR) were generated using the free online hardware generators provided by the [Spiral Project](http://www.spiral.net/). Due to licensing restrictions, we cannot include the generated files for the DSP blocks. To include these cores in the SoC, you must regenerate the cores and add the source files to the CEP file structure as described:

## Generator Websites
* DFT/iDFT: [Spiral Project DFT Gen](http://www.spiral.net/hardware/dftgen.html)
* IIR/FIR: [Spiral Project Filter Gen](http://www.spiral.net/hardware/filter.html)

## Generator Settings
* Note: We indicate deviations from the default generator setting as **`modified`**

* DFT/iDFT:

Core(s)  |DFT |IDFT
---------|---:|----:
**Problem Specification**||
Transform Size|64|64
Direction|Forward|**`Inverse`**
Data type|Fixed Point|Fixed Point
||16 bits|16 bits
||unscaled|unscaled
**Parameters Controlling Implementation**||
Architecture|fully streaming|fully streaming
radix|2|2
streaming width|2|2
data ordering|Natural in / natural out|Natural in / natural out
BRAM budget|1000|1000
Permutation Method|**`DATE'09 [4] (patent Free)`**|**`DATE'09 [4] (patent Free)`**

* IIR/FIR

Core(s)  |IIR |FIR
---------|---:|---:
Example Filter| 6| 6
Filter Taps a_k|1.000000e+000|**`1`**
||8.274631e-016|**`0`**
||1.479294e+000|**`0`**
||1.089388e-015|**`0`**
||7.013122e-001|**`0`**
||4.861197e-016|**`0`**
||1.262132e-001|**`0`**
||9.512539e-017|**`0`**
||7.898376e-003|**`0`**
||-4.496047e-018|**`0`**
||1.152699e-004|**`0`**
||-6.398766e-021|**`0`**
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
Debug Output|**`Off`**|**`Off`**

## Incorporating the Generated Source Files
* DFT
  * Save the generated Verilog file to `./CEP/fpga_hardware/cores/dsp/dft/filter_dft.v`
  * Remove the testbench module from the generated Verilog file and save it as `./CEP/fpga_hardware/cores/dsp/dft/tb_filter_dft.v`
* iDFT
  * Save the generated Verilog file to `./CEP/fpga_hardware/cores/dsp/idft/filter_idft.v`
  * Remove the testbench module from the generated Verilog file and save it as `./CEP/fpga_hardware/cores/dsp/idft/tb_filter_idft.v`
* IIR
  * Save the generated Verilog file to `./CEP/fpga_hardware/cores/dsp/iir/filter_iir.v`
* FIR
  * Save the generated Verilog file to `./CEP/fpga_hardware/cores/dsp/fir/filter_fir.v`

## Warning

Modify the parameters used to generate the cores at your own risk.  Some parameter changes will require you to modify the Wishbone bus interface and software drivers.

