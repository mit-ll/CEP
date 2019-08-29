#//************************************************************************
#// Copyright (C) 2019 Massachusetts Institute of Technology
#//
#// File Name:      run_sim_des3_axi4lite.do 
#// Program:        Common Evaluation Platform (CEP)
#// Description:    TCL script to build/execute the CEP testbench
#// Notes:          This testbench will specifically target the DES3 core
#//                 wrapped within AXI4-lite 
#//************************************************************************

# Set Project Specific Variables (assumes modelsim being run from the ./simulation directory and
# that the generated DSP source code has been placed in the <CEP BASE>/generated_dsp_code directory
set DESIGN_ROOT                     ".."
set GENERATED_DSP_ROOT              "../generated_dsp_code"
set CORES_ROOT                      "${DESIGN_ROOT}/hdl_cores"
set TB_ROOT                         "${DESIGN_ROOT}/simulation/testbench"
set TB_LOG_OUT                      "${DESIGN_ROOT}/simulation/out"
set SRAM_INITIALIZATION_FILE        "${CORES_ROOT}/ram_wb/sram.vmem"

# Create working library
rm -rf work
vlib work

# Create output directory
mkdir -p out

# Define for use during simulation
set CEP_INCLUDES                    "+incdir+${CORES_ROOT}/top+incdir+${CORES_ROOT}/pulp_axi/include"
set CEP_DEFINES                     "+define+SRAM_INITIALIZATION_FILE=\"${SRAM_INITIALIZATION_FILE}\"+define+DEBUG0"

# Include the necessary AXI logic
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/pulp_axi/src/axi_pkg.sv
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/pulp_axi/src/axi_intf.sv
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/pulp_axi/src/axi_test.sv

# Compile the CEP Cores
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3_top_wb.v
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

vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/wb2axip/rtl/wbarbiter.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/wb2axip/rtl/axilwr2wbsp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/wb2axip/rtl/axilrd2wbsp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/wb2axip/rtl/axlite2wbsp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3_top_axi4lite.v

# Compile the testbench
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/tb_support.sv
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/tb_axi4lite_des3.sv

# Force a library refresh to ensure the package is up to date
vlog -work ${DESIGN_ROOT}/simulation/work -refresh -force_refresh

# Set optimization level
vopt +acc work.tb_axi4lite_des3 -o dbugver

# "Load" the simulation
vsim dbugver -classdebug +notimingchecks -c +trace_enable -l RUN.log; 

# Run the simulation
run -all

# Quit with coverage statistics
quit -code [coverage attribute -name TESTSTATUS -concise]