//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      cep_tb.v
// Program:        Common Evaluation Platform (CEP)
// Description:    CEP Co-Simulation Top Level Testbench 
// Notes:          
//
//--------------------------------------------------------------------------------------

`ifndef COSIM_TB_TOP_MODULE
  `define COSIM_TB_TOP_MODULE       cep_tb
`endif

`ifndef CHIPYARD_TOP_MODULE
  `define CHIPYARD_TOP_MODULE       ChipTop
`endif 

`include "suite_config.v"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_cmds.incl"
`include "v2c_top.incl"

`ifndef CLOCK_PERIOD
  `define CLOCK_PERIOD          10
`endif
`ifndef RESET_DELAY
  `define RESET_DELAY           100
`endif

// JTAG related DPI imports
import "DPI-C" function int jtag_getSocketPortId();
import "DPI-C" function int jtag_cmd(input int tdo_in, output int encode);   
import "DPI-C" function int jtag_init();
import "DPI-C" function int jtag_quit();   

// Top Level Testbench Module
module `COSIM_TB_TOP_MODULE;

  //--------------------------------------------------------------------------------------
  // Wire & Reg Declarations
  //--------------------------------------------------------------------------------------
  reg                 sys_rst_n;
  wire                sys_rst;
  reg                 sys_clk;
  wire                sys_clk_pad;
    
  wire                jtag_TCK; pullup (weak1) (jtag_TCK);
  wire                jtag_TMS; pullup (weak1) (jtag_TMS);
  wire                jtag_TDI; pullup (weak1) (jtag_TDI);
  wire                jtag_TRSTn; pullup (weak1) (jtag_TRSTn);
  wire                jtag_TDO;   

  wire                uart_rxd; pullup (weak1) (uart_rxd);
  wire                uart_tb_rxd; pullup (weak1) (uart_tb_rxd);
  wire                uart_txd; 
  wire                uart_model_en;

  wire                gpio_0_0; pullup (weak1) (gpio_0_0);
  wire                gpio_0_1; pullup (weak1) (gpio_0_1);
  wire                gpio_0_2; pullup (weak1) (gpio_0_2);
  wire                gpio_0_3; pullup (weak1) (gpio_0_3);
  wire                gpio_0_4; pullup (weak1) (gpio_0_4);
  wire                gpio_0_5; pullup (weak1) (gpio_0_5);
  wire                gpio_0_6; pullup (weak1) (gpio_0_6);
  wire                gpio_0_7; pullup (weak1) (gpio_0_7);

  wire                spi_0_sck; 
  wire                spi_0_cs_0;    
  wire                spi_0_dq_0; pullup (weak1) (spi_0_dq_0);
  wire                spi_0_dq_1; pullup (weak1) (spi_0_dq_1);
  wire                spi_0_dq_2; pullup (weak1) (spi_0_dq_2);
  wire                spi_0_dq_3; pullup (weak1) (spi_0_dq_3);

  wire                spi_loopback_SCK;
  wire                spi_loopback_CS_n;
  wire                spi_loopback_MOSI;
  wire                spi_loopback_MISO;

  wire                spi_sd_model_rstn;
  wire                spi_sd_model_ncs;
  wire                spi_sd_model_sclk;
  wire                spi_sd_model_mosi;
  wire                spi_sd_model_miso;

  reg                 pll_bypass;
  wire                pll_bypass_pad;
  wire                pll_observe;
  //-------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Reset Generation
  //--------------------------------------------------------------------------------------
  initial begin
    sys_rst_n = 1'b0;

    #`RESET_DELAY

    sys_rst_n = 1'b1;
  end
  assign sys_rst = ~sys_rst_n;
  //--------------------------------------------------------------------------------------

  

  //--------------------------------------------------------------------------------------
  // Clock Generation
  //--------------------------------------------------------------------------------------
  initial
    sys_clk = 1'b0;
  always
    sys_clk = #(`CLOCK_PERIOD/2.0) ~sys_clk;
  assign sys_clk_pad = sys_clk;
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Either the UART is in loopback mode OR it is fed to a testbench UART receiver
  //--------------------------------------------------------------------------------------  
  assign uart_rxd       = (`UART_LOOPBACK_ENABLED) ? uart_txd : 1'b1;
  assign uart_model_en  = ~`UART_LOOPBACK_ENABLED;

  // Testbench UART receiver
  uart_model #(
  `ifdef ASIC_MODE
  `ifdef CADENCE
      .BIT_RATE(6_250_000),	  // CEP UART DIV = 32, internal clock = 200MHz
  `endif
  `ifdef MODELSIM
      .BIT_RATE(5_208_333), 	// CEP UART Div = 32, internal clock = 166.66MHz (should be 200MHz in Modelsim as well)
   `endif
`else
  .BIT_RATE(3_125_000),       // CEP UART DIV = 32, internal clock = 100MHz
`endif
    .CLK_HZ(100_000_000),
    .PAYLOAD_BITS(8),
    .STOP_BITS(2)
  ) uart_model_inst (
    .clk              (sys_clk),
    .resetn           (sys_rst_n),
    .uart_rxd         (uart_txd),
    .uart_rx_en       (~`UART_LOOPBACK_ENABLED)
  );  
  //--------------------------------------------------------------------------------------
  

  //--------------------------------------------------------------------------------------
  // The SPI either operates in loopback mode OR connects to the SPI SD Model
  //--------------------------------------------------------------------------------------
  
  assign spi_loopback_SCK   = spi_0_sck; 
  assign spi_loopback_CS_n  = `SPI_LOOPBACK_ENABLED ? spi_0_cs_0  : 1'b1;
  assign spi_loopback_MOSI  = spi_0_dq_0;
  assign spi_0_dq_1         = spi_loopback_MISO;
  assign spi_0_dq_1         = spi_sd_model_miso;

  assign spi_sd_model_rstn  = sys_rst_n;
  assign spi_sd_model_sclk  = spi_0_sck;
  assign spi_sd_model_ncs   = `SPI_LOOPBACK_ENABLED ? 1'b1        : spi_0_cs_0;
  assign spi_sd_model_mosi  = spi_0_dq_0;

  // SPI loopback instantiation
  spi_loopback spi_loopback_inst (
    .SCK    (spi_loopback_SCK   ),
    .CS_n   (spi_loopback_CS_n  ),
    .MOSI   (spi_loopback_MOSI  ),
    .MISO   (spi_loopback_MISO  )
  );
  
  // SPI / SD Model Instiation
  spi_sd_model spi_sd_model_inst (
    .rstn   (spi_sd_model_rstn  ),
    .ncs    (spi_sd_model_ncs   ),
    .sclk   (spi_sd_model_sclk  ),
    .mosi   (spi_sd_model_mosi  ),
    .miso   (spi_sd_model_miso  )
  ); 
  //--------------------------------------------------------------------------------------   


 
  //--------------------------------------------------------------------------------------
  // Device Under Test
  //
  // I/O manually copied from Chisel generated verilog
  //--------------------------------------------------------------------------------------

  // Force CHIP_ID's when operating in BFM_MODE (otherwise these parameters don't exist)
  `ifdef BFM_MODE
    defparam `TILE0_TL_PATH.CHIP_ID = 0;
    defparam `TILE1_TL_PATH.CHIP_ID = 1;
    defparam `TILE2_TL_PATH.CHIP_ID = 2;
    defparam `TILE3_TL_PATH.CHIP_ID = 3;
  `endif

  initial begin
  `ifdef ASIC_MODE
  `ifdef BYPASS_PLL
    `logI("PLL is set to bypass mode");
    pll_bypass = 1'b1;
  `else
    `logI("PLL is set to normal mode");
    pll_bypass = 1'b0;
  `endif
  `endif
  end
  assign  pll_bypass_pad  = pll_bypass;

  // Pinouts for the ASIC and FPGA differ
`ifdef ASIC_MODE
  `CHIPYARD_TOP_MODULE `DUT_INST ( 
    .jtag_TCK           (jtag_TCK),
    .jtag_TMS           (jtag_TMS),
    .jtag_TDI           (jtag_TDI),
    .jtag_TDO           (jtag_TDO),
    .spi_0_sck          (spi_0_sck),
    .spi_0_cs_0         (spi_0_cs_0),
    .spi_0_dq_0         (spi_0_dq_0),
    .spi_0_dq_1         (spi_0_dq_1),
    .spi_0_dq_2         (spi_0_dq_2),
    .spi_0_dq_3         (spi_0_dq_3),
    .test_mode_0        (),
    .test_mode_1        (),
    .test_mode_2        (),
    .test_mode_3        (),
    .test_io_0          (),
    .test_io_1          (),
    .test_io_2          (),
    .test_io_3          (),
    .test_io_4          (),
    .test_io_5          (),
    .test_io_6          (),
    .test_io_7          (),
    .test_io_8          (),
    .test_io_9          (),
    .test_io_10         (),
    .test_io_11         (),
    .test_io_12         (),
    .test_io_13         (),
    .test_io_14         (),
    .test_io_15         (),
    .socjtag_TCK        (),
    .socjtag_TMS        (),
    .socjtag_TDI        (),
    .socjtag_TRSTn      (),
    .socjtag_TDO        (),
    .gpio_0_0           (gpio_0_0),
    .gpio_0_1           (gpio_0_1),
    .gpio_0_2           (gpio_0_2),
    .gpio_0_3           (gpio_0_3),
    .gpio_0_4           (gpio_0_4),
    .gpio_0_5           (gpio_0_5),
    .gpio_0_6           (gpio_0_6),
    .gpio_0_7           (gpio_0_7),
    .uart_0_txd         (uart_txd),
    .uart_0_rxd         (uart_rxd),
    .reset              (sys_rst),
    .refclk             (sys_clk_pad),
    .pll_bypass         (pll_bypass_pad),
    .pll_observe        (pll_observe)
  );
  `else 
  `CHIPYARD_TOP_MODULE `DUT_INST ( 
    .jtag_TCK           (jtag_TCK),
    .jtag_TMS           (jtag_TMS),
    .jtag_TDI           (jtag_TDI),
    .jtag_TDO           (jtag_TDO),
    .spi_0_sck          (spi_0_sck),
    .spi_0_cs_0         (spi_0_cs_0),
    .spi_0_dq_0         (spi_0_dq_0),
    .spi_0_dq_1         (spi_0_dq_1),
    .spi_0_dq_2         (spi_0_dq_2),
    .spi_0_dq_3         (spi_0_dq_3),
    .gpio_0_0           (gpio_0_0),
    .gpio_0_1           (gpio_0_1),
    .gpio_0_2           (gpio_0_2),
    .gpio_0_3           (gpio_0_3),
    .gpio_0_4           (gpio_0_4),
    .gpio_0_5           (gpio_0_5),
    .gpio_0_6           (gpio_0_6),
    .gpio_0_7           (gpio_0_7),
    .uart_0_txd         (uart_txd),
    .uart_0_rxd         (uart_rxd),
    .reset              (sys_rst),
    .clock_clock        (sys_clk_pad)
  );
  `endif
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Instantiation of the System and CPU Drivers
  //--------------------------------------------------------------------------------------
  reg [3:0]   enableMask = 0;
  wire [3:0]  passMask;

  // Enable all the CPU Drivers
  initial begin
    #1 enableMask = 'hF;
  end
  
 genvar c;
  generate
    for (c = 0; c < `MAX_CORES; c++) begin : cpuId
      cpu_driver #(
        .MY_SLOT_ID   (0),
        .MY_CPU_ID    (c)
      ) driver (
        .clk          (sys_clk        ),
        .enableMe     (enableMask[c]  )
      );

      assign passMask[c] = driver.pcPass;
    end // end for
  endgenerate

  // Instantiate the "System" driver (which is ALWAYS enabled)
  system_driver #(
        .MY_SLOT_ID   (`SYSTEM_SLOT_ID),
        .MY_CPU_ID    (`SYSTEM_CPU_ID)
  ) system_driver (
    .clk        (sys_clk),
    .enableMe   (1'b1)
  );
  //--------------------------------------------------------------------------------------
  


  //--------------------------------------------------------------------------------------
  // OpenOCD interface to drive JTAG via DPI
  //--------------------------------------------------------------------------------------
  reg         enable_jtag     = 0;
  reg         quit_jtag       = 0;  
  reg         enableDel       = 0;
  reg [15:0]  clkCnt;
  int         junk;
  int         jtag_encode;
  wire        dpi_jtag_tdo    = jtag_TDO;
   
  always @(posedge `DVT_FLAG[`DVTF_ENABLE_REMOTE_BITBANG_BIT]) begin
    enable_jtag = 1;
    @(negedge `DVT_FLAG[`DVTF_ENABLE_REMOTE_BITBANG_BIT]);
    quit_jtag = 1;
  end

  `ifdef OPENOCD_ENABLE
    always @(posedge passMask[3]) begin
      repeat (40000) @(posedge sys_clk);
      `logI("Initialting QUIT to close socket...");
      junk = jtag_quit();
    end

    initial begin
      junk = jtag_init();
      jtag_TRSTn = 0;
      repeat (20) @(posedge sys_clk);
      jtag_TRSTn = 1;
      repeat (20) @(posedge sys_clk);
      jtag_TRSTn = 0;
    end

    always @(posedge sys_clk) begin
      if (sys_rst) begin
        enableDel   <= 0;
        clkCnt      <= 5;
      end else begin
        enableDel   <= enable_jtag;
        if (enableDel) begin
          clkCnt    <= clkCnt - 1;
        
          if (clkCnt == 0) begin
            clkCnt    <= 5;
        
            if (!quit_jtag) begin
              junk                                    = jtag_cmd(dpi_jtag_tdo, jtag_encode);
              {jtag_TRSTn,jtag_TCK,jtag_TMS,jtag_TDI} = jtag_encode ^ 'h8; // flip the TRSN
            end  // if (!quit_jtag)
          end // if (clkCnt == 0)
        end // if (enable && init_done_sticky)
      end // else: !if(reset || r_reset)
    end // always @ (posedge clock)
  `endif   
  //--------------------------------------------------------------------------------------
  


  //--------------------------------------------------------------------------------------
  // When operating in Bare Metal mode, instantiate the Rocket Tile Tilelink Monitors
  // They can be individually enabled from softwware and are disabled by default
  //--------------------------------------------------------------------------------------
  `ifdef BARE_MODE
    tile_monitor tile0_monitor_inst (
      .clock                                        (`TILE0_PATH.clock                                    ),
      .reset                                        (`TILE0_PATH.reset                                    ),
      .enable                                       (`CPU0_DRIVER.tl_monitor_enable                       ),
      .auto_wfi_out_0                               (`TILE0_PATH.auto_wfi_out_0                           ),
      .auto_int_local_in_3_0                        (`TILE0_PATH.auto_int_local_in_3_0                    ),
      .auto_int_local_in_2_0                        (`TILE0_PATH.auto_int_local_in_2_0                    ),
      .auto_int_local_in_1_0                        (`TILE0_PATH.auto_int_local_in_1_0                    ),
      .auto_int_local_in_1_1                        (`TILE0_PATH.auto_int_local_in_1_1                    ),
      .auto_int_local_in_0_0                        (`TILE0_PATH.auto_int_local_in_0_0                    ),
      .auto_hartid_in                               (`TILE0_PATH.auto_hartid_in                           ),
      .a_ready                                      (`TILE0_PATH.auto_tl_other_masters_out_a_ready        ),
      .a_valid                                      (`TILE0_PATH.auto_tl_other_masters_out_a_valid        ),
      .a_bits_opcode                                (`TILE0_PATH.auto_tl_other_masters_out_a_bits_opcode  ),
      .a_bits_param                                 (`TILE0_PATH.auto_tl_other_masters_out_a_bits_param   ),
      .a_bits_size                                  (`TILE0_PATH.auto_tl_other_masters_out_a_bits_size    ),
      .a_bits_source                                (`TILE0_PATH.auto_tl_other_masters_out_a_bits_source  ),
      .a_bits_address                               (`TILE0_PATH.auto_tl_other_masters_out_a_bits_address ),
      .a_bits_mask                                  (`TILE0_PATH.auto_tl_other_masters_out_a_bits_mask    ),
      .a_bits_data                                  (`TILE0_PATH.auto_tl_other_masters_out_a_bits_data    ),
      .b_ready                                      (`TILE0_PATH.auto_tl_other_masters_out_b_ready        ),
      .b_valid                                      (`TILE0_PATH.auto_tl_other_masters_out_b_valid        ),
      .b_bits_param                                 (`TILE0_PATH.auto_tl_other_masters_out_b_bits_param   ),
      .b_bits_size                                  (`TILE0_PATH.auto_tl_other_masters_out_b_bits_size    ),
      .b_bits_source                                (`TILE0_PATH.auto_tl_other_masters_out_b_bits_source  ),
      .b_bits_address                               (`TILE0_PATH.auto_tl_other_masters_out_b_bits_address ),
      .c_ready                                      (`TILE0_PATH.auto_tl_other_masters_out_c_ready        ),
      .c_valid                                      (`TILE0_PATH.auto_tl_other_masters_out_c_valid        ),
      .c_bits_opcode                                (`TILE0_PATH.auto_tl_other_masters_out_c_bits_opcode  ),
      .c_bits_param                                 (`TILE0_PATH.auto_tl_other_masters_out_c_bits_param   ),
      .c_bits_size                                  (`TILE0_PATH.auto_tl_other_masters_out_c_bits_size    ),
      .c_bits_source                                (`TILE0_PATH.auto_tl_other_masters_out_c_bits_source  ),
      .c_bits_address                               (`TILE0_PATH.auto_tl_other_masters_out_c_bits_address ),
      .c_bits_data                                  (`TILE0_PATH.auto_tl_other_masters_out_c_bits_data    ),
      .d_ready                                      (`TILE0_PATH.auto_tl_other_masters_out_d_ready        ),
      .d_valid                                      (`TILE0_PATH.auto_tl_other_masters_out_d_valid        ),
      .d_bits_opcode                                (`TILE0_PATH.auto_tl_other_masters_out_d_bits_opcode  ),
      .d_bits_param                                 (`TILE0_PATH.auto_tl_other_masters_out_d_bits_param   ),
      .d_bits_size                                  (`TILE0_PATH.auto_tl_other_masters_out_d_bits_size    ),
      .d_bits_source                                (`TILE0_PATH.auto_tl_other_masters_out_d_bits_source  ),
      .d_bits_sink                                  (`TILE0_PATH.auto_tl_other_masters_out_d_bits_sink    ),
      .d_bits_denied                                (`TILE0_PATH.auto_tl_other_masters_out_d_bits_denied  ),
      .d_bits_data                                  (`TILE0_PATH.auto_tl_other_masters_out_d_bits_data    ),
      .d_bits_corrupt                               (`TILE0_PATH.auto_tl_other_masters_out_d_bits_corrupt ),
      .e_ready                                      (`TILE0_PATH.auto_tl_other_masters_out_e_ready        ),
      .e_valid                                      (`TILE0_PATH.auto_tl_other_masters_out_e_valid        ),
      .e_bits_sink                                  (`TILE0_PATH.auto_tl_other_masters_out_e_bits_sink    )
    );

    tile_monitor tile1_monitor_inst (
      .clock                                        (`TILE1_PATH.clock                                    ),
      .reset                                        (`TILE1_PATH.reset                                    ),
      .enable                                       (`CPU1_DRIVER.tl_monitor_enable                       ),
      .auto_wfi_out_0                               (`TILE1_PATH.auto_wfi_out_0                           ),
      .auto_int_local_in_3_0                        (`TILE1_PATH.auto_int_local_in_3_0                    ),
      .auto_int_local_in_2_0                        (`TILE1_PATH.auto_int_local_in_2_0                    ),
      .auto_int_local_in_1_0                        (`TILE1_PATH.auto_int_local_in_1_0                    ),
      .auto_int_local_in_1_1                        (`TILE1_PATH.auto_int_local_in_1_1                    ),
      .auto_int_local_in_0_0                        (`TILE1_PATH.auto_int_local_in_0_0                    ),
      .auto_hartid_in                               (`TILE1_PATH.auto_hartid_in                           ),
      .a_ready                                      (`TILE1_PATH.auto_tl_other_masters_out_a_ready        ),
      .a_valid                                      (`TILE1_PATH.auto_tl_other_masters_out_a_valid        ),
      .a_bits_opcode                                (`TILE1_PATH.auto_tl_other_masters_out_a_bits_opcode  ),
      .a_bits_param                                 (`TILE1_PATH.auto_tl_other_masters_out_a_bits_param   ),
      .a_bits_size                                  (`TILE1_PATH.auto_tl_other_masters_out_a_bits_size    ),
      .a_bits_source                                (`TILE1_PATH.auto_tl_other_masters_out_a_bits_source  ),
      .a_bits_address                               (`TILE1_PATH.auto_tl_other_masters_out_a_bits_address ),
      .a_bits_mask                                  (`TILE1_PATH.auto_tl_other_masters_out_a_bits_mask    ),
      .a_bits_data                                  (`TILE1_PATH.auto_tl_other_masters_out_a_bits_data    ),
      .b_ready                                      (`TILE1_PATH.auto_tl_other_masters_out_b_ready        ),
      .b_valid                                      (`TILE1_PATH.auto_tl_other_masters_out_b_valid        ),
      .b_bits_param                                 (`TILE1_PATH.auto_tl_other_masters_out_b_bits_param   ),
      .b_bits_size                                  (`TILE1_PATH.auto_tl_other_masters_out_b_bits_size    ),
      .b_bits_source                                (`TILE1_PATH.auto_tl_other_masters_out_b_bits_source  ),
      .b_bits_address                               (`TILE1_PATH.auto_tl_other_masters_out_b_bits_address ),
      .c_ready                                      (`TILE1_PATH.auto_tl_other_masters_out_c_ready        ),
      .c_valid                                      (`TILE1_PATH.auto_tl_other_masters_out_c_valid        ),
      .c_bits_opcode                                (`TILE1_PATH.auto_tl_other_masters_out_c_bits_opcode  ),
      .c_bits_param                                 (`TILE1_PATH.auto_tl_other_masters_out_c_bits_param   ),
      .c_bits_size                                  (`TILE1_PATH.auto_tl_other_masters_out_c_bits_size    ),
      .c_bits_source                                (`TILE1_PATH.auto_tl_other_masters_out_c_bits_source  ),
      .c_bits_address                               (`TILE1_PATH.auto_tl_other_masters_out_c_bits_address ),
      .c_bits_data                                  (`TILE1_PATH.auto_tl_other_masters_out_c_bits_data    ),
      .d_ready                                      (`TILE1_PATH.auto_tl_other_masters_out_d_ready        ),
      .d_valid                                      (`TILE1_PATH.auto_tl_other_masters_out_d_valid        ),
      .d_bits_opcode                                (`TILE1_PATH.auto_tl_other_masters_out_d_bits_opcode  ),
      .d_bits_param                                 (`TILE1_PATH.auto_tl_other_masters_out_d_bits_param   ),
      .d_bits_size                                  (`TILE1_PATH.auto_tl_other_masters_out_d_bits_size    ),
      .d_bits_source                                (`TILE1_PATH.auto_tl_other_masters_out_d_bits_source  ),
      .d_bits_sink                                  (`TILE1_PATH.auto_tl_other_masters_out_d_bits_sink    ),
      .d_bits_denied                                (`TILE1_PATH.auto_tl_other_masters_out_d_bits_denied  ),
      .d_bits_data                                  (`TILE1_PATH.auto_tl_other_masters_out_d_bits_data    ),
      .d_bits_corrupt                               (`TILE1_PATH.auto_tl_other_masters_out_d_bits_corrupt ),
      .e_ready                                      (`TILE1_PATH.auto_tl_other_masters_out_e_ready        ),
      .e_valid                                      (`TILE1_PATH.auto_tl_other_masters_out_e_valid        ),
      .e_bits_sink                                  (`TILE1_PATH.auto_tl_other_masters_out_e_bits_sink    )
    );

    tile_monitor tile2_monitor_inst (
      .clock                                        (`TILE2_PATH.clock                                    ),
      .reset                                        (`TILE2_PATH.reset                                    ),
      .enable                                       (`CPU2_DRIVER.tl_monitor_enable                       ),
      .auto_wfi_out_0                               (`TILE2_PATH.auto_wfi_out_0                           ),
      .auto_int_local_in_3_0                        (`TILE2_PATH.auto_int_local_in_3_0                    ),
      .auto_int_local_in_2_0                        (`TILE2_PATH.auto_int_local_in_2_0                    ),
      .auto_int_local_in_1_0                        (`TILE2_PATH.auto_int_local_in_1_0                    ),
      .auto_int_local_in_1_1                        (`TILE2_PATH.auto_int_local_in_1_1                    ),
      .auto_int_local_in_0_0                        (`TILE2_PATH.auto_int_local_in_0_0                    ),
      .auto_hartid_in                               (`TILE2_PATH.auto_hartid_in                           ),
      .a_ready                                      (`TILE2_PATH.auto_tl_other_masters_out_a_ready        ),
      .a_valid                                      (`TILE2_PATH.auto_tl_other_masters_out_a_valid        ),
      .a_bits_opcode                                (`TILE2_PATH.auto_tl_other_masters_out_a_bits_opcode  ),
      .a_bits_param                                 (`TILE2_PATH.auto_tl_other_masters_out_a_bits_param   ),
      .a_bits_size                                  (`TILE2_PATH.auto_tl_other_masters_out_a_bits_size    ),
      .a_bits_source                                (`TILE2_PATH.auto_tl_other_masters_out_a_bits_source  ),
      .a_bits_address                               (`TILE2_PATH.auto_tl_other_masters_out_a_bits_address ),
      .a_bits_mask                                  (`TILE2_PATH.auto_tl_other_masters_out_a_bits_mask    ),
      .a_bits_data                                  (`TILE2_PATH.auto_tl_other_masters_out_a_bits_data    ),
      .b_ready                                      (`TILE2_PATH.auto_tl_other_masters_out_b_ready        ),
      .b_valid                                      (`TILE2_PATH.auto_tl_other_masters_out_b_valid        ),
      .b_bits_param                                 (`TILE2_PATH.auto_tl_other_masters_out_b_bits_param   ),
      .b_bits_size                                  (`TILE2_PATH.auto_tl_other_masters_out_b_bits_size    ),
      .b_bits_source                                (`TILE2_PATH.auto_tl_other_masters_out_b_bits_source  ),
      .b_bits_address                               (`TILE2_PATH.auto_tl_other_masters_out_b_bits_address ),
      .c_ready                                      (`TILE2_PATH.auto_tl_other_masters_out_c_ready        ),
      .c_valid                                      (`TILE2_PATH.auto_tl_other_masters_out_c_valid        ),
      .c_bits_opcode                                (`TILE2_PATH.auto_tl_other_masters_out_c_bits_opcode  ),
      .c_bits_param                                 (`TILE2_PATH.auto_tl_other_masters_out_c_bits_param   ),
      .c_bits_size                                  (`TILE2_PATH.auto_tl_other_masters_out_c_bits_size    ),
      .c_bits_source                                (`TILE2_PATH.auto_tl_other_masters_out_c_bits_source  ),
      .c_bits_address                               (`TILE2_PATH.auto_tl_other_masters_out_c_bits_address ),
      .c_bits_data                                  (`TILE2_PATH.auto_tl_other_masters_out_c_bits_data    ),
      .d_ready                                      (`TILE2_PATH.auto_tl_other_masters_out_d_ready        ),
      .d_valid                                      (`TILE2_PATH.auto_tl_other_masters_out_d_valid        ),
      .d_bits_opcode                                (`TILE2_PATH.auto_tl_other_masters_out_d_bits_opcode  ),
      .d_bits_param                                 (`TILE2_PATH.auto_tl_other_masters_out_d_bits_param   ),
      .d_bits_size                                  (`TILE2_PATH.auto_tl_other_masters_out_d_bits_size    ),
      .d_bits_source                                (`TILE2_PATH.auto_tl_other_masters_out_d_bits_source  ),
      .d_bits_sink                                  (`TILE2_PATH.auto_tl_other_masters_out_d_bits_sink    ),
      .d_bits_denied                                (`TILE2_PATH.auto_tl_other_masters_out_d_bits_denied  ),
      .d_bits_data                                  (`TILE2_PATH.auto_tl_other_masters_out_d_bits_data    ),
      .d_bits_corrupt                               (`TILE2_PATH.auto_tl_other_masters_out_d_bits_corrupt ),
      .e_ready                                      (`TILE2_PATH.auto_tl_other_masters_out_e_ready        ),
      .e_valid                                      (`TILE2_PATH.auto_tl_other_masters_out_e_valid        ),
      .e_bits_sink                                  (`TILE2_PATH.auto_tl_other_masters_out_e_bits_sink    )
    );

    tile_monitor tile3_monitor_inst (
      .clock                                        (`TILE3_PATH.clock                                    ),
      .reset                                        (`TILE3_PATH.reset                                    ),
      .enable                                       (`CPU3_DRIVER.tl_monitor_enable                       ),
      .auto_wfi_out_0                               (`TILE3_PATH.auto_wfi_out_0                           ),
      .auto_int_local_in_3_0                        (`TILE3_PATH.auto_int_local_in_3_0                    ),
      .auto_int_local_in_2_0                        (`TILE3_PATH.auto_int_local_in_2_0                    ),
      .auto_int_local_in_1_0                        (`TILE3_PATH.auto_int_local_in_1_0                    ),
      .auto_int_local_in_1_1                        (`TILE3_PATH.auto_int_local_in_1_1                    ),
      .auto_int_local_in_0_0                        (`TILE3_PATH.auto_int_local_in_0_0                    ),
      .auto_hartid_in                               (`TILE3_PATH.auto_hartid_in                           ),
      .a_ready                                      (`TILE3_PATH.auto_tl_other_masters_out_a_ready        ),
      .a_valid                                      (`TILE3_PATH.auto_tl_other_masters_out_a_valid        ),
      .a_bits_opcode                                (`TILE3_PATH.auto_tl_other_masters_out_a_bits_opcode  ),
      .a_bits_param                                 (`TILE3_PATH.auto_tl_other_masters_out_a_bits_param   ),
      .a_bits_size                                  (`TILE3_PATH.auto_tl_other_masters_out_a_bits_size    ),
      .a_bits_source                                (`TILE3_PATH.auto_tl_other_masters_out_a_bits_source  ),
      .a_bits_address                               (`TILE3_PATH.auto_tl_other_masters_out_a_bits_address ),
      .a_bits_mask                                  (`TILE3_PATH.auto_tl_other_masters_out_a_bits_mask    ),
      .a_bits_data                                  (`TILE3_PATH.auto_tl_other_masters_out_a_bits_data    ),
      .b_ready                                      (`TILE3_PATH.auto_tl_other_masters_out_b_ready        ),
      .b_valid                                      (`TILE3_PATH.auto_tl_other_masters_out_b_valid        ),
      .b_bits_param                                 (`TILE3_PATH.auto_tl_other_masters_out_b_bits_param   ),
      .b_bits_size                                  (`TILE3_PATH.auto_tl_other_masters_out_b_bits_size    ),
      .b_bits_source                                (`TILE3_PATH.auto_tl_other_masters_out_b_bits_source  ),
      .b_bits_address                               (`TILE3_PATH.auto_tl_other_masters_out_b_bits_address ),
      .c_ready                                      (`TILE3_PATH.auto_tl_other_masters_out_c_ready        ),
      .c_valid                                      (`TILE3_PATH.auto_tl_other_masters_out_c_valid        ),
      .c_bits_opcode                                (`TILE3_PATH.auto_tl_other_masters_out_c_bits_opcode  ),
      .c_bits_param                                 (`TILE3_PATH.auto_tl_other_masters_out_c_bits_param   ),
      .c_bits_size                                  (`TILE3_PATH.auto_tl_other_masters_out_c_bits_size    ),
      .c_bits_source                                (`TILE3_PATH.auto_tl_other_masters_out_c_bits_source  ),
      .c_bits_address                               (`TILE3_PATH.auto_tl_other_masters_out_c_bits_address ),
      .c_bits_data                                  (`TILE3_PATH.auto_tl_other_masters_out_c_bits_data    ),
      .d_ready                                      (`TILE3_PATH.auto_tl_other_masters_out_d_ready        ),
      .d_valid                                      (`TILE3_PATH.auto_tl_other_masters_out_d_valid        ),
      .d_bits_opcode                                (`TILE3_PATH.auto_tl_other_masters_out_d_bits_opcode  ),
      .d_bits_param                                 (`TILE3_PATH.auto_tl_other_masters_out_d_bits_param   ),
      .d_bits_size                                  (`TILE3_PATH.auto_tl_other_masters_out_d_bits_size    ),
      .d_bits_source                                (`TILE3_PATH.auto_tl_other_masters_out_d_bits_source  ),
      .d_bits_sink                                  (`TILE3_PATH.auto_tl_other_masters_out_d_bits_sink    ),
      .d_bits_denied                                (`TILE3_PATH.auto_tl_other_masters_out_d_bits_denied  ),
      .d_bits_data                                  (`TILE3_PATH.auto_tl_other_masters_out_d_bits_data    ),
      .d_bits_corrupt                               (`TILE3_PATH.auto_tl_other_masters_out_d_bits_corrupt ),
      .e_ready                                      (`TILE3_PATH.auto_tl_other_masters_out_e_ready        ),
      .e_valid                                      (`TILE3_PATH.auto_tl_other_masters_out_e_valid        ),
      .e_bits_sink                                  (`TILE3_PATH.auto_tl_other_masters_out_e_bits_sink    )
    );
    `endif // `ifdef BARE_MODE 
    //--------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------
    // When operating in Virtual mode, instantiate the page table walker monitor modules
    //--------------------------------------------------------------------------------------
    `ifdef VIRTUAL_MODE
    ptw_monitor ptwC0R0 (
      .clk                                  (`TILE0_PATH.ptw.clock                              ),
      .trace_valid                          (`TILE0_PATH.core.csr_io_trace_0_valid              ),
      .pc_valid                             (`TILE0_PATH.core.coreMonitorBundle_valid           ),
      .pc                                   (`TILE0_PATH.core.coreMonitorBundle_pc              ),
      .io_requestor_x_req_ready             (`TILE0_PATH.ptw.io_requestor_0_req_ready           ),
      .io_requestor_x_req_valid             (`TILE0_PATH.ptw.io_requestor_0_req_valid           ),
      .io_requestor_x_req_bits_bits_addr    (`TILE0_PATH.ptw.io_requestor_0_req_bits_bits_addr  ),
      .io_requestor_x_resp_valid            (`TILE0_PATH.ptw.io_requestor_0_resp_valid          ),
      .io_requestor_x_resp_bits_ae          (`TILE0_PATH.ptw.io_requestor_0_resp_bits_ae        ),
      .io_requestor_x_resp_bits_pte_ppn     (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_ppn   ),
      .io_requestor_x_resp_bits_pte_d       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_d     ),
      .io_requestor_x_resp_bits_pte_a       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_a     ),
      .io_requestor_x_resp_bits_pte_g       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_g     ),
      .io_requestor_x_resp_bits_pte_u       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_u     ),
      .io_requestor_x_resp_bits_pte_x       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_x     ),
      .io_requestor_x_resp_bits_pte_w       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_w     ),
      .io_requestor_x_resp_bits_pte_r       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_r     ),
      .io_requestor_x_resp_bits_pte_v       (`TILE0_PATH.ptw.io_requestor_0_resp_bits_pte_v     )
    );

    ptw_monitor ptwC0R1 (
      .clk                                  (`TILE0_PATH.ptw.clock                              ),
      .trace_valid                          (1'b0                                               ),
      .pc_valid                             (1'b0                                               ),
      .pc                                   (64'h0                                              ),
      .io_requestor_x_req_ready             (`TILE0_PATH.ptw.io_requestor_1_req_ready           ),
      .io_requestor_x_req_valid             (`TILE0_PATH.ptw.io_requestor_1_req_valid           ),
      .io_requestor_x_req_bits_bits_addr    (`TILE0_PATH.ptw.io_requestor_1_req_bits_bits_addr  ),
      .io_requestor_x_resp_valid            (`TILE0_PATH.ptw.io_requestor_1_resp_valid          ),
      .io_requestor_x_resp_bits_ae          (`TILE0_PATH.ptw.io_requestor_1_resp_bits_ae        ),
      .io_requestor_x_resp_bits_pte_ppn     (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_ppn   ),
      .io_requestor_x_resp_bits_pte_d       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_d     ),
      .io_requestor_x_resp_bits_pte_a       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_a     ),
      .io_requestor_x_resp_bits_pte_g       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_g     ),
      .io_requestor_x_resp_bits_pte_u       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_u     ),
      .io_requestor_x_resp_bits_pte_x       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_x     ),
      .io_requestor_x_resp_bits_pte_w       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_w     ),
      .io_requestor_x_resp_bits_pte_r       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_r     ),
      .io_requestor_x_resp_bits_pte_v       (`TILE0_PATH.ptw.io_requestor_1_resp_bits_pte_v     )
    );
    `endif
    //--------------------------------------------------------------------------------------   

endmodule  // `COSIM_TB_TOP_MODULE