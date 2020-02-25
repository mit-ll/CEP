//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

`include "v2c_cmds.incl"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "config.v"
`include "v2c_top.incl"
`timescale 1ns/1ns
module cep_driver
  (
   input clk,
   input reset,
   input enableMe,
   output reg [31:0] __simTime
   );



//
// Added by script
parameter MY_SLOT_ID  = 4'h0,
  MY_LOCAL_ID = 4'h0;
// Done
//
//

//
//================================
// Internals
//================================
//
reg [255:0]    dvtFlags;
reg [255:0]    r_data;
initial begin
   dvtFlags = 0;
end

   wire clk100Mhz = clk;

   //
   // to support print command
   //
   reg [31:0] printf_adr;
   reg [1:0]  printf_coreId;
   reg [(128*8)-1:0] printf_buf; // 128bytes
   reg [(128*8)-1:0] tmp;
   reg 		     clear=0;
   integer 	     cnt;
   string 	     str;
   
   //
   always @(posedge dvtFlags[`DVTF_PRINTF_CMD]) begin
      printf_adr = dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO];
      //`logI("Calling PRINTF adr=0x%x",printf_adr);
      // go fill ther buffer
      cep_tb.read_ddr3_cache_n_clear(printf_adr,printf_buf[(128*8)-1:64*8]);
      cep_tb.read_ddr3_cache_n_clear(printf_adr | 'h40,printf_buf[(64*8)-1:0]);
      // left justify
      clear = 0;
      tmp = 0;
      // move trailing after newline or null
      for (cnt=0;cnt<128;cnt=cnt+1) begin
	 if (!clear && 
	     (printf_buf[(128*8)-1:(127*8)] != 'h0) && 
	     (printf_buf[(128*8)-1:(127*8)] != 'h0A) &&         // '\n'
	     (printf_buf[(128*8)-1:(127*8)] != 'h0D)) begin	// '\r'     
	    tmp = (tmp << 8) | printf_buf[(128*8)-1:(127*8)];
	    printf_buf = printf_buf << 8;
	 end
	 else begin
	    clear = 1;
	    tmp = tmp << 8;
	 end
      end
      //
      $sformat(str,"C%-d: %-s",printf_adr[1:0],tmp);
      $display("%s",str);
      //
      dvtFlags[`DVTF_PRINTF_CMD] = 0;
   end
   

   //
   //
   //
   always @(*) dvtFlags[`DVTF_READ_CALIBRATION_DONE] = `MIG_PATH.init_calib_complete;
   always @(*) dvtFlags[`DVTF_PROGRAM_LOADED]        = cep_tb.program_loaded;

   reg backdoor_enable = 0;
   always @(posedge dvtFlags[`DVTF_ENABLE_MEM_BACKDOOR]) begin
      backdoor_enable = dvtFlags[`DVTF_PAT_LO];
      dvtFlags[`DVTF_ENABLE_MEM_BACKDOOR] = 0;
      `logI("Setting backdoor_enable=%d",backdoor_enable);
   end

   always @(posedge dvtFlags[`DVTF_PUT_CORE_IN_RESET]) begin
      if (dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] == MY_LOCAL_ID) begin
	 force_core_in_reset();
      end
      dvtFlags[`DVTF_PUT_CORE_IN_RESET] = 0;
      
   end
   
   always @(*) dvtFlags[`DVTF_READ_CALIBRATION_DONE] = `MIG_PATH.init_calib_complete;   

 
//
// ================================================
// Task Lists
// ================================================
//
reg  rflag,cflag;
reg [15:0] rstatus;

//
// READ_STATUS
//
`define SHIPC_READ_STATUS_TASK READ_STATUS_TASK(__shIpc_p0)
task   READ_STATUS_TASK;
  output [31:0] r_data;
  begin
    // convert to port
`ifdef USE_DPI
     inBox.mPar[0] = 0;
`else     
     r_data = 0;
`endif     
     @(posedge clk100Mhz);
  end
endtask // READ_STATUS_TASK;

`define SHIPC_WRITE_DVT_FLAG_TASK WRITE_DVT_FLAG_TASK(__shIpc_p0,__shIpc_p1,__shIpc_p2)
task   WRITE_DVT_FLAG_TASK;
input [31:0] msb;
input [31:0] lsb;
input [31:0] value; 
integer s;
   begin
`ifdef USE_DPI
      for (s=inBox.mPar[1];s<=inBox.mPar[0];s=s+1) begin 
	 dvtFlags[s] = inBox.mPar[2] & 1'b1; 
	 inBox.mPar[2] = inBox.mPar[2] >> 1; 
      end      
`else
      for (s =lsb;s<=msb;s=s+1) begin dvtFlags[s] = value[0]; value = value >> 1; end
`endif
   //`logI("dvtFlags=%b",dvtFlags);
   @(posedge clk100Mhz);
   
end
endtask // WRITE_DVT_FLAG_TASK;

`define SHIPC_READ_DVT_FLAG_TASK READ_DVT_FLAG_TASK(__shIpc_p0,__shIpc_p1,__shIpc_p0)
task   READ_DVT_FLAG_TASK;
input [31:0] msb;
input [31:0] lsb;
output [31:0] r_data;
integer s;
reg [31:0] tmp;

begin
   tmp = 0;
`ifdef USE_DPI
   for (s=inBox.mPar[0];s>=inBox.mPar[1];s=s-1) begin
      tmp = {tmp[30:0],dvtFlags[s]};
   end
   inBox.mPar[0] = tmp;
`else
   for (s =msb;s>=lsb;s=s-1) begin 
     tmp = {tmp[30:0],dvtFlags[s]};
   end
   r_data = tmp;
`endif   
   //`logI("dvtFlags=%b r_data=%x",dvtFlags,r_data);
   @(posedge clk100Mhz);
   
end
endtask // READ_DVT_FLAG_TASK;

`define SHIPC_READ_ERROR_CNT_TASK READ_ERROR_CNT_TASK(__shIpc_p0)
task   READ_ERROR_CNT_TASK;
output [31:0] r_data;
   begin
`ifdef USE_DPI      
   $vpp_getErrorCount(inBox.mPar[0]);
`else      
   $vpp_getErrorCount(r_data);
`endif      
   //`logI("READ_ERROR_CNT_TASK = %x",r_data);
end
endtask // READ_ERROR_CNT_TASK;

   //
   // ---------------------------   
   // WRITE64_64
   // ---------------------------
   //
`ifdef USE_DPI
`define SHIPC_WRITE64_64_TASK WRITE64_64_DPI()
task   WRITE64_64_DPI;
begin
   //`logI("%m a=%x d=%x",a,d);
   //
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.write_ddr3_backdoor(inBox.mAdr,inBox.mPar[0]);
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
	1: `CORE1_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
	2: `CORE2_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,inBox.mPar[0]);
	3: `CORE3_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,inBox.mPar[0]);     
      endcase // case (MY_LOCAL_ID)
   end // else: !if(backdoor_enable)
`else
   cep_tb.write_ddr3_backdoor(inBox.mAdr,inBox.mPar[0]);               
`endif
end
endtask // WRITE64_64_TASK
`endif
   
   //
   // ---------------------------   
   // READ64_64
   // ---------------------------
   //
`ifdef USE_DPI   
`define SHIPC_READ64_64_TASK READ64_64_DPI()
task READ64_64_DPI;
   begin
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.read_ddr3_backdoor(inBox.mAdr,inBox.mPar[0]);      
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
	1: `CORE1_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
	2: `CORE2_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, inBox.mPar[0]);
	3: `CORE3_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, inBox.mPar[0]);     
      endcase // case (MY_LOCAL_ID)
   end
`else
      cep_tb.read_ddr3_backdoor(inBox.mAdr, inBox.mPar[0]);
`endif // !`ifdef BFM_MODE
      //`logI("%m a=%x d=%x",a,d);
   end
endtask // READ64_64_TASK
`endif
   
   //
   // ---------------------------   
   // WRITE32_64
   // ---------------------------
   //
`ifdef USE_DPI
`define SHIPC_WRITE32_64_TASK WRITE32_64_DPI()
task   WRITE32_64_DPI;
   reg [63:0] d;
begin
   //`logI("%m a=%x d=%x",a,d);
   //
   d[63:32] = inBox.mPar[0];
   d[31:0]  = inBox.mPar[1];
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.write_ddr3_backdoor(inBox.mAdr,d);
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,d);
	1: `CORE1_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,d);
	2: `CORE2_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,d);
	3: `CORE3_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, inBox.mAdr,d);     
      endcase // case (MY_LOCAL_ID)
   end // else: !if(backdoor_enable)
`else
   cep_tb.write_ddr3_backdoor(inBox.mAdr,d);               
`endif
end
endtask // WRITE32_64_TASK

`else   

// ================= OLD PLI  
`define SHIPC_WRITE32_64_TASK WRITE32_64_TASK(__shIpc_address[31:0],{__shIpc_p0[31:0],__shIpc_p1[31:0]})
task   WRITE32_64_TASK;
input [31:0] a;
input [63:0] d;
begin
   //`logI("%m a=%x d=%x",a,d);
   //
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.write_ddr3_backdoor(a,d);      
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, a, d);
	1: `CORE1_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, a, d);
	2: `CORE2_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, a, d);
	3: `CORE3_TL_PATH.tl_x_ul_write(MY_LOCAL_ID & 'h1, a, d);     
      endcase // case (MY_LOCAL_ID)
   end // else: !if(backdoor_enable)
`else
   cep_tb.write_ddr3_backdoor(a,d);               
`endif
end
endtask // WRITE32_64_TASK
`endif // !`ifdef USE_DPI
   
   //
   // ---------------------------   
   // READ32_64
   // ---------------------------
   //
`ifdef USE_DPI   
`define SHIPC_READ32_64_TASK READ32_64_DPI()
task READ32_64_DPI;
reg [63:0] d;
   begin
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.read_ddr3_backdoor(inBox.mAdr,d);      
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, d);
	1: `CORE1_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, d);
	2: `CORE2_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, d);
	3: `CORE3_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, inBox.mAdr, d);     
      endcase // case (MY_LOCAL_ID)
   end
`else
      cep_tb.read_ddr3_backdoor(inBox.mAdr,d);            
`endif // !`ifdef BFM_MODE
      
      inBox.mPar[0] = d[63:32];
      inBox.mPar[1] = d[31:0];      
      //`logI("%m a=%x d=%x",a,d);
   end
endtask // READ32_64_TASK
`else
   // -------------- OLD PLI     
`define SHIPC_READ32_64_TASK READ32_64_TASK(__shIpc_address[31:0],{__shIpc_p0[31:0],__shIpc_p1[31:0]})
task READ32_64_TASK;
input [31:0] a;
output [63:0] d;
   begin
`ifdef BFM_MODE
   if (backdoor_enable) begin
      cep_tb.read_ddr3_backdoor(a,d);      
   end
   else begin
      case (MY_LOCAL_ID)
	0: `CORE0_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, a, d);
	1: `CORE1_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, a, d);
	2: `CORE2_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, a, d);
	3: `CORE3_TL_PATH.tl_x_ul_read(MY_LOCAL_ID & 'h1, a, d);     
      endcase // case (MY_LOCAL_ID)
   end
`else
      cep_tb.read_ddr3_backdoor(a,d);            
`endif
      //`logI("%m a=%x d=%x",a,d);
   end
endtask // READ32_64_TASK
`endif
   
  //
  // =============================================
  // For playback support
  // =============================================
  //
   
  //
  // =============================================
  // SHIPC Support Common Codes
  // =============================================
  //
`include "v2c_cmds.incl"
`include "v2c_top.incl"
`define SHIPC_XACTOR_ID  MY_LOCAL_ID
`define SHIPC_CLK   clk
`include "driver_common.incl"
`undef   SHIPC_CLK
`undef SHIPC_XACTOR_ID      

//`include "common_xactor.incl"

   //
   // Put the core in reset that is not active
   //
   `define FORCE_RESET_IF_NOT_USED
   `ifdef FORCE_RESET_IF_NOT_USED
   //
   initial begin
      repeat(10) @(posedge clk);
      if (!myIsActive) begin
	 force_core_in_reset();	 
      end // if (!myIsActive)
   end // initial begin
   `endif //  `ifdef FORCE_RESET_IF_NOT_USED

   //
   task force_core_in_reset;
      begin
	 case (MY_LOCAL_ID)
	   0: begin
	      `logI("Forcing CORE#0 in reset...");
    `ifdef BARE_MODE
	      force `CORE0_PATH.core.reset =1;
    `endif
    `ifdef BFM_MODE
	      force `CORE0_PATH.reset =1;
    `endif
	   end
	   1: begin
	      `logI("Forcing CORE#1 in reset...");
    `ifdef BARE_MODE
	      force `CORE1_PATH.core.reset =1;
    `endif
    `ifdef BFM_MODE
	      force `CORE1_PATH.reset =1;
    `endif	      
	   end
	   2: begin
	      `logI("Forcing CORE#2 in reset...");
    `ifdef BARE_MODE
	      force `CORE2_PATH.core.reset =1;
    `endif
    `ifdef BFM_MODE
	      force `CORE2_PATH.reset =1;
    `endif	      
	   end
	   3: begin
	      `logI("Forcing CORE#3 in reset...");
    `ifdef BARE_MODE
	      force `CORE3_PATH.core.reset =1;
    `endif
    `ifdef BFM_MODE
	      force `CORE3_PATH.reset =1;
    `endif	      
	   end		 
	 endcase // case (MY_LOCAL_ID)
      end
   endtask
  
endmodule // qpic_oc12
