#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_arty100t_jtag.tcl
#// Project       : Common Evaluation Platform (CEP)
#// Description   : TCL script for automatic programming of Arty100t CEP build via JTA
#// Notes         : 
#//--------------------------------------------------------------------------------------
open_hw_manager
connect_hw_server
open_hw_target
set_property PROGRAM.FILE {./generated-src/chipyard.fpga.arty100t.Arty100TFPGATestHarness.RocketArty100TCEPConfig/obj/Arty100TFPGATestHarness.bit} [get_hw_devices xc7a100t_0]
program_hw_devices
close_hw_manager
quit
