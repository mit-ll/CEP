#//************************************************************************
#// File Name:      run_sim.do 
#// Program:        Common Evaluation Platform (CEP)
#// Description:    TCL script to build/execute the CEP testbench
#// Notes:          
#//************************************************************************

# Set the environment variables for pointing to the Xilinx libraries
set PATH_TO_XILINX_INSTALL          "/opt/Xilinx/Vivado/2018.1"
set PATH_TO_XILINX_VERILOG_SRC      "${PATH_TO_XILINX_INSTALL}/data/verilog/src"

# Set Project Specific Variables (assumes modelsim being run from the ./simulation directory and
# that the generated DSP source code has been placed in the <CEP BASE>/generated_dsp_code directory

set DESIGN_ROOT                     ".."
set GENERATED_DSP_ROOT              "../generated_dsp_code"
set CORES_ROOT                      "${DESIGN_ROOT}/hdl_cores"
set MOR1KX_ROOT                     "${DESIGN_ROOT}/submodules/mor1kx"
set TB_ROOT                         "${DESIGN_ROOT}/simulation/testbench"
set TB_LOG_OUT                      "${DESIGN_ROOT}/simulation/out"
set SRAM_INITIALIZATION_FILE        "${CORES_ROOT}/ram_wb/sram.vmem"

# Create working library
vlib work

# Create output directory
mkdir -p out

# Define for use during simulation
set CEP_INCLUDES                    "+incdir+${CORES_ROOT}/top+${MOR1KX_ROOT}/rtl/verilog+${MOR1KX_ROOT}/rtl/bench/verilog+${TB_ROOT}+${CORES_ROOT}/adv_debug_sys/Hardware/adv_dbg_if/rtl/verilog+${CORES_ROOT}/jtag_tap"
set CEP_DEFINES                     "+define+SRAM_INITIALIZATION_FILE=\"${SRAM_INITIALIZATION_FILE}\""

# Compile the Xilinx "Globals", Unisim, and such
vlog ${PATH_TO_XILINX_VERILOG_SRC}/glbl.v
vlog ${PATH_TO_XILINX_VERILOG_SRC}/unisim_retarget_comp.v

# Compile the testbench
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/bench/verilog/mor1kx_monitor.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/orpsoc_testbench.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/uart_decoder.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${TB_ROOT}/wiredelay.v

# Compile the MOR1KX
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cpu_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_dcache.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_fetch_tcm_prontoespresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cpu_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_fetch_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_branch_prediction.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_ticktimer.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_decode.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cpu.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_wb_mux_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_ctrl_prontoespresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_rf_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_pic.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cpu_prontoespresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_decode_execute_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx-defines.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_lsu_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cache_lru.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_branch_predictor_saturation_counter.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_true_dpram_sclk.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_icache.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_fetch_prontoespresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_rf_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx-sprs.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_wb_mux_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_bus_if_wb32.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_ctrl_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_dmmu.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_branch_predictor_simple.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_addsub.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_muldiv.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_f2i.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_rnd.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_cmp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/pfpu32/pfpu32_i2f.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_branch_predictor_gshare.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_fetch_espresso.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_execute_alu.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_store_buffer.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_lsu_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_simple_dpram_sclk.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_execute_ctrl_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_pcu.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_immu.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_ctrl_cappuccino.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${MOR1KX_ROOT}/rtl/verilog/mor1kx_cfgrs.v

# Compile the CEP Cores
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/aes/aes_128.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/aes/aes_192.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/aes/aes_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/aes/round.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/aes/table.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/arbiter/arbiter_bytebus.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/arbiter/arbiter_dbus.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/arbiter/arbiter_ibus.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/clkgen/clkgen.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/crp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/des3/des3_test_ao.v
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
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/dsp/dft/dft_top_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/dsp/fir/fir_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/dsp/idft/idft_top_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/dsp/iir/iir_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/cacode_test.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/cacode.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/gps_clkgen_test.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/gps_clkgen.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/gps_test.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/gps_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/gps.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/gps/pcode.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/md5/md5_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/md5/pancham_round.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/md5/pancham.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/ram_wb/ram_wb_b3.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/ram_wb/ram_wb.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/adder.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/blockmem1r1w.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/blockmem2r1wptr.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/blockmem2r1w.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/blockmem2rptr1w.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/modexp_core.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/modexp_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/modexp.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/montprod.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/residue.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/shl.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/rsa/rtl/shr.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/sha256/sha256_k_constants.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/sha256/sha256_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/sha256/sha256.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/sha256/sha256_w_mem.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/raminfr.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_defines.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_receiver.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_regs.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_rfifo.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_sync_flops.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_tfifo.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_transmitter.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/uart/uart_wb.v

# Compile the generated DSP code
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${GENERATED_DSP_ROOT}/dft_top.v
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${GENERATED_DSP_ROOT}/idft_top.v
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${GENERATED_DSP_ROOT}/IIR_filter.v
vlog -sv ${CEP_INCLUDES} ${CEP_DEFINES} ${GENERATED_DSP_ROOT}/FIR_filter.v

# Compile the CEP Top Level
vlog ${CEP_INCLUDES} ${CEP_DEFINES} ${CORES_ROOT}/top/orpsoc_top.v

# Force a library refresh to ensure the package is up to date
vlog -work ${DESIGN_ROOT}/simulation/work -refresh -force_refresh

# "Load" the simulation
vsim work.orpsoc_testbench +notimingchecks -c -noglitch -novopt -gLOG_DIR=${TB_LOG_OUT} +trace_enable -l RUN.log; 

# Enable logging
log -r /*

# Run the simulation
run -all
