//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      playback.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    Playack task to replay command sequences captured from chip level BFM mode
// Notes:
//
//************************************************************************
`ifndef PLAYBACK_CMD_H
`define PLAYBACK_CMD_H
// Write to : <physicalAdr> <writeData>
`define WRITE__CMD  1
// Read and compare: <physicalAdr> <Data2Compare>
`define RDnCMP_CMD  2
// Read and spin until match : <physicalAdr> <Data2Match> <mask> <timeout>
`define RDSPIN_CMD  3

`define WRITE__CMD_SIZE  3
`define RDnCMP_CMD_SIZE  3
`define RDSPIN_CMD_SIZE  5
`endif //  `ifndef PLAYBACK_CMD_H

//
//
// System verilog uses to playback the sequences 
//
task cep_raw_write(input longint adr, input longint data);
   begin
      driver.tl_x_ul_write(0,adr,data);      
   end
endtask // if

task cep_raw_read(input longint adr, output longint data);
   begin
      driver.tl_x_ul_read(0,adr,data);            
   end
endtask // if

//
// Check if it is within range since the vectors file might involve multiple modules (For example: DFT/IDFT are captured together)
//
function inRange(input longint adr, input longint upperAdr,input longint lowerAdr);
  begin
     inRange = (adr >= lowerAdr) && (adr < upperAdr);
  end
endfunction

//
//
task automatic cep_playback(ref longint seqPtr[], input longint upperAdr, input longint lowerAdr,
			    input int totalCmds, input int totalSize, input int verbose, output int errCnt);
   //
   int 			      i=0, TO;
   longint 		      rdDat;      
   
   //
   begin
      //
      if (verbose) begin
	 $display("%m: playback command sequence totalCmds=%0d totalSize=%0d range=0x%0x:%0x\n",
		  totalCmds,totalSize,lowerAdr,upperAdr-1);
      end
      for (int c=1;c<=totalCmds;c++) begin
	 //
	 // Execute Write Command
	 //
	 if (seqPtr[i] == `WRITE__CMD) begin
	    if (inRange(seqPtr[i+1],upperAdr,lowerAdr)) begin	 
	       cep_raw_write(seqPtr[i+1],seqPtr[i+2]);
	       if (verbose) begin
		  $display("cmd=%4d Write:  adr=0x%08x data=0x%016x",	       
			   c, seqPtr[i+1],seqPtr[i+2]);	
	       end
	    end
	    i += `WRITE__CMD_SIZE;	    
	 end
	 //
	 // Execute Read_and_Compare Command
	 //	 
	 else if (seqPtr[i] == `RDnCMP_CMD) begin
	    if (inRange(seqPtr[i+1],upperAdr,lowerAdr)) begin	 	    
	       cep_raw_read(seqPtr[i+1],rdDat);
	       if (rdDat !== seqPtr[i+2]) begin
		  $display("cmd=%4d ***ERROR*** Mismatch adr=0x%08x  exp=0x%016x act=0x%016x",
			   c, seqPtr[i+1],seqPtr[i+2],rdDat);
		  errCnt = c;
		  break;	       
	       end 
	       else if (verbose) begin
		  $display("cmd=%4d Read_OK adr=0x%08x exp= 0x%016x act=0x%016x",
			   c, seqPtr[i+1],seqPtr[i+2],rdDat);	
	       end
	    end
	    i += `RDnCMP_CMD_SIZE;
	 end // if (seqPtr[i] == RDnCMP_CMD)
	 //
	 // Execute Read_and_Spin Command
	 //	 	 
	 else if (seqPtr[i] == `RDSPIN_CMD) begin
	    if (inRange(seqPtr[i+1],upperAdr,lowerAdr)) begin	 	    
	       TO = seqPtr[i+4];
	       while (TO > 0) begin
		  cep_raw_read(seqPtr[i+1],rdDat);
		  if (((rdDat ^ seqPtr[i+2]) & seqPtr[i+3]) == 0) begin
		     break;
		  end
		  TO--;
		  if (TO <= 0) begin
		     $display("cmd=%4d ***ERROR***: timeout adr=0x%08x exp=0x%016x act=0x%016x",
			      c, seqPtr[i+1],seqPtr[i+2],rdDat);
		     errCnt = c;
		     break;
		  end
	       end // while (TO > 0)
	       if (verbose)  begin
		  $display("cmd=%4d Spin_OK adr=0x%08x act= 0x%016x timeOutRemain=%0d",
			   c, seqPtr[i+1],rdDat, TO);			  
	       end
	    end
	    i += `RDSPIN_CMD_SIZE;
	 end // if (seqPtr[i] == `RDSPIN_CMD)
	 if (errCnt) break;
      end // for (int c=1;c<=totalCmds;c++)
   end
endtask // if



