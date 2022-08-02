#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_vcu118_flash.tcl
#// Project       : Common Evaluation Platform (CEP)
#// Description   : TCL script for automatic programming of VCU118 CEP build
#// Notes         : T
#//--------------------------------------------------------------------------------------

# Script to program the memory configuration device
open_hw_manager
connect_hw_server
open_hw_target

# Create MCS file
write_cfgmem  -format mcs -size 128 -interface SPIx8 -loadbit {up 0x00000000 "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness.bit" } -force -file "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness.mcs"

# Select and program the Flash device
current_hw_device [get_hw_devices xcvu9p_0]
refresh_hw_device -update_hw_probes false [lindex [get_hw_devices xcvu9p_0] 0]
create_hw_cfgmem -hw_device [lindex [get_hw_devices xcvu9p_0] 0] [lindex [get_cfgmem_parts {mt25qu01g-spi-x1_x2_x4_x8}] 0]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
refresh_hw_device [lindex [get_hw_devices xcvu9p_0] 0]
set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.FILES [list "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness_primary.mcs" "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness_secondary.mcs" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.PRM_FILES [list "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness_primary.prm" "./generated-src/chipyard.fpga.vcu118.VCU118FPGATestHarness.RocketVCU118CEPConfig/obj/VCU118FPGATestHarness_secondary.prm" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
startgroup 
create_hw_bitstream -hw_device [lindex [get_hw_devices xcvu9p_0] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices xcvu9p_0] 0]]; program_hw_devices [lindex [get_hw_devices xcvu9p_0] 0]; refresh_hw_device [lindex [get_hw_devices xcvu9p_0] 0];
program_hw_cfgmem -hw_cfgmem [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
close_hw_manager
quit