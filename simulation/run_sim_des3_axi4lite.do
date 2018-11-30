#//************************************************************************
#// Copyright (C) 2018 Massachusetts Institute of Technology
#//
#// File Name:      run_sim_des3_axi4lite.do 
#// Program:        Common Evaluation Platform (CEP)
#// Description:    TCL script to build/execute the CEP testbench
#// Notes:          This testbench will specifically target the DES3 core
#//                 wrapped within AXI4-lite 
#//************************************************************************

# Set the environment variables for pointing to the Xilinx libraries
set PATH_TO_XILINX_INSTALL          "/opt/Xilinx/Vivado/2018.1"
set PATH_TO_XILINX_VERILOG_SRC      "${PATH_TO_XILINX_INSTALL}/data/verilog/src"

# Set Project Specific Variables (assumes modelsim being run from the ./simulation directory and
# that the generated DSP source code has been placed in the <CEP BASE>/generated_dsp_code directory

set DESIGN_ROOT                     ".."
set GENERATED_DSP_ROOT              "../generated_dsp_code"
set AXI_ROOT                        "${DESIGN_ROOT}/submodules/axi"
set CORES_ROOT                      "${DESIGN_ROOT}/hdl_cores"
set MOR1KX_ROOT                     "${DESIGN_ROOT}/submodules/mor1kx"
set TB_ROOT                         "${DESIGN_ROOT}/simulation/testbench"
set TB_LOG_OUT                      "${DESIGN_ROOT}/simulation/out"
set SRAM_INITIALIZATION_FILE        "${CORES_ROOT}/ram_wb/sram.vmem"

# Create working library
rm -rf work
vlib work

# Create output directory
mkdir -p out

# Define for use during simulation
set CEP_INCLUDES                    "+incdir+${CORES_ROOT}/top+${MOR1KX_ROOT}/rtl/verilog+${MOR1KX_ROOT}/rtl/bench/verilog+${TB_ROOT}"
set CEP_DEFINES                     "+define+SRAM_INITIALIZATION_FILE=\"${SRAM_INITIALIZATION_FILE}\"+define+DEBUG0"

# Compile the Xilinx "Globals", Unisim, and such
#vlog ${PATH_TO_XILINX_VERILOG_SRC}/glbl.v
#vlog ${PATH_TO_XILINX_VERILOG_SRC}/unisim_retarget_comp.v

# Include the necessary AXI logic
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${AXI_ROOT}/src/axi_pkg.sv
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${AXI_ROOT}/src/axi_intf.sv
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${AXI_ROOT}/src/axi_test.sv

# Compile the CEP Cores
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/key_sel3.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox1.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox2.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox3.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox4.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox5.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox6.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox7.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/sbox8.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/crp.v
vcom ${CORES_ROOT}/bonfire_axi4l2wb/bonfire_axi4l2wb.vhd
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3_top_axi4lite.sv

# Compile the testbench
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/tb_axi_lite_support.sv
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/tb_axi_lite_des3.sv

# Force a library refresh to ensure the package is up to date
vlog -work ${DESIGN_ROOT}/simulation/work -refresh -force_refresh

# Set optimization level
vopt +acc work.tb_axi_lite_des3 -o dbugver

# "Load" the simulation
vsim dbugver -classdebug +notimingchecks -c -noglitch +trace_enable -l RUN.log; 

# Run the simulation
run -all

# Quit with coverage statistics
quit -code [coverage attribute -name TESTSTATUS -concise]
