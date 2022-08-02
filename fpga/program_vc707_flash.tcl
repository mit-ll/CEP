#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_vc707_flash.tcl
#// Project       : Common Evaluation Platform (CEP)
#// Description   : TCL script for automatic programming of VC707 CEP build via FLASH
#// Notes         : 
#//--------------------------------------------------------------------------------------

# Script to program the memory configuration device
open_hw_manager
connect_hw_server
open_hw_target

# Create MCS file
write_cfgmem  -format mcs -size 128 -interface BPIx16 -loadbit {up 0x00000000 "./generated-src/chipyard.fpga.vc707.VC707FPGATestHarness.RocketVC707CEPConfig/obj/VC707FPGATestHarness.bit" } -force -file "./generated-src/chipyard.fpga.vc707.VC707FPGATestHarness.RocketVC707CEPConfig/obj/VC707FPGATestHarness.mcs"

# Select and program the Flash device
create_hw_cfgmem -hw_device [lindex [get_hw_devices xc7vx485t_0] 0] [lindex [get_cfgmem_parts {mt28gu01gaax1e-bpi-x16}] 0]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7vx485t_0] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7vx485t_0] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7vx485t_0] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7vx485t_0] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7vx485t_0] 0]]
refresh_hw_device [lindex [get_hw_devices xc7vx485t_0] 0]
set_property PROGRAM.FILES [list "./generated-src/chipyard.fpga.vc707.VC707FPGATestHarness.RocketVC707CEPConfig/obj/VC707FPGATestHarness.mcs" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0]]
set_property PROGRAM.PRM_FILE {./generated-src/chipyard.fpga.vc707.VC707FPGATestHarness.RocketVC707CEPConfig/obj/VC707FPGATestHarness.prm} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0]]
set_property PROGRAM.BPI_RS_PINS {none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]] 
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
startgroup
if {![string equal [get_property PROGRAM.HW_CFGMEM_TYPE  [lindex [get_hw_devices] 0]] [get_property MEM_TYPE [get_property CFGMEM_PART [get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]]]] }  { create_hw_bitstream -hw_device [lindex [get_hw_devices] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices] 0]]; program_hw_devices [lindex [get_hw_devices] 0]; }; 
program_hw_cfgmem -hw_cfgmem [get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
close_hw_manager
quit