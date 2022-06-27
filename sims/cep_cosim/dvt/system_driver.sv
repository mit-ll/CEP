//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      system_driver.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    System Level testbench driver
// Notes:          
//
//--------------------------------------------------------------------------------------

`include "suite_config.v"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_cmds.incl"
`include "v2c_top.incl"

module system_driver (
  input               clk,
  input               enableMe
);

  parameter MY_SLOT_ID                = 4'h0;
  parameter MY_CPU_ID                 = 4'h0;

  reg [255:0]         dvtFlags        = 0;
  reg [255:0]         r_data;
  reg [31:0]          printf_addr;
  reg [1:0]           printf_coreId;
  reg [(128*8)-1:0]   printf_buf;
  reg [(128*8)-1:0]   tmp;
  reg                 clear           = 0;
  integer             cnt;
  string              str;
  reg                 program_loaded  = 0;

  // The following bit, which can be controlled via a V2C command, determines
  // where a DUT_WRITE32_64/DUT_READ32_64 will be sent
  //
  // 0 - Main Memory
  // 1 - SD FLash Memory
  //
  reg                 backdoor_select = 0;
  //--------------------------------------------------------------------------------------
  // Define system driver supported DPI tasks prior to the inclusion of sys/driver_common.incl
  //--------------------------------------------------------------------------------------    
  // WRITE32_64
  `define SHIPC_WRITE32_64_TASK WRITE32_64_DPI()
  task WRITE32_64_DPI;
    reg [63:0] d;
    begin
      d[63:32] = inBox.mPar[0];
      d[31:0]  = inBox.mPar[1];

      if (backdoor_select)
        write_sdflash_backdoor(inBox.mAdr, d);
      else
        write_mainmem_backdoor(inBox.mAdr, d);
    end
  endtask // WRITE32_64_DPI

  // READ32_64
  `define SHIPC_READ32_64_TASK READ32_64_DPI()
  task READ32_64_DPI;
    reg [63:0] d;
    begin
      if (backdoor_select)
        read_sdflash_backdoor(inBox.mAdr, d);
      else
        read_mainmem_backdoor(inBox.mAdr, d);
      
      inBox.mPar[0] = d[63:32];
      inBox.mPar[1] = d[31:0];      
    end
  endtask // READ32_64_DPI
  
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
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // SHIPC Support Common Codes
  //
  // These includes must remain within the verilog module and
  // is dependent on the SHIPC_CLK macro.
  //--------------------------------------------------------------------------------------
  `define     SHIPC_XACTOR_ID     MY_CPU_ID
  `define     SHIPC_CLK           clk
  `include    "dpi_common.incl"
  `include    "dump_control.incl"      
  `undef      SHIPC_CLK
  `undef      SHIPC_XACTOR_ID      
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // The following functionality allows for selective control of the UART and SD during
  // the booting process.  By default (given that scratch_word0 resets to zero), UART
  // and SD Boot are enabled in the bootrom.  This needs to be the default behavior
  // for tapeout.
  //
  // When booting bare metal in the CEP CoSim, the default behavior is to bypass
  // SD boot and the UART.... and thus the scratch_word0 will be forced to the appropriate
  // value UNTIL core0 indicates a running status.
  //
  // If bits 1 and 0 are set, the UART Welcome message will be disabled
  // If bits 2 and 3 are set, SD Boot will be disabled
  //
  // This is only applicable in bare metal mode.  The RISC-V ISA tests don't use
  // the CEP Registers, so no changes should be needed there.
  //--------------------------------------------------------------------------------------
  `ifdef BARE_MODE

    initial begin
      force `CEPREGS_PATH.scratch_word0[3:0] = 4'hF;
    end

    // Automatically release the register when the core indicates it is running
    always @(`CEPREGS_PATH.core0_status) begin
      if (`CEPREGS_PATH.core0_status == `CEP_RUNNING_STATUS) 
        release `CEPREGS_PATH.scratch_word0[3:0];
    end

    // The UART has been enabled in the bootrom, release the appropriate
    // bits of the scratchpad register AND force the divider to a FAST speed 
    // for the remainder of the simulation
    always @(posedge `DVT_FLAG[`DVTF_BOOTROM_ENABLE_UART]) begin
      force `CEPREGS_PATH.scratch_word0[1:0] = 0;
      release `CEPREGS_PATH.scratch_word0[1:0];
      `logI("BOOTROM: Enabling the UART");
      `DVT_FLAG[`DVTF_BOOTROM_ENABLE_UART] = 0;

      // A divider of 16 does not seem to simulate properly in xcellium      
      force `DUT_UART_DIVIDER = 16'h0020;

    end // always @(posedge `DVT_FLAG[`DVTF_BOOTROM_ENABLE_UART])

    // The SPI interface has been enabled in the bootrom, release the
    // appropriate bits of the scratchpad register AND force the divder to
    // a FAST speed for the remainder of the simulation
    //
    // Additional, the payload size in bootrom will be forced to a
    // managable size.
    always @(posedge `DVT_FLAG[`DVTF_BOOTROM_ENABLE_SDBOOT]) begin
      force `CEPREGS_PATH.scratch_word0[3:2] = 0;
      release `CEPREGS_PATH.scratch_word0[3:2];
      `logI("BOOTROM: Enabling the SD Boot");
      `DVT_FLAG[`DVTF_BOOTROM_ENABLE_SDBOOT] = 0;

      // Force divider to a manageable size
      force `DUT_SPI_SCKDIV = 12'h010;

      // Force one of the scratch registers to a payload size that
      // will override the bootrom default (the fullboot test executable
      // is loaded is <16k bytes or 32 512-byte blocks)
      force `CEPREGS_PATH.scratch_word7 = 64'h0000_0000_0000_0020;
      
    end // always @(posedge `DVT_FLAG[`DVTF_BOOTROM_ENABLE_SDBOOT])
  
  `endif
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // DVT Flag Processing
  //--------------------------------------------------------------------------------------
  always @(posedge `DVT_FLAG[`DVTF_SET_BACKDOOR_SELECT]) begin
    backdoor_select = `DVT_FLAG[`DVTF_PAT_LO];
    `logI("Setting Backdoor Select to %d", backdoor_select);
    `DVT_FLAG[`DVTF_SET_BACKDOOR_SELECT] = 0;
  end // always @(posedge `DVT_FLAG[`DVTF_SET_BACKDOOR_SELECT])

  always @(posedge `DVT_FLAG[`DVTF_GET_BACKDOOR_SELECT]) begin
      dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO]   = backdoor_select;
      dvtFlags[`DVTF_GET_BACKDOOR_SELECT]  = 0; // self-clear
  end // always @(posedge `DVT_FLAG[`DVTF_GET_BACKDOOR_SELECT])

  always @(posedge `DVT_FLAG[`DVTF_SET_PROGRAM_LOADED]) begin
    `logI("Program is now loaded");
    program_loaded = `DVT_FLAG[`DVTF_PAT_LO];
    `DVT_FLAG[`DVTF_SET_PROGRAM_LOADED] = 0;
  end // always @(posedge `DVT_FLAG[`DVTF_SET_PROGRAM_LOADED])

  always @(posedge `DVT_FLAG[`DVTF_TOGGLE_CHIP_RESET_BIT]) 
  begin
    wait (`PBUS_RESET == 0);
    @(negedge `PBUS_CLOCK);
    #2000;
    `logI("Asserting pbus_Reset");
    force `PBUS_RESET = 1;
    repeat (10) @(negedge `PBUS_CLOCK);
    #2000;
    release `PBUS_RESET;      
    `DVT_FLAG[`DVTF_TOGGLE_CHIP_RESET_BIT] = 0;
  end // always @(posedge `DVT_FLAG[`DVTF_TOGGLE_CHIP_RESET_BIT]) 

  always @(posedge `DVT_FLAG[`DVTF_TOGGLE_DMI_RESET_BIT]) 
  begin
    `logI("Forcing topMod_debug_ndreset");
    force `DEBUG_NDRESET = 1;
    repeat (10) @(negedge `PBUS_CLOCK);
    release `DEBUG_NDRESET;
    `DVT_FLAG[`DVTF_TOGGLE_DMI_RESET_BIT] = 0;
  end // always @(posedge `DVT_FLAG[`DVTF_TOGGLE_DMI_RESET_BIT]) 

  always @(posedge `DVT_FLAG[`DVTF_GET_SOCKET_ID_BIT]) 
  begin
    `logI("DVTF_GET_SOCKET_ID_BIT");
    `ifdef OPENOCD_ENABLE
      `DVT_FLAG[`DVTF_PAT_HI:`DVTF_PAT_LO] = jtag_getSocketPortId();
    `endif
    `logI("SocketId = 0x%08x",`DVT_FLAG[`DVTF_PAT_HI:`DVTF_PAT_LO]);
    `DVT_FLAG[`DVTF_GET_SOCKET_ID_BIT] = 0;
  end // always @(posedge `DVT_FLAG[`DVTF_GET_SOCKET_ID_BIT])

  reg uart_loopback_enabled = 1;
  always @(posedge `DVT_FLAG[`DVTF_CONTROL_UART_LOOPBACK]) 
  begin
    uart_loopback_enabled = `DVT_FLAG[`DVTF_PAT_LO];
    `logI("DVTF_CONTROL_UART_LOOPBACK - %0d", uart_loopback_enabled);
    `DVT_FLAG[`DVTF_CONTROL_UART_LOOPBACK] = 0;
  end //posedge `DVT_FLAG[`DVTF_CONTROL_UART_LOOPBACK]) 

  reg spi_loopback_enabled = 1;
  always @(posedge `DVT_FLAG[`DVTF_CONTROL_SPI_LOOPBACK]) 
  begin
    spi_loopback_enabled = `DVT_FLAG[`DVTF_PAT_LO];
    `logI("DVTF_CONTROL_SPI_LOOPBACK - %0d", spi_loopback_enabled);
    `DVT_FLAG[`DVTF_CONTROL_SPI_LOOPBACK] = 0;
  end //posedge `DVT_FLAG[`DVTF_CONTROL_UART_LOOPBACK]) 
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Tasks support "backdoor" read/write access from/to Main Memory
  //
  // They should only be accessed from the system thread given that they assert
  // signals on the memory components vs internal methods (as was the case in the DDR
  // memory).  Otherwise, you could potentially get multiple threads driving the same
  // signals concurrently, which will have an unpredictable behavior.
  //--------------------------------------------------------------------------------------  
  // Writes data directly to the Scratchpad (Main) Memory
  task write_mainmem_backdoor;
    input [31:0] addr;
    input [63:0] data;

    begin
    
      // If the memory is in reset, wait for it to be released
      if (`SCRATCHPAD_WRAPPER_PATH.rst == 1) @(negedge `SCRATCHPAD_WRAPPER_PATH.rst);

      @(negedge `SCRATCHPAD_WRAPPER_PATH.clk);

      // All backdoor memory access is 64-bit
      force `SCRATCHPAD_WRAPPER_PATH.scratchpad_mask_i        = '1;
      force `SCRATCHPAD_WRAPPER_PATH.scratchpad_write_i       = 1;
      force `SCRATCHPAD_WRAPPER_PATH.scratchpad_addr_i        = addr >> 3;
      force `SCRATCHPAD_WRAPPER_PATH.scratchpad_wdata_i       = data;

      @(negedge `SCRATCHPAD_WRAPPER_PATH.clk);
     
      release `SCRATCHPAD_WRAPPER_PATH.scratchpad_mask_i;
      release `SCRATCHPAD_WRAPPER_PATH.scratchpad_write_i;
      release `SCRATCHPAD_WRAPPER_PATH.scratchpad_addr_i;
      release `SCRATCHPAD_WRAPPER_PATH.scratchpad_wdata_i;

    end
  endtask // write_mainmem_backdoor

  // Reads data directly from the Scratcpad (Main) Memory
  task read_mainmem_backdoor;
    input   [31:0] addr;
    output  [63:0] data;

    begin
    
      // If the memory is in reset, wait for it to be released
      if (`SCRATCHPAD_WRAPPER_PATH.rst == 1) @(negedge `SCRATCHPAD_WRAPPER_PATH.rst);

      // Reads are registered, need to be synchronized to the clock
      force `SCRATCHPAD_WRAPPER_PATH.scratchpad_addr_i    = addr >> 3;
      @(posedge `SCRATCHPAD_WRAPPER_PATH.clk);
      @(negedge `SCRATCHPAD_WRAPPER_PATH.clk);

      #1;

      data = `SCRATCHPAD_WRAPPER_PATH.scratchpad_rdata_o;
      release `SCRATCHPAD_WRAPPER_PATH.scratchpad_addr_i;

    end
  endtask // read_mainmem_backdoor
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Task to support "backdoor" read/write access from/to SD Flash Memory
  //
  // Note: SDFlash is 8-bits wide, so byte ordering will be important (Little Endian)
  //--------------------------------------------------------------------------------------
  task write_sdflash_backdoor;
    input [31:0] addr;
    input [63:0] data;

    begin

      `logI("Write 0x%x to address 0x%x", data, addr);

      // If the memory is in reset, wait for it to be released
      if (`SDCARD_PATH.rstn == 0) @(posedge `SDCARD_PATH.rstn);

      for (int i = 0; i < 8; i++) begin
        `SDCARD_PATH.write_flash_byte(addr + i, data[i*8 +: 8]);
      end

    end
  endtask // write_sdflash_backdopor


  task read_sdflash_backdoor;
    input   [31:0] addr;
    output  [63:0] data;

    begin

      // If the memory is in reset, wait for it to be released
      if (`SDCARD_PATH.rstn == 0) @(posedge `SDCARD_PATH.rstn);

      for (int i = 0; i < 8; i++) begin
        data[i*8 +: 8] = `SDCARD_PATH.flash_mem[addr + i];
      end

      // Advance a clock for good measure
      @(posedge `SDCARD_PATH.sclk);

    end
  endtask // write_sdflash_backdopor
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // System Driver support tasks when running the RISCV_TESTS
  //--------------------------------------------------------------------------------------
  // This is to handle single threading core: one core active at a time
  `ifdef RISCV_TESTS
    reg [63:0]  passFail [0:4]      = '{default:0};
    reg         passFailValid       = 0;
    int         file;
    initial begin
      `logI("==== ISA RISCV_TESTS is active ===");      

      // Perform a simple file I/O test to ensure file is there
      file = $fopen("PassFail.hex", "r");
      if (file) begin
        $fclose(file);
        passFailValid = 1;
        $readmemh("PassFail.hex", passFail);
        `logI("Reading from PassFail.hex: pass = 0x%0x, fail = 0x%0x, finish = 0x%0x, write_tohost = 0x%0x, hangme = 0x%0x",
          passFail[0], passFail[1], passFail[2], passFail[3], passFail[4]);
      end
    end
   
    // Force all cores into reset
    task ResetAllCores;
      begin
        force `TILE0_PATH.reset = 1;
        force `TILE1_PATH.reset = 1;
        force `TILE2_PATH.reset = 1;
        force `TILE3_PATH.reset = 1;
   
        repeat (2) @(posedge clk);
   
        release `TILE0_PATH.reset;  
        release `TILE1_PATH.reset;  
        release `TILE2_PATH.reset;  
        release `TILE3_PATH.reset;  
      end
    endtask // ResetAllCores

  `endif // endif `ifdef RISCV_TESTS
  //--------------------------------------------------------------------------------------
   
endmodule // v2c_top
