//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      cpu_driver.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    Provides the BFM_MODE connections between
//                 the Tilelink masters and DPI mailboxes
//
//                 Also provides some monitoring functions when
//                 the RISCV_TESTS are enabled (in BARE_MODE)
// Notes:          Backdoor memory access is no longer supported
//                 from the individual core drivers and is thus only
//                 accessible from the system thread.
//
//--------------------------------------------------------------------------------------
`include "suite_config.v"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_cmds.incl"
`include "v2c_top.incl"

module cpu_driver
(
  input               clk,
  input               enableMe
);

  // Overriden at instantiation
  parameter MY_SLOT_ID            = 4'h0;
  parameter MY_CPU_ID             = 4'h0;

  reg [255:0]         dvtFlags          = 0;
  reg [255:0]         r_data;
  reg [31:0]          printf_addr;
  reg [1:0]           printf_coreId;
  reg [(128*8)-1:0]   printf_buf;
  reg [(128*8)-1:0]   tmp;
  reg                 clear             = 0;
  integer             cnt;
  string              str;
  reg                 program_loaded    = 0;
  
  //--------------------------------------------------------------------------------------
  // Define system driver supported DPI tasks prior to the inclusion of sys/driver_common.incl
  //--------------------------------------------------------------------------------------    
  // READ_STATUS_TASK
  `define SHIPC_READ_STATUS_TASK READ_STATUS_TASK(__shIpc_p0)
  task READ_STATUS_TASK;
    output [31:0] r_data;
    begin
      inBox.mPar[0] = 0;
      @(posedge clk);
    end
  endtask // READ_STATUS_TASK;

  // WRITE_DVT_FLAG_TASK
  `define SHIPC_WRITE_DVT_FLAG_TASK WRITE_DVT_FLAG_TASK(__shIpc_p0,__shIpc_p1,__shIpc_p2)
  task WRITE_DVT_FLAG_TASK;
    input [31:0] msb;
    input [31:0] lsb;
    input [31:0] value; 
    begin
      for (int s = inBox.mPar[1]; s <= inBox.mPar[0]; s++) begin 
        dvtFlags[s]   = inBox.mPar[2] & 1'b1; 
        inBox.mPar[2] = inBox.mPar[2] >> 1; 
      end      
      
      @(posedge clk);  
    end
  endtask // WRITE_DVT_FLAG_TASK;

  // READ_DVT_FLAG_TASK
  `define SHIPC_READ_DVT_FLAG_TASK READ_DVT_FLAG_TASK(__shIpc_p0,__shIpc_p1,{__shIpc_p0[31:0],__shIpc_p1[31:0]})
  task READ_DVT_FLAG_TASK;
    input [31:0]    msb;
    input [31:0]    lsb;
    output [63:0]   r_data;
    integer         m;
    integer         l;
    reg [63:0]      tmp;
    begin
      tmp = 0;
    
      m = inBox.mPar[0];
      l = inBox.mPar[1];

      for (int s = m; s >= l; s--) begin       
        tmp = {tmp[62:0], dvtFlags[s]};
      end
      
      inBox.mPar[0] = tmp;
   
      @(posedge clk);   
    end
  endtask // READ_DVT_FLAG_TASK;

  // READ_ERROR_CNT_TASK
  `define SHIPC_READ_ERROR_CNT_TASK READ_ERROR_CNT_TASK(__shIpc_p0)
  task READ_ERROR_CNT_TASK;
    output [31:0]   r_data;
    begin
      $vpp_getErrorCount(inBox.mPar[0]);
    end
  endtask // READ_ERROR_CNT_TASK;

  // The following support task are only valid in BFM mode
  `ifdef BFM_MODE

    // WRITE64_BURST
    `define SHIPC_WRITE64_BURST_TASK WRITE64_BURST_DPI()
    task WRITE64_BURST_DPI;
      reg [3:0]   bits_size;
      begin
        bits_size = $clog2(inBox.mAdrHi << 3); // unit of 8 bytes
   
        case (MY_CPU_ID)
          0: begin
            for (int i=0;i<inBox.mAdrHi;i++) `TILE0_TL_PATH.tl_buf[i] = inBox.mPar[i];
            `TILE0_TL_PATH.tl_a_ul_write_burst(MY_CPU_ID & 'h1, inBox.mAdr,'hFF,bits_size);
          end
          1: begin
            for (int i=0;i<inBox.mAdrHi;i++) `TILE1_TL_PATH.tl_buf[i] = inBox.mPar[i];
            `TILE1_TL_PATH.tl_a_ul_write_burst(MY_CPU_ID & 'h1, inBox.mAdr,'hFF,bits_size);
          end
          2: begin
            for (int i=0;i<inBox.mAdrHi;i++) `TILE2_TL_PATH.tl_buf[i] = inBox.mPar[i];
            `TILE2_TL_PATH.tl_a_ul_write_burst(MY_CPU_ID & 'h1, inBox.mAdr,'hFF,bits_size);
          end
          3: begin
            for (int i=0;i<inBox.mAdrHi;i++) `TILE3_TL_PATH.tl_buf[i] = inBox.mPar[i];
            `TILE3_TL_PATH.tl_a_ul_write_burst(MY_CPU_ID & 'h1, inBox.mAdr,'hFF,bits_size);
          end     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE64_BURST_TASK

    // ATOMIC_RDW64
    `define SHIPC_ATOMIC_RDW64_TASK ATOMIC_RDW64_DPI()
    task ATOMIC_RDW64_DPI;
      reg [3:0]   bits_size;
      begin
        bits_size = 3;
   
        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_a_ul_logical_data(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mAdrHi,inBox.mPar[0],inBox.mPar[1],bits_size);
          1: `TILE1_TL_PATH.tl_a_ul_logical_data(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mAdrHi,inBox.mPar[0],inBox.mPar[1],bits_size);
          2: `TILE2_TL_PATH.tl_a_ul_logical_data(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mAdrHi,inBox.mPar[0],inBox.mPar[1],bits_size);
          3: `TILE3_TL_PATH.tl_a_ul_logical_data(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mAdrHi,inBox.mPar[0],inBox.mPar[1],bits_size);
        endcase
      end
    endtask // ATOMIC_RDW64_TASK

    // WRITE64_64
    `define SHIPC_WRITE64_64_TASK WRITE64_64_DPI()
    task WRITE64_64_DPI;
      begin
        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
          1: `TILE1_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
          2: `TILE2_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
          3: `TILE3_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,inBox.mPar[0]);     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE64_64_TASK

    // READ64_BURST
    `define SHIPC_READ64_BURST_TASK READ64_BURST_DPI()
    task READ64_BURST_DPI;
      reg [3:0] bits_size;
      begin
        bits_size = $clog2(inBox.mAdrHi << 3); // unit of 8 bytes
  
        case (MY_CPU_ID)
          0: begin
            `TILE0_TL_PATH.tl_a_ul_read_burst(MY_CPU_ID & 'h1, inBox.mAdr,bits_size);
            for (int i=0;i<inBox.mAdrHi;i++) inBox.mPar[i] = `TILE0_TL_PATH.tl_buf[i];
          end
          1: begin
            `TILE1_TL_PATH.tl_a_ul_read_burst(MY_CPU_ID & 'h1, inBox.mAdr,bits_size);
            for (int i=0;i<inBox.mAdrHi;i++) inBox.mPar[i] = `TILE1_TL_PATH.tl_buf[i];  
          end
          2: begin
            `TILE2_TL_PATH.tl_a_ul_read_burst(MY_CPU_ID & 'h1, inBox.mAdr,bits_size);
            for (int i=0;i<inBox.mAdrHi;i++) inBox.mPar[i] = `TILE2_TL_PATH.tl_buf[i];  
          end
          3: begin
            `TILE3_TL_PATH.tl_a_ul_read_burst(MY_CPU_ID & 'h1, inBox.mAdr,bits_size);
            for (int i=0;i<inBox.mAdrHi;i++) inBox.mPar[i] = `TILE3_TL_PATH.tl_buf[i];  
          end     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE64_BURST_TASK
   
    // READ64_64
    `define SHIPC_READ64_64_TASK READ64_64_DPI()
    task READ64_64_DPI;
      begin
        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
          1: `TILE1_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
          2: `TILE2_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
          3: `TILE3_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, inBox.mPar[0]);     
        endcase // case (MY_CPU_ID)
      end
    endtask // READ64_64_TASK
   
    // WRITE32_64
    `define SHIPC_WRITE32_64_TASK WRITE32_64_DPI()
    task WRITE32_64_DPI;
      reg [63:0] d;
      begin
        d[63:32] = inBox.mPar[0];
        d[31:0]  = inBox.mPar[1];

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,d);
          1: `TILE1_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,d);
          2: `TILE2_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,d);
          3: `TILE3_TL_PATH.tl_x_ul_write(MY_CPU_ID & 'h1, inBox.mAdr,d);     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE32_64_TASK

    // WRITE32_8_DPI
    `define SHIPC_WRITE32_8_TASK WRITE32_8_DPI()
    task WRITE32_8_DPI;
      reg [63:0] d;
      reg [7:0]  mask, byte8;

      begin
        mask  = 1 << inBox.mAdr[2:0];
        byte8 = inBox.mPar[0] & 'hff;
   
        d = {8{byte8}};

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,0);
          1: `TILE1_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,0);
          2: `TILE2_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,0);
          3: `TILE3_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,0);     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE32_8_DPI

    // WRITE32_16_DPI
    `define SHIPC_WRITE32_16_TASK WRITE32_16_DPI()
    task WRITE32_16_DPI;
      reg [63:0] d;
      reg [7:0]  mask;
      reg [15:0] word;
      begin
        mask = 3 << (inBox.mAdr[2:1]*2);
        word = inBox.mPar[0] & 'hffff;
          
        d = {4{word}};

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,1);
          1: `TILE1_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,1);
          2: `TILE2_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,1);
          3: `TILE3_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,1);     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE32_16_DPI

    // WRITE32_32_DPI
    `define SHIPC_WRITE32_32_TASK WRITE32_32_DPI()
    task WRITE32_32_DPI;
      reg [63:0] d;
      reg [7:0]  mask;
      begin
        if (inBox.mAdr[2]) 
          mask = 'hF0;
        else 
          mask = 'h0F;

        d[63:32] = inBox.mPar[0];
        d[31:0] = inBox.mPar[0];   
      
        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,2);
          1: `TILE1_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,2);
          2: `TILE2_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,2);
          3: `TILE3_TL_PATH.tl_a_ul_write_generic(MY_CPU_ID & 'h1, inBox.mAdr,d,mask,2);     
        endcase // case (MY_CPU_ID)
      end
    endtask // WRITE32_32_DPI
  
    // READ32_64
    `define SHIPC_READ32_64_TASK READ32_64_DPI()
    task READ32_64_DPI;
      reg [63:0] d;
      begin
        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, d);
          1: `TILE1_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, d);
          2: `TILE2_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, d);
          3: `TILE3_TL_PATH.tl_x_ul_read(MY_CPU_ID & 'h1, inBox.mAdr, d);     
        endcase // case (MY_CPU_ID)

        inBox.mPar[0] = d[63:32];
        inBox.mPar[1] = d[31:0];      
      end
    endtask // READ32_64_DPI

    // READ32_8_DPI
    `define SHIPC_READ32_8_TASK READ32_8_DPI()
    task READ32_8_DPI;
      reg [63:0] d;
      reg [7:0]  mask;
      begin
        mask = 1 << inBox.mAdr[2:0];

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 0, d);
          1: `TILE1_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 0, d);
          2: `TILE2_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 0, d);
          3: `TILE3_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 0, d);     
        endcase // case (MY_CPU_ID)
      
        case (inBox.mAdr[2:0])
          0 : inBox.mPar[0] = d[(8*0)+7:(8*0)];
          1 : inBox.mPar[0] = d[(8*1)+7:(8*1)];
          2 : inBox.mPar[0] = d[(8*2)+7:(8*2)];
          3 : inBox.mPar[0] = d[(8*3)+7:(8*3)];
          4 : inBox.mPar[0] = d[(8*4)+7:(8*4)];
          5 : inBox.mPar[0] = d[(8*5)+7:(8*5)];
          6 : inBox.mPar[0] = d[(8*6)+7:(8*6)];
          7 : inBox.mPar[0] = d[(8*7)+7:(8*7)];
        endcase
      end
    endtask // READ32_8_DPI

    // READ32_16_DPI
    `define SHIPC_READ32_16_TASK READ32_16_DPI()
    task READ32_16_DPI;
      reg [63:0] d;
      reg [7:0]  mask;
      begin
        mask = 3 << (inBox.mAdr[2:1]*2);      

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 1, d);
          1: `TILE1_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 1, d);
          2: `TILE2_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 1, d);
          3: `TILE3_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 1, d);     
        endcase // case (MY_CPU_ID)
      
        case (inBox.mAdr[2:1])
          0 : inBox.mPar[0] = d[(16*0)+15:(16*0)];
          1 : inBox.mPar[0] = d[(16*1)+15:(16*1)];
          2 : inBox.mPar[0] = d[(16*2)+15:(16*2)];
          3 : inBox.mPar[0] = d[(16*3)+15:(16*3)];
        endcase
      end
    endtask // READ32_16_DPI

    // READ32_32_DPI
    `define SHIPC_READ32_32_TASK READ32_32_DPI()
    task READ32_32_DPI;
      reg [63:0] d;
      reg [7:0]  mask;
      begin
        if 
          (inBox.mAdr[2]) mask = 'hF0;
        else 
          mask = 'h0F;      

        case (MY_CPU_ID)
          0: `TILE0_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 2, d);
          1: `TILE1_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 2, d);
          2: `TILE2_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 2, d);
          3: `TILE3_TL_PATH.tl_x_ul_read_generic(MY_CPU_ID & 'h1, inBox.mAdr, mask, 2, d);     
        endcase // case (MY_CPU_ID)
        
        inBox.mPar[0] = inBox.mAdr[2] ? d[63:32] : d[31:0];
      end
    endtask // READ32_32_DPI

  `endif // ifdef BFM_MODE
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // SHIPC Support Common Codes
  //--------------------------------------------------------------------------------------
  `define     SHIPC_XACTOR_ID     MY_CPU_ID
  `define     SHIPC_CLK           clk
  `include    "dpi_common.incl"
  `undef      SHIPC_CLK
  `undef      SHIPC_XACTOR_ID      
 //--------------------------------------------------------------------------------------
 


  //--------------------------------------------------------------------------------------
  // DVT Flag Processing
  //--------------------------------------------------------------------------------------
  // Core reset only makes sense in Bare Metal Mode
  `ifdef BARE_MODE
    reg tl_monitor_enable = 0;
    wire uart_busy;

    assign uart_busy      = `DUT_UART_BUSY;

    always @(posedge dvtFlags[`DVTF_UART_BUSY]) begin
      dvtFlags[`DVTF_PAT_LO]          = uart_busy;
      dvtFlags[`DVTF_UART_BUSY]       = 0; // self-clear
    end // end always

    always @(posedge dvtFlags[`DVTF_FORCE_CORE_RESET]) begin
      if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_CPU_ID) begin
        force_core_reset();
      end
      dvtFlags[`DVTF_FORCE_CORE_RESET] = 0;  
    end // end always

    always @(posedge dvtFlags[`DVTF_RELEASE_CORE_RESET]) begin
      if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_CPU_ID) begin
        release_core_reset();
      end
      dvtFlags[`DVTF_RELEASE_CORE_RESET] = 0;  
    end // end always

    // Enable the Tilelink monitor for the specified core
    always @(posedge dvtFlags[`DVTF_ENABLE_TL_MONITORS]) begin
      if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_CPU_ID) begin
        tl_monitor_enable = 1;
      end
      dvtFlags[`DVTF_ENABLE_TL_MONITORS] = 0;
    end // end always

    always @(posedge dvtFlags[`DVTF_GET_CORE_RESET_STATUS]) begin
      case (MY_CPU_ID)      
        0: dvtFlags[`DVTF_PAT_LO] = `CORE0_RESET;
        1: dvtFlags[`DVTF_PAT_LO] = `CORE1_RESET;
        2: dvtFlags[`DVTF_PAT_LO] = `CORE2_RESET;
        3: dvtFlags[`DVTF_PAT_LO] = `CORE3_RESET;
      endcase
      dvtFlags[`DVTF_GET_CORE_RESET_STATUS] = 0; // self-clear
    end // end always

  `endif

  always @(*) dvtFlags[`DVTF_GET_PROGRAM_LOADED]    = `PROGRAM_LOADED;

  always @(posedge dvtFlags[`DVTF_FORCE_TILE_RESET]) begin
    if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_CPU_ID) begin
      force_tile_reset();
    end
    dvtFlags[`DVTF_FORCE_TILE_RESET] = 0;  
  end // end always

  always @(posedge dvtFlags[`DVTF_RELEASE_TILE_RESET]) begin
    if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_CPU_ID) begin
      release_tile_reset();
    end
    dvtFlags[`DVTF_RELEASE_TILE_RESET] = 0;
  end // end always

  always @(posedge dvtFlags[`DVTF_GET_CORE_STATUS]) begin
    if      (dvtFlags[1:0] == 0) dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] = `CEPREGS_PATH.core0_status;
    else if (dvtFlags[1:0] == 1) dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] = `CEPREGS_PATH.core1_status;
    else if (dvtFlags[1:0] == 2) dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] = `CEPREGS_PATH.core2_status;
    else if (dvtFlags[1:0] == 3) dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] = `CEPREGS_PATH.core3_status;
    dvtFlags[`DVTF_GET_CORE_STATUS] = 0;
  end
  //--------------------------------------------------------------------------------------
   


  //--------------------------------------------------------------------------------------
  // Per core reset control
  //-------------------------------------------------------------------------------------- 

  // Put the core in reset that is not active
  initial begin

    // In bare metal mode, the cores will default to reset to allow
    // for backdoor loading of the executable to main memory.
    `ifdef BARE_MODE;
      force_tile_reset();
    `endif

    // In BFM mode,  we'll allow for a few cycles for the C side to come
    // up, otherwise we'll hold the TL master in reset
    `ifdef BFM_MODE
      repeat(10) @(posedge clk);
      if (!myIsActive) begin
        force_tile_reset();  
      end // if (!myIsActive)
    `endif
  end // initial begin

  // Task to force the current drivers tile to reset
  task force_tile_reset;
    begin
      case (MY_CPU_ID)
        0: begin
          `logI("Forcing Tile #0 in reset...");
          force `TILE0_RESET = 1;
        end
        1: begin
          `logI("Forcing Tile #1 in reset...");
          force `TILE1_RESET = 1;
        end
        2: begin
          `logI("Forcing Tile #2 in reset...");
          force `TILE2_RESET = 1;
        end
        3: begin
          `logI("Forcing Tile #3 in reset...");
          force `TILE3_RESET = 1;
        end     
      endcase // case (MY_CPU_ID)
    end
  endtask

  // Task to release the current drivers tile from reset
  task release_tile_reset;
    begin
      case (MY_CPU_ID)
        0: begin
          `logI("Releasing Tile #0 reset...");
          release `TILE0_RESET;
        end
        1: begin
          `logI("Releasing Tile #1 reset...");
          release `TILE1_RESET;
        end
        2: begin
          `logI("Releasing Tile #2 reset...");
          release `TILE2_RESET;
        end
        3: begin
          `logI("Releasing Tile #3 reset...");
          release `TILE3_RESET;
        end     
      endcase // case (MY_CPU_ID)
    end
  endtask

  // The following tasks allow for stimulation of the Core (within the Tile) reset
  `ifdef BARE_MODE

    // Task to force the current drivers core into reset
    task force_core_reset;
      begin
        case (MY_CPU_ID)
          0: begin
            `logI("Forcing Core #0 in reset...");
            force `CORE0_RESET = 1;
          end
          1: begin
            `logI("Forcing Core #1 in reset...");
            force `CORE1_RESET = 1;
          end
          2: begin
            `logI("Forcing Core #2 in reset...");
            force `CORE2_RESET = 1;
          end
          3: begin
            `logI("Forcing Core #3 in reset...");
            force `CORE3_RESET = 1;
          end     
        endcase // case (MY_CPU_ID)
      end
    endtask

    // Task to release the current drivers core from reset
    task release_core_reset;
      begin
        case (MY_CPU_ID)
          0: begin
            `logI("Releasing Core #0 reset...");
            release `CORE0_RESET;
          end
          1: begin
            `logI("Releasing Core #1 reset...");
            release `CORE1_RESET;
          end
          2: begin
            `logI("Releasing Core #2 reset...");
            release `CORE2_RESET;
          end
          3: begin
            `logI("Releasing Core #3 reset...");
            release `CORE3_RESET;
          end     
        endcase // case (MY_CPU_ID)
      end
    endtask
  
  `endif
  //-------------------------------------------------------------------------------------- 



  //--------------------------------------------------------------------------------------
  // Support functions for the RISC-V ISA Tests (which WILL require BARE_MODE)
  //--------------------------------------------------------------------------------------
  reg             pcPass              = 0;
  reg             pcFail              = 0;

  `ifdef RISCV_TESTS

    wire [63:0]   curPC;
    wire          curPCValid;
    wire          curPCReset;
    reg           DisableStuckChecker = 0;
    reg           SingleCoreOnly      = 0;
    int           stuckCnt            = 0;
    reg [63:0]    lastPc              = 0;
    wire          pcStuck             = (stuckCnt >= 500);

    // Get Pass / Fail Status
    always @(posedge dvtFlags[`DVTF_GET_PASS_FAIL_STATUS]) begin
      dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO]   = {pcFail, pcPass};
      dvtFlags[`DVTF_GET_PASS_FAIL_STATUS]  = 0; // self-clear
    end
    
    // Disable Stuck Checker
    always @(posedge dvtFlags[`DVTF_DISABLE_STUCKCHECKER]) begin
      `logI("DisableStuckChecker = 1");
      DisableStuckChecker                   = 1;
      dvtFlags[`DVTF_DISABLE_STUCKCHECKER]  = 0; // self-clear
    end   

    always @(posedge dvtFlags[`DVTF_SINGLE_CORE_ONLY]) begin
      `logI("SingleCoreOnly = 1");
      SingleCoreOnly                    = 1;
      DisableStuckChecker               = 1;
      dvtFlags[`DVTF_SINGLE_CORE_ONLY]  = 0; // self-clear
    end   

    // Generate per-CPU items
    generate
      case (MY_CPU_ID)
        0: begin
          assign curPC          = `CORE0_PC;
          assign curPCValid     = `CORE0_VALID;
          assign curPCReset     = `CORE0_RESET;
        end
        1: begin
          assign curPC          = `CORE1_PC;
          assign curPCValid     = `CORE1_VALID;
          assign curPCReset     = `CORE1_RESET;
        end
        2: begin
          assign curPC          = `CORE2_PC;
          assign curPCValid     = `CORE2_VALID;
          assign curPCReset     = `CORE2_RESET;
        end
        default: begin
          assign curPC          = `CORE3_PC;
          assign curPCValid     = `CORE3_VALID;
          assign curPCReset     = `CORE3_RESET;
        end
      endcase
    endgenerate

    // Take some action when a pass or failure is detected
    always @(posedge pcPass or posedge pcFail) begin
      if (~curPCReset) begin
        `logI("C%0d Pass/Fail Detected!!!... Put it to sleep", MY_CPU_ID);
        
        repeat (20) @(posedge clk);
        case (MY_CPU_ID)
          0       : force `CORE0_RESET = 1;
          1       : force `CORE1_RESET = 1;
          2       : force `CORE2_RESET = 1;
          default : force `CORE3_RESET = 1;
        endcase          
      end
    end // end always
   
    // Pass / Fail based on program counting reaching a particular location in the test 
    // Pass Condition - <test_pass> || <pass> || <finish> || <write_tohost>
    // Fail Condition - pcStuck || <test_fail> || <fail> || <hangme>
    always @(*) begin
      // A PC Stuck condition has been detected
      if (pcStuck && ~DisableStuckChecker) begin
        `logE("PC seems to be stuck!!!! Terminating...");
        pcFail = 1;
      end else if (curPCValid) begin
        // Did the PassFail.hex load correctly?
        if (`RISCV_PASSFAILVALID) begin
          case (curPC)
            `RISCV_PASSFAIL[0]  : pcPass = 1;
            `RISCV_PASSFAIL[2]  : pcPass = 1;
            `RISCV_PASSFAIL[3]  : pcPass = 1;
            `RISCV_PASSFAIL[4]  : pcFail = 1;
            `RISCV_PASSFAIL[1]  : pcFail = 1;
            default             : if (SingleCoreOnly && (MY_CPU_ID != 0)) pcPass = 1;
          endcase
        end else begin
          pcFail = 1;
        end // if (`RISCV_PASSFAILVALID)
      end // if (curPCValid)
    end   // end always @(*)

    // A running counter to indicate how many times the current PC has been "stuck" at the same value
    always @(posedge clk) begin
      if (curPCValid) begin
        lastPc <= curPC;
        if (curPC == lastPc) 
          stuckCnt <= stuckCnt + 1;
        else 
          stuckCnt <= 0;
      end
    end // end always
  
  `endif //  `ifdef RISCV_TESTS
  //--------------------------------------------------------------------------------------

endmodule // cpu_driver
