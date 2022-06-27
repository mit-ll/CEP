//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:     sd_spi_model.v
// Program:       Common Evaluation Platform (CEP)
// Description:   SD SPI Simulation Model originally released by tsuhuai.chan@gmail.com and
//                subsequented extracted from "Warke, Tejas Pravin, "Verification of 
//                SD/MMC Controller IP Using UVM" (2018). Thesis. Rochester Institute
//                of Technology"
//
// Notes:         Specification referenced is:
//                "SD Specifications Part 1 Physical Layer Simplified Specification 8.00, September 23, 2020"
//
//                - CARD_VHS changed to 4'b0001 (indicating support for 2.7-3.6V)
//                - CMD8 R7 response updated to be compliant with specification.  As we are operating in SPI
//                  mode, the R7 response is taken from Section 7.3.2.6 of the specification
//                - Changed CSD_VER to v2.0 (2'b01) given the bootrom assumes v2.0 or later
//                - Modified ist (initialization logic) to be compatible with Figure 7-2 in the specification
//                - Modified ACMD41 response per specification (in SPI mode it generates a R1 response, not R3)
//                - All commands that have not been EXPLICITLY verified have been removed
//                - Restored processing/setting of the block len (was commented out)
//                - OCR bit-width/mapping corrected per spec
//                - STM coding updated to properly sequence through the PowerOff -> PowerOn -> Idle states
//                - @(posedge sclk) removed from CardResponse state given that it caused a bit alignment
//                  issue when reading data
//--------------------------------------------------------------------------------------

// Version history :
// 1.0 2016.06.13 1st released by tsuhuai.chan@gmail.com 
//
// Most of the Card information is referenced from Toshiba 2G and 256MB SD card 
// 
// Memory size of this model should be 2GB, however only 2MB is implemented to reduce system memory required during simulation
// The initial value of all internal memory is word_address + 3. 
//

`include "suite_config.v"
`include "v2c_top.incl"
`include "cep_hierMap.incl"

`define UD 1 
module spi_sd_model ( rstn , ncs, sclk, miso, mosi);
input rstn; 
input ncs; 
input sclk; 
input mosi; 
output miso; 

parameter tNCS = 1;
parameter tNCR = 1;
parameter tNCX = 0;
parameter tNAC = 1;
parameter tNWR = 1;
parameter tNBR = 0;
parameter tNDS = 0;
parameter tNEC = 0;
parameter tNRC = 1;

parameter MEM_SIZE = 2048*1024; //2M 


// State Machine state definitions
parameter PowerOff      = 0; 
parameter PowerOn       = 1; 
parameter IDLE          = 2; 
parameter CmdBit47      = 3; 
parameter CmdBit46      = 4; 
parameter CommandIn     = 5; 
parameter CardResponse  = 6; 
parameter ReadCycle     = 7; 
parameter WriteCycle    = 8; 
parameter DataResponse  = 9; 
parameter CsdCidScr     = 10; 
parameter WriteStop     = 11; 
parameter WriteCRC      = 12; 

integer i = 0; // counter index 
integer j = 0; // counter index 
integer k = 0; // /for MISO (bit count of a byte) 
integer m = 0; // for MOSI ( bit count during CMD12) 

reg miso; 
reg [7:0] flash_mem [0:MEM_SIZE - 1]; 
reg [7:0] read_data;
reg [7:0] token; //captured token during CMD24, CMD25 
reg  [15:0] crc16_in;  
reg  [6:0]  crc7_in;
reg [15:0]  crc16_out;
reg  [7:0]  sck_cnt; // 74 sclk after power on
reg  [31:0] csd_reg = 0;  
reg  init_done; // must  be  defined  before  ocr .v  
reg  [3:0] st; //SD Card  internal  state  
reg  app_cmd; //  
reg  [511:0]  ascii_command_state;  
reg  [2:0]  ist ; // initialization stage  
reg  [45:0] cmd_in;  
reg  [45:0] serial_in;

wire [5:0] cmd_index = cmd_in[45:40]; 
wire [31:0] argument = cmd_in[39:8]; 
wire [6:0] crc = cmd_in[7:1]; 
wire read_single = (cmd_index == 17); 
wire read_multi = (cmd_index == 18); 
wire write_single = (cmd_index == 24); 
wire write_multi = (cmd_index == 25); 
wire pgm_csd = (cmd_index == 27); 
wire send_csd = (cmd_index == 9); 
wire send_cid = (cmd_index == 10); 
wire send_scr = (cmd_index == 51) && app_cmd; 
wire read_cmd = read_single | read_multi; 
wire write_cmd = write_single | write_multi; 

wire mem_rw = read_cmd | write_cmd; 
reg [31:0] start_addr; 
reg [31:0] block_len; 
reg [7:0] capture_data; // for debugging 
reg [3:0] VHS; // Input VHS through MOSI 
reg [7:0] check_pattern = 0; // for CMD8
wire [3:0] CARD_VHS     = 4'b0001; // SD card accept voltage range

wire VHS_match = (VHS == CARD_VHS);
reg [1:0] multi_st ; // for CMD25 
reg [45:0] serial_in1 ; // for CMD25
wire [5:0] cmd_in1 = serial_in1 [45:40]; // for CMD25
wire stop_transmission = (cmd_in1 == 12); //for CMD25 

//Do not change the positions of these include files 
// Also, ocr .v must be included before csd.v 
wire CCS        = 1'b0;
wire CARD_UHSII = 1'b0;
wire CARD_S18A  = 1'b0;
wire [31:0] OCR = {init_done , CCS, CARD_UHSII, 4'b0000, CARD_S18A, 6'b111111, 18'd0}; //3.0~3.6V, no S18A 
wire [1:0] DAT_BUS_WIDTH = 2'b00; //1bit 
wire SECURE_MODE = 1'b0; // not in secure mode 
wire [15:0] SD_CARD_TYPE = 16'h0000; // regular SD 
wire [31:0] SIZE_OF_PROTECTED_AREA = 32'd2048; // 
// protected area = SIZE_OF_PROTECTED_AREA * MULT * BLOCK_LEN 
wire [7:0] SPEED_CLASS = 8'h4; // class 10 
wire  [7:0] PERFORMANCE_MOVE  =  8'd100 ; // 100MB/ sec  
wire  [3:0]  AU_SIZE  =  7; // 1MB  
wire  [15:0]  ERASE_SIZE  =  16'd100 ; // Erase  100 AU  
wire  [5:0] ERASE_TIMEOUT  =  16'd50 ; // 50  sec  
wire  [1:0]  ERASE_OFFSET  =  0; // 0  sec  

wire [511:0] SSR = {DAT_BUS_WIDTH, SECURE_MODE, 6'b0, 6'b0, SD_CARD_TYPE, SIZE_OF_PROTECTED_AREA, SPEED_CLASS, 
                    PERFORMANCE_MOVE, AU_SIZE,  4'b0 , ERASE_SIZE,  ERASE_TIMEOUT, ERASE_OFFSET,  400'b0};

wire [7:0] MID = 8'd02; 
wire [15:0] OID = 16'h544D; 
wire [39:0] PNM = "SD02G"; 
wire [7:0] PRV = 8'h00; 
wire [31:0] PSN = 32'h6543a238; 
wire [11:0] MDT = {4'd15, 8'h12}; 
wire [6:0] CID_CRC = 7'b1100001; //dummy 
wire [127:0] CID = {MID, OID, PNM, PRV, PSN, 4'b0, MDT, CID_CRC ,1'b1}; 
wire [1:0] CSD_VER = 2'b01; // Ver 2.0 
wire [7:0] TAAC = {1'b0, 4'd7, 3'd2}; //3.0*100ns 
wire [7:0] NSAC = 8'd101; 
wire [7:0] TRAN_SPEED = 8'h32; 
wire [3:0] READ_BL_LEN = 4'd11; //2^READ_BL_LEN, 2048bytes 
wire READ_BL_PARTIAL = 1'b1; // always 1 in SD card 
wire WRITE_BLK_MISALIGN = 1'b0; // crossing physical blocak boundaries is invalid 
wire READ_BLK_MISALIGN = 1'b0; // crossing physical blocak boundaries is invalid 
wire DSR_IMP = 1'b0; //no DSR implemented 
wire [11:0] C_SIZE = 2047; 
wire [2:0] VDD_R_CURR_MIN = 3'd1; //1mA 
wire [2:0] VDD_R_CURR_MAX = 3'd2; //10mA 
wire [2:0] VDD_W_CURR_MIN = 3'd1; //1mA 
wire [2:0] VDD_W_CURR_MAX = 3'd2; //10mA 
wire [2:0] C_SIZE_MULT = 3'd7; //MULT=512 
wire ERASE_BLK_EN = 1'b0; // Erase in unit of SECTOR_SIZE 
wire [6:0] SECTOR_SIZE = 7'd127; //128 WRITE BLOCK 
wire [6:0] WP_GRP_SIZE = 7'd127; //128 
wire WP_GRP_ENABLE = 1'b0; //no GROUP WP 
wire [2:0] R2W_FACTOR = 3'd0; 
wire [3:0] WRITE_BL_LEN = READ_BL_LEN; 
wire WRITE_BL_PARTIAL = 1'b0; // 
wire iFILE_FORMAT_GRP = 1'b0; 
wire iCOPY = 1'b0; 
wire iPERM_WRITE_PROTECT = 1'b0; // DISABLE PERMENTAL WRITE PROTECT 
wire iTMP_WRITE_PROTECT = 1'b0; // 
wire [1:0] iFILE_FORMAT = 1'b0; //
wire [6:0] iCSD_CRC = 7'b1010001; // dummy 
reg FILE_FORMAT_GRP; 
reg COPY; 
reg PERM_WRITE_PROTECT; 
reg TMP_WRITE_PROTECT; 
reg [1:0] FILE_FORMAT; 
reg [6:0] CSD_CRC; 
wire v1sdsc = (CSD_VER == 0) & ~CCS; // Ver 1, SDSC 
wire v2sdsc = (CSD_VER == 1) & ~CCS; // Ver 2, SDSC 
wire v2sdhc = (CSD_VER == 1) & CCS; // Ver 2, SDHC 
wire sdsc = ~CCS; 
wire [127:0] CSD = {CSD_VER, 6'b0, TAAC, NSAC, TRAN_SPEED, 12'b0101_1011_0101 , READ_BL_LEN, 
                    READ_BL_PARTIAL, WRITE_BLK_MISALIGN, READ_BLK_MISALIGN, DSR_IMP,  
                    2'b0 , C_SIZE , VDD_R_CURR_MIN, VDD_R_CURR_MAX, VDD_W_CURR_MIN,  
                    VDD_W_CURR_MAX, C_SIZE_MULT, ERASE_BLK_EN, SECTOR_SIZE, WP_GRP_SIZE, 
                    WP_GRP_ENABLE, 2'b00, R2W_FACTOR, WRITE_BL_LEN, WRITE_BL_PARTIAL, 5'b0, 
                    FILE_FORMAT_GRP, COPY, PERM_WRITE_PROTECT, TMP_WRITE_PROTECT, FILE_FORMAT, 
                    2'b0, CSD_CRC, 1'b1} ;  
wire  OUT_OF_RANGE = 1'b0 ;  
wire  ADDRESS_ERROR = 1'b0 ;  
wire  BLOCK_LEN_ERROR = 1'b0 ;  
wire  ERASE_SEQ_ERROR = 1'b0 ;  
wire  ERASE_PARAM = 1'b0 ;  
wire  WP_VIOLATION = 1'b0 ;  
wire  CARD_IS_LOCKED = 1'b0 ;  
wire  LOCK_UNLOCK_FAILED = 1'b0 ;  
wire  COM_CRC_ERROR = 1'b0 ;  
wire  ILLEGAL_COMMAND = 1'b0 ;  

wire CARD_ECC_FAILED = 1'b0; 
wire CC_ERROR = 1'b0; 
wire ERROR = 1'b0; 
wire CSD_OVERWRITE = 1'b0; 
wire WP_ERASE_SKIP = 1'b0; 
wire CARD_ECC_DISABLE = 1'b0; 
wire ERASE_RESET = 1'b0; 
wire [3:0] CURRENT_ST = 1; // ready 
wire READY_FOR_DATA = 1'b1; 
wire APP_CMD = 1'b0; 
wire AKE_SEQ_ERROR = 1'b0; 
wire IN_IDLE_ST = (CURRENT_ST == 4'b1); 
wire [15:0] CSR = {OUT_OF_RANGE, ADDRESS_ERROR, BLOCK_LEN_ERROR , ERASE_SEQ_ERROR , ERASE_PARAM, WP_VIOLATION, CARD_IS_LOCKED, 
                   LOCK_UNLOCK_FAILED, COM_CRC_ERROR, ILLEGAL_COMMAND, CARD_ECC_FAILED, CC_ERROR, ERROR, 2'b0 , CSD_OVERWRITE, 
                    WP_ERASE_SKIP, CARD_ECC_DISABLE , ERASE_RESET, CURRENT_ST, READY_FOR_DATA, 2'b0 , APP_CMD, 1'b0, AKE_SEQ_ERROR, 3'b0}; 
wire [3:0] SCR_STRUCTURE = 4'd0; // Ver1.0 
wire  [3:0]  SD_SPEC = 4'd2; // Ver2.0 or 3.0  
wire  DATA_STAT_AFTER_ERASE = 1'b1 ;  
wire  [2:0]  SD_SECURITY = 3'd4 ; // Ver3.0  
wire  [3:0]  SD_BUS_WIDTHS = 4'b0001 ; // 1 bit  
wire  SD_SPEC3 = 1'b1 ; // Ver3.0  
wire  [13:0]  CMD_SUPPORT = 14'b0 ; //  

wire [63:0] SCR = {SCR_STRUCTURE, SD_SPEC, DATA_STAT_AFTER_ERASE, SD_SECURITY, SD_BUS_WIDTHS, SD_SPEC3 , 13'b0, CMD_SUPPORT, 32'b0}; 


task write_flash_byte (input[31:0] addr, input [7:0] data); begin
  `logI("SD_MODEL: Backdoor Write 0x%02x to address 0x%08x", data, addr);
  flash_mem[addr] = data;

  #1;
end
endtask

task R1; input [7:0] data ; begin 
  `logI("SD_MODEL: SD R1: 0x%02x at %0d ns ", data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 8; k = k + 1) begin 
    @(negedge sclk); miso = data[7 - k]; 
  end 
end 
endtask 

task R1b; input [7:0] data ; begin 
  `logI("SD_MODEL: SD R1B: 0x%02x at %0d ns", data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 8; k = k + 1) begin 
    @(negedge sclk); miso = data[7 - k]; 
  end 
end 
endtask 

task R2; input [15:0] data ; begin 
  `logI("SD_MODEL: SD R2: 0x%04x at %0d ns", data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 16; k = k + 1) begin 
    @(negedge sclk); miso = data[15 - k]; 
  end 
end 
endtask

task R3; input [39:0] data ; begin 
  `logI("SD_MODEL: SD R3: 0x%10x at %0d ns", data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 40; k = k + 1) begin 
    @(negedge sclk); miso = data[39 - k]; 
  end 
end 
endtask 

task R7; input [39:0] data ; begin 
  `logI("SD_MODEL: SD R7: 0x%10x at %0d ns", data,`SYSTEM_SIM_TIME); 
  for (k = 0; k < 40; k = k + 1) begin 
    @(negedge sclk); miso = data[39 - k]; 
  end 
end
endtask 

task DataOut; input [7:0] data ; begin 
  `logI("SD_MODEL:  SD DataOut 0x%02x at %0d ns", data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 8; k = k + 1) begin 
    @(negedge sclk); miso = data[7 - k]; 
  end 
end 
endtask 

task DataIn ; begin 
  for (k = 7; k >= 0; k = k - 1) begin 
    @(posedge sclk) capture_data[k] = mosi; 
  end 
  `logI("SD_MODEL: SD DataIn : %2h at %0d ns", capture_data, `SYSTEM_SIM_TIME ) ; 
end 
endtask 

always @(*) begin 
  if (pgm_csd) csd_reg = argument; 
end 

task CRCOut; input [15:0] data ; begin 
  `logI("SD_MODEL:  SD CRC Out 0x%04x at %0d ns" ,data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 16; k = k + 1) begin 
    @(negedge sclk); miso = data[15 - k]; 
  end 
end 
endtask 

task TokenOut; input [7:0] data ; begin 
  `logI("SD_MODEL:  SD TokenOut 0x%02x at %0d ns" ,data, `SYSTEM_SIM_TIME); 
  for (k = 0; k < 8; k = k + 1) begin 
    @(negedge sclk); miso = data[7 - k]; 
  end 
end 
endtask

function [15:0] crc16_round(input [15:0] crc, [7:0] data); begin
  crc = (crc >> 8) | (crc << 8);
  crc ^= data;
  crc ^= (crc >> 4) & 4'hF;
  crc ^= crc << 12;
  crc ^= (crc & 8'hFF) << 5;
  crc16_round = crc;
end
endfunction

always @(*) begin 
  if (~pgm_csd) begin 
    FILE_FORMAT_GRP = iFILE_FORMAT_GRP; 
    COPY = iCOPY; 
    PERM_WRITE_PROTECT = iPERM_WRITE_PROTECT; 
    TMP_WRITE_PROTECT = iTMP_WRITE_PROTECT; 
    FILE_FORMAT = iFILE_FORMAT; 
    CSD_CRC = iCSD_CRC ; 
  end else begin 
    FILE_FORMAT_GRP = argument [15]; 
    COPY = argument [14]; 
    PERM_WRITE_PROTECT = argument [13]; 
    TMP_WRITE_PROTECT = argument [12]; 
    FILE_FORMAT = argument [11:10]; 
    CSD_CRC = argument [7:1]; 
  end 
end 

always @( * )  begin  
  if (~ rstn ) app_cmd = 1'b0;  
  else if ( cmd_index == 55 && st == IDLE ) app_cmd = 1;  
  else if ( cmd_index != 55 && st == IDLE ) app_cmd = 0;  
end 

always @(*) begin 
  if (sdsc && mem_rw) 
    start_addr = argument; 
  else if (v2sdhc && mem_rw) 
    start_addr = argument * block_len; 
end 

// Set the Block Length
always @(*) begin 
  if (v2sdhc) 
    block_len = 512; 
  else if (sdsc && cmd_index == 0) block_len = (READ_BL_LEN == 9) ? 512 : (READ_BL_LEN == 10) ? 1024 : 2048; 
  else if (sdsc && cmd_index == 16) block_len = argument [31:0]; 
end 

always @(*) begin 
  if (cmd_index == 8) VHS = argument[11:8]; 
  if (cmd_index == 8) check_pattern = argument[7:0]; 
end 


// Logic for controlling the "initialization" of the SD Card model
always @(*) begin 
  if (ist == 0 && cmd_index == 0) begin 
    ist = 1;
    `logI("SD_MODEL: Moving to InitStage %d/3 at %0d ns" , ist, `SYSTEM_SIM_TIME); 
  end 

  if (ist == 1 && cmd_index == 8) begin 
    ist = 2;
    `logI("SD_MODEL: Moving to InitStage %d/3 at %0d ns" , ist, `SYSTEM_SIM_TIME); 
  end 

  if (ist == 2 && cmd_index == 41) begin 
    ist = 3;
    `logI("SD_MODEL: Moving to InitStage %d/3 at %0d ns" , ist, `SYSTEM_SIM_TIME); 
  end 

  if ( ist == 3 && st == IDLE) begin 
    ist = 4; 
    `logI("SD_MODEL: Init Done at %0d ns" ,`SYSTEM_SIM_TIME); 

    if (v2sdhc) `logI("SD_MODEL: Init Done at %0d ns, Ver 2, SDHC detected" ,`SYSTEM_SIM_TIME);
    else if (v2sdsc) `logI("SD_MODEL: Init Done at %0d ns, Ver 2, SDSC detected" ,`SYSTEM_SIM_TIME);
    else if (v1sdsc) `logI("SD_MODEL: Init Done at %0d ns, Ver 1, SDSC detected" ,`SYSTEM_SIM_TIME);

    init_done = 1; 
  end 
end 

always @(*) begin 
  if (st == ReadCycle) begin 
    // `logI (" readcycle ") ; 
    case ( multi_st ) 
      0: begin  
        @( posedge sclk ) if (~ncs && ~mosi) multi_st  =  1; else multi_st = 0;  
      end  
      1: begin  
        @( posedge sclk ) ; if (mosi) multi_st  =  2; else  multi_st = 1;  
      end  
      2: begin  
        m = 0;  
        while (m < 46) begin  
          @(posedge sclk ) serial_in1[45 - m] = mosi ;  
          #1 m = m + 1;  
        end  
        multi_st = 0;  
      end  
    endcase  
  end  
end  

always @(*) begin 
  case (st) 
    PowerOff : begin 
      @( posedge rstn) st <= PowerOn; 
    end 

    PowerOn : begin 
      for (i = 0; i < 75; i++) begin
        @ ( posedge sclk);
        sck_cnt = sck_cnt + 1;
      end

      st <= IDLE; 
    end 

    IDLE : begin 
      @( posedge sclk) if (~ ncs && ~mosi) st <= CmdBit46; else st <= IDLE; 
    end 

    CmdBit46 : begin 
      @( posedge sclk); if (mosi) st <= CommandIn; else st <= CmdBit46;       
    end 

    CommandIn : begin // capture command input -> NCR 
      for (i = 0; i < 46; i = i + 1) begin 
        @(posedge sclk);
        serial_in[45- i] = mosi; 
      end 
      cmd_in = serial_in; 

      repeat (tNCR * 8) @( posedge sclk ); 
      
      st <= CardResponse; 
    end 

    CardResponse : begin // CardResponse -> delay 
      `logI("SD_MODEL: Card Response app_cmd/read_multi/cmd_index/read_cmd = %0d/%0d/%0d/%0d", app_cmd, read_multi, cmd_index, read_cmd);
      // Not an application specific command
      if (~app_cmd) begin 
        case (cmd_index) 
          6'd0    : R1({7'b000_0000, init_done ? 1'b0 : 1'b1});
          6'd16,
          6'd18,
          6'd55   : R1({7'b000_0000, init_done ? 1'b0 : 1'b1});
          6'd8    : begin
                      if (VHS_match)
                        `logI ("SD_MODEL: VHS match");
                      else
                        `logI ("SD_MODEL: VHS not match");
                      // Per spec Section 7.3.2.1: bit 40 of the R7 response (which is bit 0 of the embedded
                      // R1 response is "in idle state: the card is in the idle state and running the initializing process)
                      R7({init_done ? 8'h00 : 8'h01, 20'h00000, VHS_match ? VHS : 4'b0, check_pattern});
                    end
          6'd58   : R3({8'b0000_0000, OCR}); 
          default : R1(8'b0000_0100); //illegal command 
        endcase 
      // Application specific commands
      end else if (~read_multi) begin
        case (cmd_index)
          6'd41   : R1({7'b000_0000, init_done ? 1'b0 : 1'b1});
          default : R1(8'b0000_0100); //illegal command 
        endcase 
      end // if (~read_multi)

      // Deassert miso after any response is sent, but it should only transition on the negedge of sclk
      @(negedge sclk);
      miso = 1;

      if (read_cmd && init_done /*&& ~stop_transmission*/) begin 
        repeat (tNAC * 8) @(posedge sclk); 
        st <= ReadCycle; 
      end else if (read_cmd && init_done && stop_transmission) begin 
        repeat (tNEC * 8) @(posedge sclk ); 
        st <= IDLE; 
      end else if ((send_csd || send_cid || send_scr) && init_done) begin 
        repeat (tNCX * 8) @(posedge sclk ); st <= CsdCidScr;
      end else if (write_cmd && init_done) begin 
        repeat (tNWR*8) @( posedge sclk ); 
        st <= WriteCycle ; 
      end else begin 
        repeat (tNEC*8) @(posedge sclk ); 
        st <= IDLE; 
      end 
    end // CardResponse
    CsdCidScr : begin
      if (send_csd) begin 
        DataOut(CSD[127:120]); 
        DataOut(CSD[119:112]); 
        DataOut(CSD[111:104]); 
        DataOut(CSD[103:96]); 
        DataOut(CSD[95:88]); 
        DataOut(CSD[87:80]); 
        DataOut(CSD[79:72]); 
        DataOut(CSD[71:64]); 
        DataOut(CSD[63:56]); 
        DataOut(CSD[55:48]); 
        DataOut(CSD[47:40]); 
        DataOut(CSD[39:32]); 
        DataOut(CSD[31:24]); 
        DataOut(CSD[23:16]); 
        DataOut(CSD[15:8]); 
        DataOut(CSD[7:0]); 
      end else if (send_cid) begin 
        DataOut(CID[127:120]); 
        DataOut(CID[119:112]); 
        DataOut(CID[111:104]); 
        DataOut(CID[103:96]); 
        DataOut(CID[95:88]); 
        DataOut(CID[87:80]); 
        DataOut(CID[79:72]); 
        DataOut(CID[71:64]); 
        DataOut(CID[63:56]); 
        DataOut(CID[55:48]); 
        DataOut(CID[47:40]); 
        DataOut(CID[39:32]); 
        DataOut(CID[31:24]); 
        DataOut(CID[23:16]); 
        DataOut(CID[15:8]); 
        DataOut(CID[7:0]); 
      end else if (send_scr) begin 
        DataOut(SCR[63:56]); 
        DataOut(SCR[55:48]); 
        DataOut(SCR[47:40]); 
        DataOut(SCR[39:32]); 
        DataOut(SCR[31:24]); 
        DataOut(SCR[23:16]); 
        DataOut(SCR[15:8]); 
        DataOut(SCR[7:0]); 
      end 

      @(posedge sclk); 
      repeat (tNEC*8) @(posedge sclk ) ; 
      st <= IDLE; 
    end 

    ReadCycle: begin //Start Token -> Data -> CRC(stucked at 16'hAAAA) -> NEC( or NAC) 

      // Perform a single block read
      if ( read_single ) begin 
      
        // Read Token
        TokenOut(8'hFE);

        // Reset the CRC
        crc16_out = 0;

        // Read from main memory
        for (i = 0; i < block_len; i = i + 1) begin
          read_data = flash_mem[start_addr + i];
          DataOut(read_data);
          crc16_out = crc16_round(crc16_out, read_data);
        end 

        // Send CRC
        CRCOut(crc16_out);

        @(posedge sclk); 

        // Wait some cycles, return to IDLE
        repeat (tNEC*8) @(negedge sclk); 
        st <= IDLE;

      // Perform a multiple block read
      end else if (read_multi) begin

        // Initialize block index
        j = 0;

        do begin

          // Start Token
          TokenOut(8'hFE);

          // Reset the CRC
          crc16_out = 0;

          // Read from main memory
          for (i = 0; i < block_len; i = i + 1) begin
            read_data = flash_mem[start_addr + block_len * j + i];
            DataOut(read_data);
            crc16_out = crc16_round(crc16_out, read_data);
          end 

          // Send CRC
          CRCOut(crc16_out);

          `logI("SD_MODEL: Multi-block read, block count = %d", j + 1);

          // Check stop tranmission after every block
          if (stop_transmission) begin
            
            repeat (tNEC*8) @( posedge sclk ) ; 
            `logI("SD_MODEL: STOP transmission"); 
            
            @(posedge sclk);

            // Tranmit stop acknowledgemtn           
            R1(8'b0000_0000) ; 

            // Break from the do loop
          end else 
            repeat (tNAC*8) @( negedge sclk ) ; 

          // increment the block index
          j++;

        end while (1); // end do

        // Wait some cycles, return to IDLE
        repeat (tNEC*8) @( posedge sclk ) ; 
        st <= IDLE; 

      end // if (read_multi)
    end // ReadCycle 

    WriteCycle: begin // Start Token -> Data 
      i = 0; 
      while (i < 8) begin 
        @(posedge sclk) token[7- i] = mosi; 
        i = i + 1; 
      end 

      if (token == 8'hfe && write_single) `logI("SD_MODEL: Single Write Start Token OK"); 
      else if (token != 8'hfe && write_single) `logI("SD_MODEL:  Single Write Start Token NG"); 
      if (token == 8'hfc && write_multi) `logI("SD_MODEL: Multiblock Write Start Token OK"); 
      else if ((token != 8'hfc && token != 8'hfd) && write_multi) `logI("SD_MODEL: Multiblock Write Start Token NG"); 
      if (token == 8'hfd && write_multi) begin 
        `logI("SD_MODEL: Multiblock Write Stop Token"); 
        st <= WriteStop; 
      end 
      i = 0; 
      while ( i < block_len ) begin 
        DataIn ; flash_mem[start_addr + i] = capture_data; 
        i = i + 1; 
      end 
      st <= WriteCRC; 
    end 

    WriteCRC : begin // Capture incoming CRC of data 
      i = 0; 
      while ( i < 16) begin 
        @(posedge sclk) crc16_in[15 - i] = mosi; 
        i = i + 1; 
      end 
      st <= DataResponse; 
    end 

    DataResponse: begin //All clock after data response CRC 
      DataOut(8'b00000101); 
      @(negedge sclk); 
      miso = 0; 
      repeat (tNEC*8) @( negedge sclk ) ; 
      repeat (tNDS*8) @( negedge sclk ) ; 
      miso = 1'bz; 
      @(negedge sclk); miso = 1'b0; 
      repeat (100) @(negedge sclk); 
      miso = 1; 
      @(negedge sclk); 
      miso = 1; 
      repeat (5) @( posedge sclk ); 
      if ( write_single ) st <= IDLE; 
      else if (write_multi) st <= WriteCycle; 
    end 

    WriteStop : begin 
      repeat (tNBR*8) @( posedge sclk ) ; 
      miso = 0; 
      repeat (tNEC*8) @( posedge sclk ) ; 
      repeat (tNDS*8) @( posedge sclk) miso = 1'bz; 
      @( posedge sclk) miso = 1'b0; #1000000;//1ms processing time for each block programming 
      @( posedge sclk) miso = 1'b1; 
      repeat (tNEC*8) @( posedge sclk ) ; 
      @( posedge sclk); 
      st <= IDLE; 
    end 

    // Trap state
    default : begin
      ;
    end
  endcase 
end 

always @(st) begin 
  case (st) 
    PowerOff      : ascii_command_state = "PowerOff"; 
    PowerOn       : ascii_command_state = "PowerOn";  
    IDLE          : ascii_command_state = "IDLE"; 
    CmdBit47      : ascii_command_state = "CmdBit47"; 
    CmdBit46      : ascii_command_state = "CmdBit46"; 
    CommandIn     : ascii_command_state = "CommandIn";
    CardResponse  : ascii_command_state = "CardResponse" ; 
    ReadCycle     : ascii_command_state = "ReadCycle"; 
    WriteCycle    : ascii_command_state = "WriteCycle"; 
    DataResponse  : ascii_command_state = "DataResponse"; 
    CsdCidScr     : ascii_command_state = "CsdCidScr";  
    WriteStop     : ascii_command_state = "WriteStop";  
    WriteCRC      : ascii_command_state = "WriteCRC";  
    default       : ascii_command_state = "ERROR";  
  endcase
end

initial begin 
  sck_cnt       = 0;   
  cmd_in        = 46'h3fffffffffff; 
  serial_in     = 46'h0; 
  crc16_in      = 16'h0; 
  crc7_in       = 7'h0;
  crc16_out     = 16'h0;
  token         = 8'h0;
  st            <= PowerOff; 
  miso          = 1'b1; 
  init_done     = 0; 
  ist           = 0; 
  capture_data  = 8'h0; 
  start_addr    = 32'h0; 
  VHS           = 4'h0; 
  serial_in1    = 46'h0; 
  multi_st      = 0; 
  block_len     = 512; 
  
  read_data     = 0;
  for (i = 0; i < MEM_SIZE - 1; i=i+1) begin 
    flash_mem[i] = 0; 
  end 
end 

endmodule 
