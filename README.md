# CEP - Common Evaluation Platform (v1.2)

© 2018 Massachusetts Institute of Technology.

Release Notes:

v1.0 - Initial release

v1.1 - (19 July 2018)
* Directory structure has been reorganized (details below)
* Upgraded to the non-deprecated mor1kx (added as a subnodule)
* Verified with both the pre-built and build-your-own version of the Newlib toolchain as described on [http://openrisc.io](http://openrisc.io)
* In addition to test vectors for each of the cores, some additional test_software, such as "hello world", have been added for reference
* Expanded testbench (details below)
* Bug fixes and general code cleanup [Additional details in the git commit history]

v1.1.1 - (27 September 2018)
* Added CEP\_utilization\_placed.rpt in implSummaries directory.  This summarizes utlization of the CEP v1.1 targetted to the VC707 using Vivado 2018.1.

v1.2 - (15 November 2018)
* Major Update: The underlying bus has been converted from Wishbone-B4 to AXI4-Lite.  All cores as well as the MOR1KX has been wrapped with translation modules.  All the wrapping logic and interconnect are now in SystemVerilog.
* Regression Suite: In additon to each core's unit tests, a regression test suite has been included.  When compiled by the or1k toolchain, it be loaded/synthesized into the CEP RAM block and run at boot time.
* Modelsim Testing: Unit-level and system-level modelsim-based testbenches added
* GPS clkgen: The GPS clock gen component has been moved to the top level of the CEP, simplifying its replacement when targetting an ASIC.
* Misc. bug fixes

## General Overview
* The CEP is targetted to be a representative, open source, System on a Chip (SoC) surrogate platform.  Wherever possible, dependance on specific FPGA hardware has been avoided towards a goal of ASIC synthesis.  However, there are two components cores (clkgen & ram) will need to be modified to for your target.

## Repository Directory Structure
```
CEP ----- fpga/
       |     |
       |     |-- constraints/ - Constraint file(s) for FPGA Development boards for which the CEP has been built
       |     |
       |     |-- implSummaries/ - .png snapshots of each core's utilization summary (v1.0) and Vivado 2018.2 reports for the CEP on the VC707 (v1.2)
       |     |
       |     |-- vc707example/ - Vivado project for targetting the CEP to the VC-707 FPGA Development Board
       |
       |-- hdl_cores/ - Source for all the components within the CEP (with the exception of the mor1kx, generated DSP, and AXI4-Lite support packages).  
       |                All the blocks that implement algorithms also have corresponding test vectors.
       |
       |-- generated_dsp_code/  - Placeholder for the generated DSP code
       |
       |-- simulation/
       |     |
       |     |-- run_regression.sh - Shell script to run unit tests for all CEP cores.  The CEP test, run_sim_CEP.do, is explictly excluded and should be run manually, if so desired.
       |     |
       |     |-- run_sim_<xxx>.do - Modelsim TCL script for running individual core tests or for the entire CEP.
       |     |
       |     |-- waves_<xxx>.do - Modelsim TCL script for adding an initial set of waveforms for the particular test.  The <xxx> should correspond to the run_sim_<xxx>.do script.
       |     |
       |     |-- testbench/ - Testbench source files
       |
       |-- software/
       |     |
       |     |-- bootloader/ - board specific mor1kx bootloaders
       |     |
       |     |-- test_software/ - Additional software tests beyond those provided with the individual cores.  Includes regression test suite.
       |     |
       |     |-- utils/ - Utilities containing tools for creation of appropriated formatted images for memory initialization.  Makefile contained within.
       |
       |-- submodules/ - Location of any git submodules used in the CEP.  The 
                         --recurse-submodules switch should be used when cloning the CEP repo.
```

## Cloning the CEP Repository
The CEP relies on submodules.  As such, ensure the `--recurse-submodules` switch is used when cloning the repository.  Alternatively, change to the root directory of the repository after cloning and execute the following:
`git submodule update --init --recursive`

## Install the or1k toolchain
Follow the instructions on [http://openrisc.io](http://openrisc.io) (tested with both pre-built and compiled versions)
        
## Compile the memory conversion utilities
Change to the `./software/utils` directory.  Run `make`.  These utilities translate the compiled or1k executables into a form that can be used to initialize the SRAM.
                  
## Changing the program loaded into memory
The mor1k reads it's instructions from the raml component, which is initialized using the verilog $readmemh method.  The initialization (.vmem) file is specified through the definition of the **SRAM\_INITIALIZATION\_FILE** verilog define. Setting of this value is included in both the Modelsim simulation TCL scripts and `vc707example.xpr` Vivado project.  Examples of how to create these files are included in the test\_software directory as well as the test vectors for the various algorithmic cores.  Please examine the corresponding Makefiles.

## Enabling/disabling cores
Modify via commenting/uncommenting the desired alogrithmic cores in `./hdl_cores/top/orpsoc-defines.v`.  Mandatory cores are explictly identified (e.g., mor1kx, RAM, and UART).

## Note regarding DSP cores
Due to licensing, the generated source code for the DFT, IDFT, IIR, and IIR components are not included with the CEP repository.  Instructions for generating these cores can be found in the [./hdl_cores/dsp/README.md](./hdl_cores/dsp/README.md) file.  Scripts assume that the DSP generated code has been placed in `<CEP_ROOT>/generared_dsp_code`.

## Simulate using Modelsim
Edit `run_sim_<XXX>.do`.... change the **DESIGN_ROOT** and **GENERATED_DSP_ROOT** variables to point to checkedout CEP repo and generated DSP files accordingly.

Once the desired program has been "loaded" into memory as described above, simulation is accomplished by running modelsim using following command in the ./simulation directory:
    `vsim -do run_sim.do`

Alternatively, a non-gui simulation can be run by simply adding the -c switch:
    `vsim -c -do run_sim.do`

## Synthesize using Vivado (tested with Vivado 2018.2)
Select the desired set of cores and the program to load as described above.

Change to the `./fpga/vc707example` directory

Launch vivado and load the `vc707example.xpr` project.

Add the generated DSP source code as noted above.

Select Generate Bitstream within the Vivado GUI.  Once the bitstream is generated, launch the hardware manager.  Option is to run via JTAG or load in the on-board BPI flash.

Flash instructions can be found at [https://scholar.princeton.edu/jbalkind/blog/programming-vc707-virtex-7-bpi-flash](https://scholar.princeton.edu/jbalkind/blog/programming-vc707-virtex-7-bpi-flash).  This will require an `.mcs` file which can be generated using the `vc707example.tcl` script.

The `vc707example.tcl` script can be executed using the `source vc707example.tcl` command in the Vivado TCL console.

## Using the CEP on the VC707
Following programming of the CEP into BPI flash, the board can be rebooted with all output going to the USB-based UART.

## Adding a core to the SoC
* Adding a custom core is possible and the authors would recommend using the existing cores as an example.  The `orpsoc_top.sv` and `orpsoc-defines.sv` files would be a good place to start.  Examples of how to address the cores can be found in the unit-level test code as well as the regression test suite.

## Licensing
The CEP has been developed by selecting components with non-viral, open source licensing.  Specific licensing information can be found in [licenseLog.txt](./licenseLog.txt) in the root CEP directory.

DISTRIBUTION STATEMENT A. Approved for public release: distribution unlimited.

This material is based upon work supported by the Assistant Secretary of Defense for Research and Engineering under Air Force Contract No. FA8721-05-C-0002 and/or FA8702-15-D-0001. Any opinions, findings, conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the Assistant Secretary of Defense for Research and Engineering.

© 2018 Massachusetts Institute of Technology.

The software/firmware is provided to you on an As-Is basis.

Delivered to the U.S. Government with Unlimited Rights, as defined in DFARS Part 252.227-7013 or 7014 (Feb 2014). Notwithstanding any copyright notice, U.S. Government rights in this work are defined by DFARS 252.227-7013 or DFARS 252.227-7014 as detailed above. Use of this work other than as specifically authorized by the U.S. Government may violate any copyrights that exist in this work.
