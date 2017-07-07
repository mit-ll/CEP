# CEP
Common Evaluation Platform

## Simualate using ModelSim
* Comment-out the synthesis define in fpga_hardware/cores/top/orpsoc-defines.v
* Run `vsim` to compile the Verilog files into the `work` library. If this is your first time trying to compile in ModelSim, you may need to create a `work` library (after you delete the empty one that points to nothing).
* Exit ModelSim
* Compile the desired program, convert it to a bin file, then convert the bin file to a vmem file.
* Copy the vmem file to fpga_hardware/sram.vmem
* Run `sh runVSIM.sh` in fpga_hardware
* Once vsim finishes loading the design, type `run -all`

## Synthesize using Vivado
* Uncomment the synthesis define in fpga_hardware/cores/top/orpsoc-defines.v
* Start Vivado and create a new project (ideally targeting the VC707 dev. board).
