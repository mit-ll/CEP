//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//*****************************************************************************
// (c) Copyright 2009 - 2010 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
//
//*****************************************************************************
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor             : Xilinx
// \   \   \/     Version            : 4.0
//  \   \         Application        : MIG
//  /   /         Filename           : sim_tb_top.v
// /___/   /\     Date Last Modified : $Date: 2011/06/07 13:45:16 $
// \   \  /  \    Date Created       : Tue Sept 21 2010
//  \___\/\___\
//
// Device           : 7 Series
// Design Name      : DDR3 SDRAM
// Purpose          :
//                   Top-level testbench for testing DDR3.
//                   Instantiates:
//                     1. IP_TOP (top-level representing FPGA, contains core,
//                        clocking, built-in testbench/memory checker and other
//                        support structures)
//                     2. DDR3 Memory
//                     3. Miscellaneous clock generation and reset logic
//                     4. For ECC ON case inserts error on LSB bit
//                        of data from DRAM to FPGA.
// Reference        :
// Revision History :
//*****************************************************************************
`timescale 1ns/10ps
`include "v2c_cmds.incl"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_top.incl"
//
`include "config.v"

`timescale 1ps/100fs

module cep_tb;


   //***************************************************************************
   // Traffic Gen related parameters
   //***************************************************************************
   parameter SIMULATION            = "TRUE";
   parameter BEGIN_ADDRESS         = 32'h00000000;
   parameter END_ADDRESS           = 32'h00000fff;
   parameter PRBS_EADDR_MASK_POS   = 32'hff000000;

   //***************************************************************************
   // The following parameters refer to width of various ports
   //***************************************************************************
   parameter COL_WIDTH             = 10;
                                     // # of memory Column Address bits.
   parameter CS_WIDTH              = 1;
                                     // # of unique CS outputs to memory.
   parameter DM_WIDTH              = 8;
                                     // # of DM (data mask)
   parameter DQ_WIDTH              = 64;
                                     // # of DQ (data)
   parameter DQS_WIDTH             = 8;
   parameter DQS_CNT_WIDTH         = 3;
                                     // = ceil(log2(DQS_WIDTH))
   parameter DRAM_WIDTH            = 8;
                                     // # of DQ per DQS
   parameter ECC                   = "OFF";
   parameter RANKS                 = 1;
                                     // # of Ranks.
   parameter ODT_WIDTH             = 1;
                                     // # of ODT outputs to memory.
   parameter ROW_WIDTH             = 14;
                                     // # of memory Row Address bits.
   parameter ADDR_WIDTH            = 28;
                                     // # = RANK_WIDTH + BANK_WIDTH
                                     //     + ROW_WIDTH + COL_WIDTH;
                                     // Chip Select is always tied to low for
                                     // single rank devices
   //***************************************************************************
   // The following parameters are mode register settings
   //***************************************************************************
   parameter BURST_MODE            = "8";
                                     // DDR3 SDRAM:
                                     // Burst Length (Mode Register 0).
                                     // # = "8", "4", "OTF".
                                     // DDR2 SDRAM:
                                     // Burst Length (Mode Register).
                                     // # = "8", "4".
   parameter CA_MIRROR             = "OFF";
                                     // C/A mirror opt for DDR3 dual rank
   
   //***************************************************************************
   // The following parameters are multiplier and divisor factors for PLLE2.
   // Based on the selected design frequency these parameters vary.
   //***************************************************************************
   parameter CLKIN_PERIOD          = 5000;
                                     // Input Clock Period


   //***************************************************************************
   // Simulation parameters
   //***************************************************************************
   parameter SIM_BYPASS_INIT_CAL   = "FAST";
                                     // # = "SIM_INIT_CAL_FULL" -  Complete
                                     //              memory init &
                                     //              calibration sequence
                                     // # = "SKIP" - Not supported
                                     // # = "FAST" - Complete memory init & use
                                     //              abbreviated calib sequence

   //***************************************************************************
   // IODELAY and PHY related parameters
   //***************************************************************************
   parameter TCQ                   = 100;
   //***************************************************************************
   // IODELAY and PHY related parameters
   //***************************************************************************
   parameter RST_ACT_LOW           = 0;
                                     // =1 for active low reset,
                                     // =0 for active high.

   //***************************************************************************
   // Referece clock frequency parameters
   //***************************************************************************
   parameter REFCLK_FREQ           = 200.0;
                                     // IODELAYCTRL reference clock frequency
   //***************************************************************************
   // System clock frequency parameters
   //***************************************************************************
   parameter tCK                   = 1250;
                                     // memory tCK paramter.
                     // # = Clock Period in pS.
   parameter nCK_PER_CLK           = 4;
                                     // # of memory CKs per fabric CLK

   
   //***************************************************************************
   // AXI4 Shim parameters
   //***************************************************************************
   parameter C_S_AXI_ID_WIDTH              = 4;
                                             // Width of all master and slave ID signals.
                                             // # = >= 1.
   parameter C_S_AXI_ADDR_WIDTH            = 30;
                                             // Width of S_AXI_AWADDR, S_AXI_ARADDR, M_AXI_AWADDR and
                                             // M_AXI_ARADDR for all SI/MI slots.
                                             // # = 32.
   parameter C_S_AXI_DATA_WIDTH            = 64;
                                             // Width of WDATA and RDATA on SI slot.
                                             // Must be <= APP_DATA_WIDTH.
                                             // # = 32, 64, 128, 256.
   parameter C_S_AXI_SUPPORTS_NARROW_BURST = 0;
                                             // Indicates whether to instatiate upsizer
                                             // Range: 0, 1


   //***************************************************************************
   // Debug and Internal parameters
   //***************************************************************************
   parameter DEBUG_PORT            = "OFF";
                                     // # = "ON" Enable debug signals/controls.
                                     //   = "OFF" Disable debug signals/controls.
   //***************************************************************************
   // Debug and Internal parameters
   //***************************************************************************
   parameter DRAM_TYPE             = "DDR3";

    

  //**************************************************************************//
  // Local parameters Declarations
  //**************************************************************************//

  localparam real TPROP_DQS          = 0.00;
                                       // Delay for DQS signal during Write Operation
  localparam real TPROP_DQS_RD       = 0.00;
                       // Delay for DQS signal during Read Operation
  localparam real TPROP_PCB_CTRL     = 0.00;
                       // Delay for Address and Ctrl signals
  localparam real TPROP_PCB_DATA     = 0.00;
                       // Delay for data signal during Write operation
  localparam real TPROP_PCB_DATA_RD  = 0.00;
                       // Delay for data signal during Read operation

  localparam MEMORY_WIDTH            = 8;
  localparam NUM_COMP                = DQ_WIDTH/MEMORY_WIDTH;
  localparam ECC_TEST 		   	= "OFF" ;
  localparam ERR_INSERT = (ECC_TEST == "ON") ? "OFF" : ECC ;
  

  localparam real REFCLK_PERIOD = (1000000.0/(2*REFCLK_FREQ));
  localparam RESET_PERIOD = 200000; //in pSec  
  localparam real SYSCLK_PERIOD = tCK;
    
    

  //**************************************************************************//
  // Wire Declarations
  //**************************************************************************//
  reg                                sys_rst_n;
  wire                               sys_rst;


  reg                     sys_clk_i;

  reg clk_ref_i;

  
  wire                               ddr3_reset_n;
  wire [DQ_WIDTH-1:0]                ddr3_dq_fpga;
  wire [DQS_WIDTH-1:0]               ddr3_dqs_p_fpga;
  wire [DQS_WIDTH-1:0]               ddr3_dqs_n_fpga;
  wire [ROW_WIDTH-1:0]               ddr3_addr_fpga;
  wire [3-1:0]              ddr3_ba_fpga;
  wire                               ddr3_ras_n_fpga;
  wire                               ddr3_cas_n_fpga;
  wire                               ddr3_we_n_fpga;
  wire [1-1:0]               ddr3_cke_fpga;
  wire [1-1:0]                ddr3_ck_p_fpga;
  wire [1-1:0]                ddr3_ck_n_fpga;
    
  
  wire                               init_calib_complete;
  wire                               tg_compare_error;
  wire [(CS_WIDTH*1)-1:0] ddr3_cs_n_fpga;
    
  wire [DM_WIDTH-1:0]                ddr3_dm_fpga;
    
  wire [ODT_WIDTH-1:0]               ddr3_odt_fpga;
    
  
  reg [(CS_WIDTH*1)-1:0] ddr3_cs_n_sdram_tmp;
    
  reg [DM_WIDTH-1:0]                 ddr3_dm_sdram_tmp;
    
  reg [ODT_WIDTH-1:0]                ddr3_odt_sdram_tmp;
    

  
  wire [DQ_WIDTH-1:0]                ddr3_dq_sdram;
  reg [ROW_WIDTH-1:0]                ddr3_addr_sdram [0:1];
  reg [3-1:0]               ddr3_ba_sdram [0:1];
  reg                                ddr3_ras_n_sdram;
  reg                                ddr3_cas_n_sdram;
  reg                                ddr3_we_n_sdram;
  wire [(CS_WIDTH*1)-1:0] ddr3_cs_n_sdram;
  wire [ODT_WIDTH-1:0]               ddr3_odt_sdram;
  reg [1-1:0]                ddr3_cke_sdram;
  wire [DM_WIDTH-1:0]                ddr3_dm_sdram;
  wire [DQS_WIDTH-1:0]               ddr3_dqs_p_sdram;
  wire [DQS_WIDTH-1:0]               ddr3_dqs_n_sdram;
  reg [1-1:0]                 ddr3_ck_p_sdram;
  reg [1-1:0]                 ddr3_ck_n_sdram;
  
    

//**************************************************************************//

  //**************************************************************************//
  // Reset Generation
  //**************************************************************************//
  initial begin
    sys_rst_n = 1'b0;
    #RESET_PERIOD
      sys_rst_n = 1'b1;
  end
   //
   reg chipReset = 0;
   
    always @(posedge `DVT_FLAG[`DVTF_TOGGLE_CHIP_RESET_BIT]) begin
      wait (fpga.topDesign.topMod.pbus_reset==0);
      @(negedge fpga.topDesign.topMod.pbus_clock);
      #2000;
      `logI("Asserting pbus_Reset");
      force fpga.topDesign.topMod.pbus_reset = 1;
      repeat (10) @(negedge fpga.topDesign.topMod.pbus_clock);
      #2000;
      release fpga.topDesign.topMod.pbus_reset;      
      `DVT_FLAG[`DVTF_TOGGLE_CHIP_RESET_BIT] = 0;
   end
   
   assign sys_rst = RST_ACT_LOW ? sys_rst_n : ~sys_rst_n;

  //**************************************************************************//
  // Clock Generation
  //**************************************************************************//

  initial
    sys_clk_i = 1'b0;
  always
    sys_clk_i = #(CLKIN_PERIOD/2.0) ~sys_clk_i;


  initial
    clk_ref_i = 1'b0;
  always
    clk_ref_i = #REFCLK_PERIOD ~clk_ref_i;




  always @( * ) begin
    ddr3_ck_p_sdram      <=  #(TPROP_PCB_CTRL) ddr3_ck_p_fpga;
    ddr3_ck_n_sdram      <=  #(TPROP_PCB_CTRL) ddr3_ck_n_fpga;
    ddr3_addr_sdram[0]   <=  #(TPROP_PCB_CTRL) ddr3_addr_fpga;
    ddr3_addr_sdram[1]   <=  #(TPROP_PCB_CTRL) (CA_MIRROR == "ON") ?
                                                 {ddr3_addr_fpga[ROW_WIDTH-1:9],
                                                  ddr3_addr_fpga[7], ddr3_addr_fpga[8],
                                                  ddr3_addr_fpga[5], ddr3_addr_fpga[6],
                                                  ddr3_addr_fpga[3], ddr3_addr_fpga[4],
                                                  ddr3_addr_fpga[2:0]} :
                                                 ddr3_addr_fpga;
    ddr3_ba_sdram[0]     <=  #(TPROP_PCB_CTRL) ddr3_ba_fpga;
    ddr3_ba_sdram[1]     <=  #(TPROP_PCB_CTRL) (CA_MIRROR == "ON") ?
                                                 {ddr3_ba_fpga[3-1:2],
                                                  ddr3_ba_fpga[0],
                                                  ddr3_ba_fpga[1]} :
                                                 ddr3_ba_fpga;
    ddr3_ras_n_sdram     <=  #(TPROP_PCB_CTRL) ddr3_ras_n_fpga;
    ddr3_cas_n_sdram     <=  #(TPROP_PCB_CTRL) ddr3_cas_n_fpga;
    ddr3_we_n_sdram      <=  #(TPROP_PCB_CTRL) ddr3_we_n_fpga;
    ddr3_cke_sdram       <=  #(TPROP_PCB_CTRL) ddr3_cke_fpga;
  end
    

  always @( * )
    ddr3_cs_n_sdram_tmp   <=  #(TPROP_PCB_CTRL) ddr3_cs_n_fpga;
  assign ddr3_cs_n_sdram =  ddr3_cs_n_sdram_tmp;
    

  always @( * )
    ddr3_dm_sdram_tmp <=  #(TPROP_PCB_DATA) ddr3_dm_fpga;//DM signal generation
  assign ddr3_dm_sdram = ddr3_dm_sdram_tmp;
    

  always @( * )
    ddr3_odt_sdram_tmp  <=  #(TPROP_PCB_CTRL) ddr3_odt_fpga;
  assign ddr3_odt_sdram =  ddr3_odt_sdram_tmp;
    

// Controlling the bi-directional BUS

  genvar dqwd;
  generate
    for (dqwd = 1;dqwd < DQ_WIDTH;dqwd = dqwd+1) begin : dq_delay
      WireDelay #
       (
        .Delay_g    (TPROP_PCB_DATA),
        .Delay_rd   (TPROP_PCB_DATA_RD),
        .ERR_INSERT ("OFF")
       )
      u_delay_dq
       (
        .A             (ddr3_dq_fpga[dqwd]),
        .B             (ddr3_dq_sdram[dqwd]),
        .reset         (sys_rst_n),
        .phy_init_done (init_calib_complete)
       );
    end
    // For ECC ON case error is inserted on LSB bit from DRAM to FPGA
          WireDelay #
       (
        .Delay_g    (TPROP_PCB_DATA),
        .Delay_rd   (TPROP_PCB_DATA_RD),
        .ERR_INSERT (ERR_INSERT)
       )
      u_delay_dq_0
       (
        .A             (ddr3_dq_fpga[0]),
        .B             (ddr3_dq_sdram[0]),
        .reset         (sys_rst_n),
        .phy_init_done (init_calib_complete)
       );
  endgenerate

  genvar dqswd;
  generate
    for (dqswd = 0;dqswd < DQS_WIDTH;dqswd = dqswd+1) begin : dqs_delay
      WireDelay #
       (
        .Delay_g    (TPROP_DQS),
        .Delay_rd   (TPROP_DQS_RD),
        .ERR_INSERT ("OFF")
       )
      u_delay_dqs_p
       (
        .A             (ddr3_dqs_p_fpga[dqswd]),
        .B             (ddr3_dqs_p_sdram[dqswd]),
        .reset         (sys_rst_n),
        .phy_init_done (init_calib_complete)
       );

      WireDelay #
       (
        .Delay_g    (TPROP_DQS),
        .Delay_rd   (TPROP_DQS_RD),
        .ERR_INSERT ("OFF")
       )
      u_delay_dqs_n
       (
        .A             (ddr3_dqs_n_fpga[dqswd]),
        .B             (ddr3_dqs_n_sdram[dqswd]),
        .reset         (sys_rst_n),
        .phy_init_done (init_calib_complete)
       );
    end
  endgenerate

   reg 	jtag_jtag_TCK =0; 
   reg 	jtag_jtag_TMS =0; 
   reg 	jtag_jtag_TDI =0;
   wire jtag_jtag_TDO; 
   
   wire uart_rxd;   pullup (weak1) (uart_rxd);
   wire uart_ctsn;  pullup (weak1) (uart_ctsn);
   wire uart_txd; 
   wire uart_rtsn; 

   wire sdio_sdio_clk; 
   wire sdio_sdio_cmd;    
   wire sdio_sdio_dat_0; pullup (weak1) (sdio_sdio_dat_0);
   wire sdio_sdio_dat_1; pullup (weak1) (sdio_sdio_dat_1);
   wire sdio_sdio_dat_2; pullup (weak1) (sdio_sdio_dat_2);   
   wire sdio_sdio_dat_3; pullup (weak1) (sdio_sdio_dat_3);   
   
   
   //
   // ############################################
   // C <-> Verilog Deamon and backdoor support are here
   // ############################################
   //
   //
   // Instantiate V<->C deamon    here
   //
   wire [31:0] __simTime;
   
   v2c_top v2cd(.clk(sys_clk_i),.__simTime(__simTime));

   //
   // sdome overrides
   //
   `ifdef BFM_MODE
   defparam `CORE0_TL_PATH.CHIP_ID=0;
   defparam `CORE1_TL_PATH.CHIP_ID=1;
   defparam `CORE2_TL_PATH.CHIP_ID=2;
   defparam `CORE3_TL_PATH.CHIP_ID=3;
   `endif
   
   integer 	j;
   
   initial begin
      for (j=0;j<32;j=j+1) begin
	 `FIR_PATH.datain_mem[j] = 0;	 
	 `FIR_PATH.dataout_mem[j] = 0;
      end
      //
      repeat (2) @(posedge sys_clk_i);            
      @(negedge sys_rst);
      // force FPGA to get out of reset faster
      repeat (100) @(posedge sys_clk_i);      
      force `FPGA_RESET_PATH.auto_out_1_reset = 0;
      force `FPGA_RESET_PATH.auto_out_0_reset = 0;
      //
      repeat (100) @(posedge sys_clk_i);
   end // initial begin

   //
   //===========================================================================   
   // Load the bare_boot.hex into boot rom
   //===========================================================================      
   //
   `ifdef BARE_MODE
   reg [256*8 - 1:0] path;
   initial begin
      repeat (100) @(posedge sys_clk_i);
      //path = "/home/aduong/CEP/CEP-master/cosim/bin/bareboot.hex";
      //path = "../../bin/bareboot.hex";
      path = "../../drivers/bootbare/bootbare.hex";      
      `logI("=== Overriding bootROm with file %s ==",path);      
      $readmemh(path, `BOOTROM_PATH.rom);
   end
   `endif
   
  //===========================================================================
  //  FPGA
  //===========================================================================
   VC707BaseShell fpga
     ( 
       .sys_clock_p 		(sys_clk_i),
       .sys_clock_n 		(!sys_clk_i),
       .jtag_jtag_TCK 		(jtag_jtag_TCK),
       .jtag_jtag_TMS 		(jtag_jtag_TMS),
       .jtag_jtag_TDI 		(jtag_jtag_TDI),
       .jtag_jtag_TDO 		(jtag_jtag_TDO),
       .uart_txd 		(uart_txd),
       .uart_rxd 		(uart_rxd),
       .uart_rtsn 		(uart_rtsn),
       .uart_ctsn 		(uart_ctsn),
       //
       .sdio_sdio_clk 		(sdio_sdio_clk),
       .sdio_sdio_cmd 		(sdio_sdio_cmd),
       .sdio_sdio_dat_0 	(sdio_sdio_dat_0),
       .sdio_sdio_dat_1 	(sdio_sdio_dat_1),
       .sdio_sdio_dat_2 	(sdio_sdio_dat_2),
       .sdio_sdio_dat_3 	(sdio_sdio_dat_3),
       // DDR3
       .ddr_ddr3_dq              (ddr3_dq_fpga),
       .ddr_ddr3_dqs_n           (ddr3_dqs_n_fpga),
       .ddr_ddr3_dqs_p           (ddr3_dqs_p_fpga),
       .ddr_ddr3_addr            (ddr3_addr_fpga),
       .ddr_ddr3_ba              (ddr3_ba_fpga),
       .ddr_ddr3_ras_n           (ddr3_ras_n_fpga),
       .ddr_ddr3_cas_n           (ddr3_cas_n_fpga),
       .ddr_ddr3_we_n            (ddr3_we_n_fpga),
       .ddr_ddr3_reset_n         (ddr3_reset_n),
       .ddr_ddr3_ck_p            (ddr3_ck_p_fpga),
       .ddr_ddr3_ck_n            (ddr3_ck_n_fpga),
       .ddr_ddr3_cke             (ddr3_cke_fpga),
       .ddr_ddr3_cs_n            (ddr3_cs_n_fpga),
       .ddr_ddr3_dm              (ddr3_dm_fpga),
       .ddr_ddr3_odt             (ddr3_odt_fpga),
       //
       .led 			(led),
       .reset 			(sys_rst || chipReset)
       );

   /*
    .sys_clk_i            (sys_clk_i),
    
    .init_calib_complete (init_calib_complete),
    .tg_compare_error    (tg_compare_error),
    .sys_rst             (sys_rst)
    */
   
   //**************************************************************************//
   // Memory Models instantiations
   //**************************************************************************//
   reg enableWrTrace = 0;
   reg enableRdTrace = 0;   
   reg calib_done = 0;
   always @(posedge `MIG_PATH.init_calib_complete) begin
      $display("%t Calibration Done",$time);
      calib_done = 1;
   end

   always @(posedge `DVT_FLAG[`DVTF_DISABLE_MAIN_MEM_LOGGING]) begin
      enableWrTrace = 0;
      enableRdTrace = 0;      
      `DVT_FLAG[`DVTF_DISABLE_MAIN_MEM_LOGGING] = 0;
   end
   always @(posedge `DVT_FLAG[`DVTF_ENABLE_MAIN_MEM_LOGGING]) begin
      enableWrTrace = 1;
      enableRdTrace = 1;            
      `DVT_FLAG[`DVTF_ENABLE_MAIN_MEM_LOGGING] = 0;
   end
   always @(posedge `DVT_FLAG[`DVTF_ENABLE_MAIN_MEMWR_LOGGING]) begin
      enableWrTrace = 1;
      `DVT_FLAG[`DVTF_ENABLE_MAIN_MEMWR_LOGGING] = 0;
   end
   always @(posedge `DVT_FLAG[`DVTF_ENABLE_MAIN_MEMRD_LOGGING]) begin
      enableRdTrace = 1;            
      `DVT_FLAG[`DVTF_ENABLE_MAIN_MEMRD_LOGGING] = 0;
   end   

   reg program_loaded = 0;
   always @(posedge `DVT_FLAG[`DVTF_PROGRAM_LOADED]) begin
      program_loaded = 1;
      //`DVT_FLAG[`DVTF_PROGRAM_LOADED] = 0;
   end

   //
   //
   `define den1024Mb
   wire [CS_WIDTH*NUM_COMP-1:0] ddr3MemWr;
   reg [CS_WIDTH*NUM_COMP-1:0] 	dq_in_valid_del=0;
   //
  genvar r,i;
  generate
    for (r = 0; r < CS_WIDTH; r = r + 1) begin: mem_rnk
      for (i = 0; i < NUM_COMP; i = i + 1) begin: gen_mem
	 defparam ddr3.DEBUG=0;
	 defparam ddr3.check_strict_mrbits=0;
	 defparam ddr3.check_strict_timing=0;
	 defparam ddr3.MEM_BITS=14;
	 defparam ddr3.TWLS = 2; // to remove the WARNING violation which can't be turned off from outside
	 defparam ddr3.TWLH = 2; // to remove the WARNING violation   

	 //
	 // Write Monitor
	 //
	 always @(negedge ddr3.ck) begin	 
	    dq_in_valid_del[(r*CS_WIDTH)+i] = ddr3.dq_in_valid; // delay version
	 end
	 assign ddr3MemWr[(r*CS_WIDTH)+i] = enableWrTrace && ddr3.ck && dq_in_valid_del[(r*CS_WIDTH)+i] && ((ddr3.burst_cntr % ddr3.BL_MIN) == 0);
	 always @(posedge ddr3MemWr[(r*CS_WIDTH)+i]) begin
	    if (ddr3MemWr[(r*CS_WIDTH)+i]) begin
	       `logI("MemWr: @0x%x (%x/%x/%x) =0x%x",{ddr3.memory_write.bank, ddr3.memory_write.row, ddr3.memory_write.col}*8,
		     ddr3.memory_write.bank,ddr3.memory_write.row,ddr3.memory_write.col,ddr3.memory_write.data);
	    end
	 end
	 //
        ddr3 ddr3
          (
           .rst_n   (ddr3_reset_n),
           .ck      (ddr3_ck_p_sdram[(i*MEMORY_WIDTH)/72]),
           .ck_n    (ddr3_ck_n_sdram[(i*MEMORY_WIDTH)/72]),
           .cke     (ddr3_cke_sdram[((i*MEMORY_WIDTH)/72)+(1*r)]),
           .cs_n    (ddr3_cs_n_sdram[((i*MEMORY_WIDTH)/72)+(1*r)]),
           .ras_n   (ddr3_ras_n_sdram),
           .cas_n   (ddr3_cas_n_sdram),
           .we_n    (ddr3_we_n_sdram),
           .dm_tdqs (ddr3_dm_sdram[i]),
           .ba      (ddr3_ba_sdram[r]),
           .addr    (ddr3_addr_sdram[r]),
           .dq      (ddr3_dq_sdram[MEMORY_WIDTH*(i+1)-1:MEMORY_WIDTH*(i)]),
           .dqs     (ddr3_dqs_p_sdram[i]),
           .dqs_n   (ddr3_dqs_n_sdram[i]),
           .tdqs_n  (),
//	   .enableRdTrace(enableRdTrace && calib_done),
//	   .enableWrTrace(enableWrTrace && calib_done),	   
           .odt     (ddr3_odt_sdram[((i*MEMORY_WIDTH)/72)+(1*r)])
           );
      end
    end
  endgenerate

   // backdoor load
   // for 1G
   localparam BA_BITS          =       3;
   localparam ROW_BITS         =      14;
   localparam COL_BITS         =      10;
   reg [63:0] cache [7:0] ; // 64 bytes
   
   task   write_ddr3_backdoor;
      input [31:0] a;
      input [63:0] d;
      reg [BA_BITS-1:0] bank;
      reg [ROW_BITS-1:0] row;
      reg [COL_BITS-1:0] col;
      reg [63:0] 	 mask;
      
      begin
	 if (calib_done == 0) @(posedge calib_done); // wait as soon as it is done
	 //
	 // put in the cache (little endian on 4-bytes (32-bits)
	 //
	 mask = 'hFF << (a[5:3]*8);
	 cache[7] = (cache[7] & ~mask) | (d[63:56] << (a[5:3]*8));
	 cache[6] = (cache[6] & ~mask) | (d[55:48] << (a[5:3]*8));
	 cache[5] = (cache[5] & ~mask) | (d[47:40] << (a[5:3]*8));
	 cache[4] = (cache[4] & ~mask) | (d[39:32] << (a[5:3]*8));
	 //
	 cache[3] = (cache[3] & ~mask) | (d[31:24] << (a[5:3]*8));
	 cache[2] = (cache[2] & ~mask) | (d[23:16] << (a[5:3]*8));
	 cache[1] = (cache[1] & ~mask) | (d[15:8]  << (a[5:3]*8));
	 cache[0] = (cache[0] & ~mask) | (d[7:0]   << (a[5:3]*8));
	 //
	 // flush when got the whole cahce	 
	 //
	 if (a[5:3] == 3'h7) begin // end of cache line
	    `logI("== Backdooring adr=0x%x data=0x%x",a,d);
	    {bank,row,col} = {a[29:6],3'b0}; // cache line
	    //
	    mem_rnk[0].gen_mem[0].ddr3.memory_write(bank,row,col,cache[0]);   
	    mem_rnk[0].gen_mem[1].ddr3.memory_write(bank,row,col,cache[1]);
	    mem_rnk[0].gen_mem[2].ddr3.memory_write(bank,row,col,cache[2]);
	    mem_rnk[0].gen_mem[3].ddr3.memory_write(bank,row,col,cache[3]);
	    mem_rnk[0].gen_mem[4].ddr3.memory_write(bank,row,col,cache[4]);
	    mem_rnk[0].gen_mem[5].ddr3.memory_write(bank,row,col,cache[5]);
	    mem_rnk[0].gen_mem[6].ddr3.memory_write(bank,row,col,cache[6]);
	    mem_rnk[0].gen_mem[7].ddr3.memory_write(bank,row,col,cache[7]);
	 end // if (a[5:3] == 3'h7)
      end
   endtask // WRITE32_64_TASK

   task   read_ddr3_backdoor;
      input [31:0] a;
      output [63:0] d;
      reg [BA_BITS-1:0] bank;
      reg [ROW_BITS-1:0] row;
      reg [COL_BITS-1:0] col;
      reg [63:0] 	 mask;      
      begin
	 //
	 {bank,row,col} = {a[29:6],3'b0}; // cache line
	 //
	 mem_rnk[0].gen_mem[0].ddr3.memory_read(bank,row,col,cache[0]);   
	 mem_rnk[0].gen_mem[1].ddr3.memory_read(bank,row,col,cache[1]);
	 mem_rnk[0].gen_mem[2].ddr3.memory_read(bank,row,col,cache[2]);
	 mem_rnk[0].gen_mem[3].ddr3.memory_read(bank,row,col,cache[3]);
	 mem_rnk[0].gen_mem[4].ddr3.memory_read(bank,row,col,cache[4]);
	 mem_rnk[0].gen_mem[5].ddr3.memory_read(bank,row,col,cache[5]);
	 mem_rnk[0].gen_mem[6].ddr3.memory_read(bank,row,col,cache[6]);
	 mem_rnk[0].gen_mem[7].ddr3.memory_read(bank,row,col,cache[7]);
	 // shift
	 d[63:56] = (cache[7] >> (a[5:3]*8));
	 d[55:48] = (cache[6] >> (a[5:3]*8));
	 d[47:40] = (cache[5] >> (a[5:3]*8));
	 d[39:32] = (cache[4] >> (a[5:3]*8));
	          
	 d[31:24] = (cache[3] >> (a[5:3]*8));
	 d[23:16] = (cache[2] >> (a[5:3]*8));
	 d[15:8]  = (cache[1] >> (a[5:3]*8));
	 d[7:0]   = (cache[0] >> (a[5:3]*8));
	 //
	 //`logI("== Backdooring adr=0x%x data=0x%x",a,d);
      end
   endtask // for

   task read_ddr3_cache_n_clear;
      input [31:0] a;
      output [(64*8)-1:0] pbuf;
      //
      reg [BA_BITS-1:0] bank;
      reg [ROW_BITS-1:0] row;
      reg [COL_BITS-1:0] col;
      reg [63:0] 	 d64;
      integer i;
      
      begin
	 //
	 {bank,row,col} = {a[29:6],3'b0}; // cache line
	 //
	 mem_rnk[0].gen_mem[0].ddr3.memory_read(bank,row,col,cache[0]);   
	 mem_rnk[0].gen_mem[1].ddr3.memory_read(bank,row,col,cache[1]);
	 mem_rnk[0].gen_mem[2].ddr3.memory_read(bank,row,col,cache[2]);
	 mem_rnk[0].gen_mem[3].ddr3.memory_read(bank,row,col,cache[3]);
	 mem_rnk[0].gen_mem[4].ddr3.memory_read(bank,row,col,cache[4]);
	 mem_rnk[0].gen_mem[5].ddr3.memory_read(bank,row,col,cache[5]);
	 mem_rnk[0].gen_mem[6].ddr3.memory_read(bank,row,col,cache[6]);
	 mem_rnk[0].gen_mem[7].ddr3.memory_read(bank,row,col,cache[7]);
	 //
	 mem_rnk[0].gen_mem[0].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[1].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[2].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[3].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[4].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[5].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[6].ddr3.memory_write(bank,row,col,0);
	 mem_rnk[0].gen_mem[7].ddr3.memory_write(bank,row,col,0);	 
	 // fill the buffer
	 for (i=0;i<8;i=i+1) begin
	    // shift
	    d64[63:56] = (cache[0] >> (i*8));
	    d64[55:48] = (cache[1] >> (i*8));
	    d64[47:40] = (cache[2] >> (i*8));
	    d64[39:32] = (cache[3] >> (i*8));
	    //
	    d64[31:24] = (cache[4] >> (i*8));
	    d64[23:16] = (cache[5] >> (i*8));
	    d64[15:8]  = (cache[6] >> (i*8));
	    d64[7:0]   = (cache[7] >> (i*8));
	    pbuf = (pbuf << 64) | d64;
	 end
      end
   endtask // for
   
  //***************************************************************************
  // Reporting the test case status
  // Status reporting logic exists both in simulation test bench (sim_tb_top)
  // and sim.do file for ModelSim. Any update in simulation run time or time out
  // in this file need to be updated in sim.do file as well.
  //***************************************************************************


   //
   // The driver
   //
   //
   reg [3:0] enableMask = 0;
   initial begin
      #1 enableMask = 'hF; // or contrtol from C side
   end
   genvar c;
   generate
      for (c=0;c<4;c=c+1) begin
	 cep_driver #(.MY_SLOT_ID(0),.MY_LOCAL_ID(c))
	 driver(
		.clk 		(sys_clk_i), // clk100),	 
		.reset 	        (sys_rst),
		.enableMe       (enableMask[c]),
		.__simTime	()
		);
      end
   endgenerate
   
   //
   //
   //
   initial begin
      // no secure mode
      @(posedge fpga.topDesign.topMod.rsa.blackbox.reset_n);
      @(posedge fpga.topDesign.topMod.rsa.blackbox.clk);
      fpga.topDesign.topMod.rsa.blackbox.exponation_mode_reg = 1;
   end
   //
   //
   //
   reg [`DVTF_FIR_CAPTURE_EN_BIT:`DVTF_AES_CAPTURE_EN_BIT] c2c_capture_enable=0;
   //
   `include "aes_capture.incl"
   `include "sha256_capture.incl"
   `include "md5_capture.incl"
   `include "rsa_capture.incl"
   `include "des3_capture.incl"
   `include "gps_capture.incl"
   `include "dft_capture.incl"
   `include "idft_capture.incl"
   `include "iir_capture.incl"
   `include "fir_capture.incl"                     
   //
endmodule
