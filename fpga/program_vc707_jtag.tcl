#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_vc707_jtag.tcl
#// Project       : Common Evaluation Platform (CEP)
#// Description   : TCL script for automatic programming of VC707 CEP build via JTAG
#// Notes         : 
#//--------------------------------------------------------------------------------------
open_hw_manager
connect_hw_server
open_hw_target
set_property PROGRAM.FILE {./generated-src/chipyard.fpga.vc707.VC707FPGATestHarness.RocketVC707CEPConfig/obj/VC707FPGATestHarness.bit} [get_hw_devices xc7vx485t_0]
program_hw_devices
close_hw_manager
quit
