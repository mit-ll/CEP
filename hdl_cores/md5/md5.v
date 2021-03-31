//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      md5.v
// Program:        Common Evaluation Platform (CEP)
// Description:    MD5 wrapper
// Notes:          The actual core expects input in LE format and output 
//                 is already in BE format. 
//                 This module is to make the inout/output conform to 
//                 BigEndian format (standardized from CEP perspective)
//
//************************************************************************
module md5
  (
   input 	  clk, // input clock
   input 	  rst, // global rst
   input          init,  // clear internal states to start a new transaction   
   input [511:0]  msg_padded, // input message, already padded
   input 	  msg_in_valid, // next transaction with current encryption
   
   output [127:0] msg_output, // output message, always 128 bit wide
   output 	  msg_out_valid, // if asserted, output message is valid
   output 	  ready                    // the core is ready for an input message
   );
   
   pancham md5_core
     (
      .msg_padded   ({xEndian(msg_padded[(0*64)+63: (0*64)]),
		      xEndian(msg_padded[(1*64)+63: (1*64)]),
		      xEndian(msg_padded[(2*64)+63: (2*64)]),
		      xEndian(msg_padded[(3*64)+63: (3*64)]),
		      xEndian(msg_padded[(4*64)+63: (4*64)]),
		      xEndian(msg_padded[(5*64)+63: (5*64)]),
		      xEndian(msg_padded[(6*64)+63: (6*64)]),
		      xEndian(msg_padded[(7*64)+63: (7*64)])}),		   
      // output already math the endian
      .msg_output    (msg_output),
      .clk           (clk),
      .rst           (rst),
      .init          (init ),
      .msg_in_valid  (msg_in_valid),
      .msg_out_valid (msg_out_valid), 
      .ready         (ready)
      );
   
   function [63:0] xEndian;
      input [63:0] inDat;
      begin
	 xEndian = {inDat[(0*8)+7:(0*8)],
		    inDat[(1*8)+7:(1*8)],
		    inDat[(2*8)+7:(2*8)],
		    inDat[(3*8)+7:(3*8)],
		    inDat[(4*8)+7:(4*8)],
		    inDat[(5*8)+7:(5*8)],
		    inDat[(6*8)+7:(6*8)],
		    inDat[(7*8)+7:(7*8)]};
      end
   endfunction
  

endmodule // md5
