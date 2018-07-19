# CEP v1.1
Common Evaluation Platform

Release Notes:
* Directory structure has been reorganized (details below)
* Upgraded to the non-deprecated mor1kx (added as a subnodule)
* Verified with both the pre-built and build-your-own version of the Newlib toolchain as described on [http://openrisc.io](http://openrisc.io)
* In addition to test vectors for each of the cores, some additional test_software, such as "hello world", have been added for reference
* Expanded testbench (details below)
* Bug fixes and general code cleanup [Additional details in the git commit history]

## General Overview
* The CEP is targetted to be a representative, open source, System on a Chip (SoC) test platform.  Wherever possible, dependance on specific FPGA hardware has been avoided with a goal of ASIC synthesis.  However, there are two components cores (clkgen & ram_wb) will need to be modified to for your target

## Repository Directory Structure
```
CEP ----- fpga/
       |     |
       |     |-- constraints/ - Constraint file(s) for FPGA Development boards for which the CEP has been built
       |     |
       |     |-- implSummaries/ - .png snapshots of each core's utilization summary
       |     |
       |     |-- vc707example/ - Vivado project for targetting the CEP to the VC-707 FPGA Development Board
       |
       |-- hdl_cores/ - Source for all the components within the CEP (with the exception of the mor1kx and generated DSP).  
       |                All the blocks that implement algorithms also have corresponding test vectors.
       |
       |-- generated_dsp_code/  - Placeholder for the generated DSP code
       |
       |-- simulation/
       |     |
       |     |-- run_sim.do - TCL script for simulation of the CEP with modelsim (tested with Questa 10.6c)
       |     |
       |     |-- testbench/ - Testbench source files
       |
       |-- software/
       |     |
       |     |-- bootloader/ - board specific mor1k bootloaders
       |     |
       |     |-- test_software/ - Additional software tests beyond those provided with the individual cores
       |     |
       |     |-- utils/ - Utilities containing tools for creation of appropriated formatted images for memory 
       |                  initialization.  Makefile contained within
       |
       |-- submodules/ - Location of any git submodules used in the CEP (currently the mor1kx).  The 
                         --recurse-submodules switch should be used when cloning the CEP repo
```

## Install the or1k toolchain
Follow the instructions on [http://openrisc.io](http://openrisc.io) (tested with both pre-built and compiled versions)
        
## Compile the memory conversion utilities
Change to the `./software/utils` directory.  Run `make`.
                  
## Changing the program loaded into memory
The mor1k reads it's instructions from the ram_wb component, which is initialized using the verilog $readmemh method.  The initialization (.vmem) file is specified through the definition of the **SRAM\_INITIALIZATION\_FILE** verilog define. Setting of this value is included in both the simulation TCL script and `vc707example.xpr` vivado project.  Examples of how to create these files are included in the test\_software directory as well as the test vectors for the various algorithmic cores.  Please examine the corresponding Makefiles.

## Enabling/disabling cores
Modify via commenting/uncommenting the desired alogrithmic cores in `./hdl_cores/top/orpsoc-defines.v`.  By default, all the cores are included.  The ram\_wb component must be included, and it is recommended that you keep the UART to support debug.

## Note regarding DSP cores
Due to licensing, the generated source code for the DFT, IDFT, IIR, and IIR components are not included with the CEP repository.  Instructions for generating these cores can be found in the `./hdl_cores/dsp/README.md` file.  Once created per those instructions, the `run_sim.do` file and Vivado projects will need to be updated to point to these files.  Scripts assume that the DSP generated code has been placed in `<CEP_ROOT>/generared_dsp_code`

## Simulate using Modelsim
Edit `run_sim.do`.... change the **DESIGN_ROOT** and **GENERATED_DSP_ROOT** variables to point to checkedout CEP repo and genrated DSP files accordingly.

Once the desired program has been "loaded" into memory as described above, simulation is accomplished by running modelsim using following command in the ./simulation directory:
    `vsim -do run_sim.do`

Alternatively, a non-gui simulation can be run by simply adding the -c switch:
    `vsim -c -do run_sim.do`

## Synthesize using Vivado (tested with Vivado 2018.1)
Select the desired set of cores and the program to load as described above.

Change to the `./fpga/vc707example` directory

Launch vivado and load the `vc707example.xpr` project.

Add the generated DSP source code as noted above.

Select Generate Bitstream within the Vivado GUI.  Once the bitstream is generated, launch the hardware manager.  Option is to run via JTAG or load in the on-board BPI flash.

Flash instructions can be found at [https://scholar.princeton.edu/jbalkind/blog/programming-vc707-virtex-7-bpi-flash](https://scholar.princeton.edu/jbalkind/blog/programming-vc707-virtex-7-bpi-flash)

## Adding a core to the SoC
* Add a define that will control whether the core is included in the SoC to `./hdl_cores/top/orpsoc-defines.v`
* Add the core's bus address and width information to `./hdl_cores/top/orpsoc-params.v`
   * Use the Ethernet core as an example
   * Make sure to use an available address prefix for "XX_wb_adr"
   * Refer to the number of bytes actually used by the new core's bus interface to determine the address width
   * All other widths are the default of 32 bits
   * Update the configuration of the appropriate bus(es) (i.e., instruction, data, byte-wide)
      * Increment the number of slaves
      * Insert the new core in an empty slot (the data bus can hold up to 16 slaves)
         * If the desired bus doesn't have an empty slot, the easiest thing to do is to steal a slot from another slave.  Failing that, the bus arbiter (e.g., `./hdl_cores/arbiter/arbiter_dbus.v`) will need to be modified to support more slaves.
* In `./hdl_cores/top/orpsoc_top.v`
   * Add any pin connections required by the new core to the interface of this module
   * Create a set of bus slave wires to connect the new core and the appropriate bus arbiter
   * Connect the new core to the appropriate slot in the appropriate aribter using the wires created in the previous step
   * Update the bus aribter's defparams both in terms of numbering and adding defparams for the new core's "adr" and "add_width" parameters (this may already be done for data bus slaves up to the 16th slave).
   * Use conditional compilation statements and the define created earlier to conditionally instantiate the new core
      * Drive slave outputs to 0's if new core not defined
   * If the new core has an interrupt line, using the define, conditionally connect it to the processor's interrupt controller
* If the core requires pin I/O connections:
   * Bring the signals up through the hierarchy
   * Add the pinout information to the appropiate .xdc
* Add the HDL files to the list of project files in ModelSim and re-compile
* Add the HDL files to the list of project files in Vivado and re-build

## Addressing the accelerators
The accelerator cores are accessed via memory-mapped IO.  That is to say, you read from and write to accelerator cores just as you would memory, using word granularity addresses.  The base address for each core is defined for hardware in `./hdl_cores/top/orpsoc-params.v` and for software in the header file for that core, e.g., `./hdl_cores/aes/AES.h`.  Also defined in the software header file are aliases for specific offesets in a core's interface.  To understand how the hardware and software work together, see the Wishbone bus interface for the core, e.g., `./hdl_cores/aes/aes_top.v`.


------------------------------------------------------------------------------------
- DISTRIBUTION STATEMENT A. Approved for public release: distribution unlimited.
-
- This material is based upon work supported by the Assistant Secretary of Defense
- for Research and Engineering under Air Force Contract No. FA8721-05-C-0002
- and/or FA8702-15-D-0001. Any opinions, findings, conclusions or recommendations
- expressed in this material are those of the author(s) and do not necessarily
- reflect the views of the Assistant Secretary of Defense for Research and
- Engineering.
-
- © 2019 Massachusetts Institute of Technology.
-
- The software/firmware is provided to you on an As-Is basis
-
- Delivered to the U.S. Government with Unlimited Rights, as defined in DFARS Part
- 252.227-7013 or 7014 (Feb 2014). Notwithstanding any copyright notice, U.S.
- Government rights in this work are defined by DFARS 252.227-7013 or DFARS
- 252.227-7014 as detailed above. Use of this work other than as specifically
- authorized by the U.S. Government may violate any copyrights that exist in this
- work.
------------------------------------------------------------------------------------
