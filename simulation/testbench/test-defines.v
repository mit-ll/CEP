//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : test-defines.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Top-level testbench include file for the AXI4-Lite / MOR1KX variant of the CEP
// Notes        : Core licensing information may be found in licenseLog.txt
//

`define TEST_NAME_STRING "mor1kx"
`define SIMULATOR_MODELSIM

`define BOARD_CLOCK_PERIOD 20       // 50MHz

// Used by the mor1kx monitor component
`define MOR1KX_CPU_PIPELINE cappuccino
`define MOR1KX_INST orpsoc_top_inst.mor1kx_top_axi4lite_inst.mor1kx_inst
`define CPU_WRAPPER `MOR1KX_INST.mor1kx_cpu

// Parameters for capturing VCD files
//`define VCD
`define VCD_DEPTH 0 // 0 = all
`define VCD_PATH tb_orpsoc_top.`CPU_WRAPPER
//`define PROCESSOR_MONITOR_ENABLE_LOGS
