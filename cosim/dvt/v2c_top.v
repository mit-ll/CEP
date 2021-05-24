//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// Top DVT level testbench
//
`timescale 1ns/10ps
`include "v2c_cmds.incl"
`include "cep_adrMap.incl"
`include "cep_hierMap.incl"
`include "config.v"
module v2c_top
  (
   input      clk,
   output reg [31:0] __simTime=0
   );

`include "v2c_top.incl"
   
   integer vpp_cmd;
   
   // register myself
   reg 	   done__;
   //
   // Open input/output pipes with the outside world
   //
   always @(posedge clk) __simTime = __simTime + 1;
   //
   //
   // Debug stuff
   //
   reg 	      debug;
   reg 	      ipc_start;
   //
   // Save/Restart Support
   //
   reg 	      waitForRestartFlag;
   initial begin
      waitForRestartFlag = 0;
   end

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
   cep_tb.write_ddr3_backdoor(inBox.mAdr,inBox.mPar[0]);
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
    `CORE0_TL_PATH.tl_x_ul_read(0, inBox.mAdr, inBox.mPar[0]);
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
   cep_tb.write_ddr3_backdoor(inBox.mAdr,d);
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
   cep_tb.write_ddr3_backdoor(a,d);               
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
    `CORE0_TL_PATH.tl_x_ul_read(0, inBox.mAdr, d);
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
     `CORE0_TL_PATH.tl_x_ul_read(0, a, d);
`endif
      //`logI("%m a=%x d=%x",a,d);
   end
endtask // READ32_64_TASK
`endif
   
   
   //
   // shIpc stuffs
   //
   parameter 
     MY_SLOT_ID = `SYSTEM_SLOT_ID,
     MY_LOCAL_ID = `SYSTEM_CPU_ID;
`define SHIPC_CLK clk
`include "sys_common.incl"
`include "dump_control.incl"      
   //
   initial begin
      __shIpc_EnableMode = 0;
      #1;
      __shIpc_EnableMode = 1;      
   end
`undef   SHIPC_CLK

   always @(*) dvtFlags[`DVTF_READ_CALIBRATION_DONE] = `MIG_PATH.init_calib_complete;

   always @(posedge dvtFlags[`DVTF_SET_IPC_DELAY]) begin
      ipcDelay = dvtFlags[`DVTF_PAT_LO];
      dvtFlags[`DVTF_SET_IPC_DELAY] = 0;
   end

   //
   // This is to handle singel threading core: one core active at a time
   //
`ifdef RISCV_TESTS
   initial begin
      `logI("==== ISA RISCV_TESTS is active ===");      
   end
   int virtualMode = 0;
   always @(posedge dvtFlags[`DVTF_SET_VIRTUAL_MODE]) begin
//      virtualMode = 1;
//      `logI("==== Enable Virtual Mode");
      dvtFlags[`DVTF_SET_VIRTUAL_MODE]=0;
   end
   //
   int curCore = 0;
   reg singelThread = 0;
   reg [3:0] coreActiveMask = 0;
   always @(posedge dvtFlags[`DVTF_FORCE_SINGLE_THREAD]) begin
      coreActiveMask = dvtFlags[`DVTF_PAT_HI:`DVTF_PAT_LO] ;
      singelThread = 1;
      `logI("==== Force-ing Single Thread coreMask=0x%x===",coreActiveMask);
      dvtFlags[`DVTF_FORCE_SINGLE_THREAD] =0;
   end
   always @(posedge dvtFlags[`DVTF_PASS_IS_TO_HOST]) begin
      `CORE0_DRIVER.checkToHost = 1;
      `CORE1_DRIVER.checkToHost = 1;
      `CORE2_DRIVER.checkToHost = 1;
      `CORE3_DRIVER.checkToHost = 1;
      dvtFlags[`DVTF_PASS_IS_TO_HOST] = 0; // self-clear
   end   
   
   task ResetAllCores;
      begin
	 force `CORE0_PATH.reset = 1;
	 force `CORE1_PATH.reset = 1;
	 force `CORE2_PATH.reset = 1;
	 force `CORE3_PATH.reset = 1;
	 repeat (2) @(posedge clk);
	 release `CORE0_PATH.reset;	 
	 release `CORE1_PATH.reset;	 
	 release `CORE2_PATH.reset;	 
	 release `CORE3_PATH.reset;	 
      end
   endtask // for
   
   always @(posedge singelThread) begin
      // put all core in reset.
      force `CORE0_PATH.reset =1; 
      force `CORE1_PATH.reset =1;
      force `CORE2_PATH.reset =1;
      force `CORE3_PATH.reset =1;
      // wait for calibration complete
      @(posedge cep_tb.program_loaded);
      // allow caches to get out of reset but not the core!!!
      release `CORE0_PATH.reset; 
      release `CORE1_PATH.reset;
      release `CORE2_PATH.reset;
      release `CORE3_PATH.reset;
      //
      force `CORE0_PATH.core.reset =1; 
      force `CORE1_PATH.core.reset =1;
      force `CORE2_PATH.core.reset =1;
      force `CORE3_PATH.core.reset =1;      
      //
      for (int c=0;c<4;c=c+1) begin
	 if (coreActiveMask[c]) begin
	    case (c)
	      0: begin
		 `logI("Releasing CORE0 Reset....");
		 //cep_tb.putback_memory_used();
		 if (virtualMode) begin
		    ResetAllCores();
		 end
		 release `CORE0_PATH.core.reset; 
		 @(posedge (`CORE0_DRIVER.PassStatus || `CORE0_DRIVER.FailStatus));
	      end
	      1: begin
		 `logI("Releasing CORE1 Reset....");	
		 //cep_tb.putback_memory_used();
		 if (virtualMode) begin
		    ResetAllCores();
		 end		 
		 release `CORE1_PATH.core.reset;
		 @(posedge (`CORE1_DRIVER.PassStatus || `CORE1_DRIVER.FailStatus));
	      end 
	      2: begin
		 `logI("Releasing CORE2 Reset....");	
		 //cep_tb.putback_memory_used();
		 if (virtualMode) begin
		    ResetAllCores();
		 end		 
		 release `CORE2_PATH.core.reset;
		 @(posedge (`CORE2_DRIVER.PassStatus || `CORE2_DRIVER.FailStatus));
	      end
	      3: begin
		 `logI("Releasing CORE3 Reset....");	
		 //cep_tb.putback_memory_used();
		 if (virtualMode) begin
		    ResetAllCores();
		 end		 
		 release `CORE3_PATH.core.reset;
		 @(posedge (`CORE3_DRIVER.PassStatus || `CORE3_DRIVER.FailStatus));
	      end		   
	    endcase
	 end
      end // for (int c=0;c<4;c=c+1)
   end // always @ (posedge singelThread)
`endif
   
endmodule
