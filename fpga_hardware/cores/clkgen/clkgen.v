/*
 *
 * Clock, reset generation unit for ML501 board
 * 
 * Implements clock generation according to design defines
 * 
 */
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2009, 2010 Authors and OPENCORES.ORG           ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////

`include "orpsoc-defines.v"
`include "synthesis-defines.v"

module clkgen
  (
   // Main clocks in, depending on board
   sys_clk_in_p, sys_clk_in_n,

   // Wishbone clock and reset out  
   wb_clk_o,
   wb_rst_o,

      // JTAG clock
`ifdef JTAG_DEBUG
   tck_pad_i,
   dbg_tck_o,
`endif 

   // Asynchronous, active low reset in
   rst_n_pad_i
   
   );

   input  sys_clk_in_p,sys_clk_in_n;   

   output wb_rst_o;
   output wb_clk_o;

`ifdef JTAG_DEBUG
   input  tck_pad_i;
   output dbg_tck_o;
`endif  

   // Asynchronous, active low reset (pushbutton, typically)
   input  rst_n_pad_i;
   
   // First, deal with the asychronous reset
   wire   async_rst;
   wire   async_rst_n;

   // Xilinx synthesis tools appear cluey enough to instantiate buffers when and
   // where they're needed, so we do simple assigns for this tech.
   assign async_rst_n = rst_n_pad_i;   

   // Everyone likes active-high reset signals...
   assign async_rst = ~async_rst_n;

  `ifdef JTAG_DEBUG   
    assign dbg_tck_o = tck_pad_i;
  `endif

   //
   // Declare synchronous reset wires here
   //
   
   // An active-low synchronous reset signal (usually a PLL lock signal)
   wire   sync_rst_n;

   // An active-low synchronous reset from ethernet PLL
   wire   sync_eth_rst_n;


   assign wb_clk_o = sys_clk_in_p;
   assign sync_rst_n = 1'b1;
   
   //
   // Reset generation
   //
   //

   // Reset generation for wishbone
   reg [15:0] 	   wb_rst_shr;
   always @(posedge wb_clk_o or posedge async_rst)
     if (async_rst)
       wb_rst_shr <= 16'hffff;
     else
       wb_rst_shr <= {wb_rst_shr[14:0], ~(sync_rst_n)};
   
   assign wb_rst_o = wb_rst_shr[15];
   
   
endmodule // clkgen