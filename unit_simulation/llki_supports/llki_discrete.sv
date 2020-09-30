//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      llki_discrete.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// LLKI-discret Interface
//
`include "llki_struct.h"
//   
interface llki_discrete_if #(core_id=0) ();
   //
   // IOs
   //
   logic [63:0] key_data;
   logic 	key_valid;
   logic 	key_ready;
   logic 	key_complete;
   logic 	clear_key;
   logic 	clear_key_ack;
   // added llki_struct output as keys are captured in there for XORings
   llki_s       rx_key_s;
   //
   // Master Mode
   //
   modport master(input key_ready, key_complete, clear_key_ack,
		  output key_data, key_valid, clear_key
		  );
   //
   // Slave Mode
   //
   modport slave(input key_data, key_valid, clear_key,
		 output key_ready, key_complete, clear_key_ack,
		 output rx_key_s
		 );
   
endinterface // llki_discrete
//
// LLKI Slave (bfm mode: used in core's RTL as place holder/fake respond until replaced with real logic)
//
module llki_discrete_slave #(parameter llki_s MY_STRUCT)
  (
   llki_discrete_if.slave llki,
   input           clk, 
   input           rst
   );

   reg [3:0] 	   keyNum=0;
   reg [1:0]	   state;
   reg [3:0] 	   counter;
   localparam 
     IDLE      = 2'b0,
     WAIT4DONE = 2'b01,
     DELAY_ACK = 2'b11;

   //
   always @(posedge clk or posedge rst) begin
      if (rst) begin
	 llki.key_ready     <= 1;
	 llki.key_complete  <= 0;
	 llki.clear_key_ack <= 0;
	 keyNum             <= 0;
	 counter            <= 0;
	 //
	 llki.rx_key_s.core_id     <= 0;
	 for (int i=0;i<`MAX_KEY;i++) 
	   llki.rx_key_s.key[i]    <= 0;
	 llki.rx_key_s.key_size    <= 0;
	 llki.rx_key_s.base        <= 0;
	 //
	 state              <= IDLE;
      end
      else begin
	 llki.clear_key_ack  <= 0; // pulse
	 case (state)
	   IDLE: begin
	      if (llki.clear_key && !llki.clear_key_ack) begin
		 //synthesis translate_off
		 $display("%m: Receiving clear_key to core_id=%0d",MY_STRUCT.core_id);
		 //synthesis translate_on	 
		 keyNum              <= 0;
		 llki.clear_key_ack  <= 1;
		 llki.key_complete   <= 0;
		 llki.rx_key_s.core_id     <= 0;
		 for (int i=0;i<`MAX_KEY;i++) 
		   llki.rx_key_s.key[i]    <= 0;
		 llki.rx_key_s.key_size    <= 0;
		 llki.rx_key_s.base        <= 0;
		 //
		 state               <= WAIT4DONE; // wait for done
	      end
	      else if (llki.key_valid && llki.key_ready) begin
		 //
		 // CHeck to make sure I got the expected key
		 //
		 //if ((keyNum < MY_STRUCT.key_size) && (llki.key_data == MY_STRUCT.key[keyNum])) begin
		 if (keyNum < MY_STRUCT.key_size) begin //  no checking
		    //synthesis translate_off		    
		    $display("%m: Receiving key%0d = 0x%x to core_id=%0d OK ",keyNum,llki.key_data,MY_STRUCT.core_id);
		    //synthesis translate_on
		    llki.rx_key_s.core_id     <= MY_STRUCT.core_id;
		    llki.rx_key_s.base        <= MY_STRUCT.base;
		    llki.rx_key_s.key[keyNum] <= llki.key_data ;
		    llki.rx_key_s.key_size    <= keyNum + 1;
		    //
		    llki.key_ready      <= 0; // take it away
		    keyNum              <= keyNum + 1; // next key
		    counter             <= 0;
		    //
		    state               <= DELAY_ACK;
		 end
		 else begin // bad key
		    //synthesis translate_off		    		    
		    $display("%m: ERROR: Receiving UNEXPECTED key%0d = 0x%x to core_id=%0d",keyNum,llki.key_data,MY_STRUCT.core_id);	 
		    //synthesis translate_on
		    llki.key_ready      <= 1;
		    keyNum              <= 0;
		    state               <= WAIT4DONE;
		 end // else: !if((keyNum < MY_STRUCT.key_size) && (llki.key_data == MY_STRUCT.key[keyNum]))
	      end // if (llki.key_valid && llki.key_ready)
	   end // case: IDLE
	   
	   WAIT4DONE: begin
	      if ((llki.key_valid==0) && (llki.clear_key==0)) begin
		 state               <= IDLE;
	      end
	   end

	   DELAY_ACK: begin 
	      counter <= counter + 1;
	      if (counter == 10) begin // FIXME!!!! delay some cycles based on attributes before assert ready
		 if (keyNum == MY_STRUCT.key_size) begin
		    llki.key_complete   <= 1;
		    //synthesis translate_off
		    `ifdef CADENCE
		    $displayh("%m: Received struct=%h",llki.rx_key_s.key[0]);
		    $displayh("%m: Expected struct=%h",MY_STRUCT.key[0]);
		    `else
		    $displayh("%m: Received struct=%p",llki.rx_key_s);
		    $displayh("%m: Expected struct=%p",MY_STRUCT);		    
		    `endif
		    //synthesis translate_on		    
		 end
		 llki.key_ready      <= 1;
		 state               <= WAIT4DONE;
	      end
	   end
	 endcase
      end // else: !if(rst)
   end // always @ (posedge clk or posedge rst)
   
endmodule // llki_discrete_master

//
// LLKI master (bfm mode: used in unit testbench until tileLink --> LLKI-Discrete is supported)
//
module llki_discrete_master
  (
   llki_discrete_if.master llki,
   input clk, 
   input rst
   );

   //
   //
   //
`include "../llki_supports/llki_rom.sv"
   
   initial begin
      repeat (2) @(posedge clk);
      llki.key_data      = 'h123456789ABCDEF;
      llki.key_valid     = 0;
      llki.clear_key     = 0;
   end
   //
   // task to send messages
   //
   int timeOut;
   //
   // Use in cycle-accurate TBs
   //
   task unlockReq(output errCnt);
      begin
`ifndef CADENCE	 
	 $displayh("=== Running Unlocking Sequence for core_id=%0d : %p ===", llki.core_id,llki_rom[llki.core_id]);
`else
	 $displayh("=== Running Unlocking Sequence for core_id=%0d : %h ===", llki.core_id,llki_rom[llki.core_id].key[0]);	 
`endif	 
	 // access the ROM to get # of keys
	 for (int i=0;i<llki_rom[llki.core_id].key_size;i++) begin
	    $display("%m: Loading key%0d = 0x%x to core_id=%0d",i,llki_rom[llki.core_id].key[i],llki.core_id);	    
	    loadKey(llki_rom[llki.core_id].key[i],errCnt);
	    if (errCnt) break;
	 end // if (!errCnt)
      end
   endtask //
   //
   // to be called by TL-slave's BFM
   //
   task wait4Ready(input int timeOut, output errCnt);
     begin
	errCnt = 0;
	// wait until it is ready
	timeOut = 100;
	while ((llki.key_ready==0) && (timeOut >= 0)) begin
	   @(posedge clk);
	   timeOut--;
	   if (timeOut <= 0) begin
	      $display("%m: ERROR timeout while waiting for key_ready");
	      errCnt=1;
	      break;
	   end
	end
     end
   endtask
      
   task clearKey(output errCnt);
      begin
	 $displayh("=== Running clearKey for core_id=%0d : ===", llki.core_id);	 
	// wait until it is ready
	wait4Ready(100,errCnt);
	if (!errCnt) begin
	   @(negedge clk);
	   llki.clear_key = 1;
	   @(negedge clk);
	   llki.clear_key = 0;
	   timeOut = 100;
	   while ((llki.clear_key_ack==0) && (timeOut >= 0)) begin
	      timeOut--;
	      @(posedge clk);		  
	      if (timeOut <= 0) begin
		 $display("%m: ERROR timeout while waiting for clear_key_ack");
		 errCnt=1;
		 break;
	      end
	   end
	   repeat (2) @(posedge clk);		  	       
	end // for (int i=0;i<llki_rom[llki.core_id].key_size;i++)
      end
   endtask
	
   task loadKey(input [63:0] key, output errCnt);
      int timeOut;
      begin
	 // wait until it is ready
	 wait4Ready(100,errCnt);	 
	 if (!errCnt) begin
	    @(negedge clk);
	    llki.key_data  = key;
	    llki.key_valid = 1;
	    @(negedge clk);
	    llki.key_valid = 0;	       
	    timeOut = 100;
	    while ((llki.key_ready==0) && (timeOut >= 0)) begin
	       timeOut--;
	       @(posedge clk);		  
	       if (timeOut <= 0) begin
		  $display("%m: ERROR timeout while waiting for key_ready");
		  errCnt=1;
		  break;
	       end
	    end
	    repeat (2) @(posedge clk);		  	       
	 end // for (int i=0;i<llki_rom[llki.core_id].key_size;i++)
      end
   endtask
endmodule // llki_discrete_master

   
