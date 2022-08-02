#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_vcu118_jtag.tcl
#// Project       : Common Evaluation Platform (CEP)
#// Description   : TCL script for automatic programming of the VCU118 CEP build via JTAG
#// Notes         : 
#//--------------------------------------------------------------------------------------
open_hw_manager
connect_hw_server
open_hw_target
set_property PROGRAM.FILE {./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness.bit} [get_hw_devices xcvu9p_0]
program_hw_devices
close_hw_manager
quit
