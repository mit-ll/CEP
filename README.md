# CEP
Common Evaluation Platform

## Changing the program loaded into memory

Now that you have simulated the provided program or built and run an FPGA implementation of the default program, you may want to simulate/run your own programs.  In order to do that, you will need to write (in C or assembly), compile (using the OR1K gcc toolchain and newlib), and convert the resulting binary into a vmem file suitable for loading directly loading to memory.

Here are the basic steps of the compile and conversion process:
* Compile your program using the [OR1K toolchain](http://opencores.org/or1k/OpenRISC_GNU_tool_chain)
* Convert the resulting binary file to a bin file using `or1k-elf-objdump -O binary`
* Convert the bin file to a vmem file using the `bin2vmem` utility in `software/utils`
* Rename the resulting sram file `sram.vmem` and replace the file in `fpga_hardware/cores/ram_wb`
* If you get illegal instruction exceptions (0x700), you may need to move `sram.vmem` to `fpga_hardware/`

## Enabling/disabling cores
Modify via commenting/uncommenting the desired cores in fpga_hardware/cores/top/orpsoc-defines.v.

## Simualate using ModelSim
* Comment-out the synthesis define in fpga_hardware/cores/top/orpsoc-defines.v
* Run `vsim` to compile the Verilog files into the `work` library. If this is your first time trying to compile in ModelSim, you may need to create a `work` library (after you delete the empty one that points to nothing).
* Exit ModelSim
* Compile the desired program, convert it to a bin file, then convert the bin file to a vmem file.
* Copy the vmem file to fpga_hardware/sram.vmem
* Run `sh runVSIM.sh` in fpga_hardware
* Once vsim finishes loading the design, type `run -all`
* To change simulation parameters, such as VCD printing or test name, modify fpga_hardware/cores/bench/test-defines.v

## Synthesize using Vivado
* Uncomment the synthesis define in fpga_hardware/cores/top/orpsoc-defines.v
* Start Vivado and create a new project (ideally targeting the VC707 dev. board).
    * Targeting a different development board requires that you create a .xdc file (see fpga_hardware/backend/vc707.xdc for an example) with pinout and constraints for that board.  It may also require you to modify fpga_hardware/core/clkgen/clkgen.v if the input clock is not 200 MHz or doesn't use differential signaling.
    
## Adding a core to the SoC
* Add a define that will control whether the core is included in the SoC to fpga_hardware/cores/top/orpsoc-defines.v
* Add the core's bus address and width information to fpga_hardware/cores/top/orpsoc-params.v
   * Use the Ethernet core as an example
   * Make sure to use an available address prefix for "XX_wb_adr"
   * Refer to the number of bytes actually used by the new core's bus interface to determine the address width
   * All other widths are the default of 32 bits
   * Update the configuration of the appropriate bus(es) (i.e., instruction, data, byte-wide)
      * Increment the number of slaves
      * Insert the new core in an empty slot (the data bus can hold up to 16 slaves)
         * If the desired bus doesn't have an empty slot, the easiest thing to do is to steal a slot from another slave.  Failing that, the bus arbiter (e.g., fpga_hardware/cores/arbiter/arbiter_dbus.v) will need to be modified to support more slaves.
* In cores/top/orpsoc_top.v
   * Add any pin connections required by the new core to the interface of this module
   * Create a set of bus slave wires to connect the new core and the appropriate bus arbiter
   * Connect the new core to the appropriate slot in the appropriate aribter using the wires created in the previous step
   * Update the bus aribter's defparams both in terms of numbering and adding defparams for the new core's "adr" and "add_width" parameters (this may already be done for data bus slaves up to the 16th slave).
   * Use conditional compilation statements and the define created earlier to conditionally instantiate the new core
      * Drive slave outputs to 0's if new core not defined
   * If the new core has an interrupt line, using the define, conditionally connect it to the processor's interrupt controller
* If the core requires pin I/O connections:
   * Bring the signals up through the hierarchy
   * Add the pinout information to the appropiate .xdc file in fpga_hardware/backend/
* Add the HDL files to the list of project files in ModelSim and re-compile
* Add the HDL files to the list of project files in Vivado and re-build
