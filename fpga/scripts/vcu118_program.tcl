# Script to program the VCU-118 memory configuration device
open_hw

# check the number of parameters
if {$argc < 1 || $argc > 2} {
	puts $argc
	puts {Error: Invalid number of arguments}
	puts {Usage: vcu118_program.tcl mcsfile [remote_ip]}
	exit 1
}

lassign $argv mcsfile remote_ip

# Connected to a Local Board
if {$argc == 1} {
	connect_hw_server
# Connect to a remote board
} elseif {$argc == 2} {
	connect_hw_server -url $remote_ip
}

# Connect to the hardware target
open_hw_target

# Add the memory configuration device
create_hw_cfgmem -hw_device [lindex [get_hw_devices xcvu9p_0] 0] [lindex [get_cfgmem_parts {mt25qu01g-spi-x1_x2_x4_x8}] 0]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
refresh_hw_device [lindex [get_hw_devices xcvu9p_0] 0]

# Specify the programming parameters
set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.FILES [list ${mcsfile}_primary.mcs ${mcsfile}_secondary.mcs] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.PRM_FILES [list ${mcsfile}_primary.prm ${mcsfile}_secondary.prm] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]

# Program the memory devices
startgroup 
create_hw_bitstream -hw_device [lindex [get_hw_devices xcvu9p_0] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices xcvu9p_0] 0]]; program_hw_devices [lindex [get_hw_devices xcvu9p_0] 0]; refresh_hw_device [lindex [get_hw_devices xcvu9p_0] 0];
program_hw_cfgmem -hw_cfgmem [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcvu9p_0] 0]]
endgroup

# Close things out
close_hw
quit


