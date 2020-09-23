//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
module RocketTile_beh( // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285893.2]
		       /* OLD
  input 	clock, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285894.4]
  input 	reset, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285895.4]
  input 	auto_intsink_in_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_int_in_xing_in_2_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_int_in_xing_in_1_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_int_in_xing_in_0_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_int_in_xing_in_0_sync_1, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_a_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_a_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [2:0] 	auto_tl_master_xing_out_a_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [2:0] 	auto_tl_master_xing_out_a_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
3:0  output [2:0] 	auto_tl_master_xing_out_a_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [1:0] 	auto_tl_master_xing_out_a_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [31:0] auto_tl_master_xing_out_a_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [7:0] 	auto_tl_master_xing_out_a_bits_mask, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [63:0] auto_tl_master_xing_out_a_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_a_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_b_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_b_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
xx  input [2:0] 	auto_tl_master_xing_out_b_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [1:0] 	auto_tl_master_xing_out_b_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
xx  input [2:0] 	auto_tl_master_xing_out_b_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [1:0] 	auto_tl_master_xing_out_b_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [31:0] 	auto_tl_master_xing_out_b_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
xx  input [7:0] 	auto_tl_master_xing_out_b_bits_mask, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
xx  input 	auto_tl_master_xing_out_b_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_c_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_c_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [2:0] 	auto_tl_master_xing_out_c_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [2:0] 	auto_tl_master_xing_out_c_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
3:0  output [2:0] 	auto_tl_master_xing_out_c_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [1:0] 	auto_tl_master_xing_out_c_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [31:0] auto_tl_master_xing_out_c_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output [63:0] auto_tl_master_xing_out_c_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_c_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_d_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_d_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [2:0] 	auto_tl_master_xing_out_d_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [1:0] 	auto_tl_master_xing_out_d_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
3:0  input [2:0] 	auto_tl_master_xing_out_d_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [1:0] 	auto_tl_master_xing_out_d_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
1:0  input [2:0] 	auto_tl_master_xing_out_d_bits_sink, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_d_bits_denied, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [63:0] 	auto_tl_master_xing_out_d_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_d_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input 	auto_tl_master_xing_out_e_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  output 	auto_tl_master_xing_out_e_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
1:0  output [2:0] 	auto_tl_master_xing_out_e_bits_sink, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285896.4]
  input [1:0] 	constants_hartid // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@285897.4]
			*/
  input         clock, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236736.4]
  input         reset, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236737.4]
  input         auto_intsink_in_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_int_in_xing_in_2_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_int_in_xing_in_1_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_int_in_xing_in_0_sync_0, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_int_in_xing_in_0_sync_1, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_a_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_a_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [2:0]  auto_tl_master_xing_out_a_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [2:0]  auto_tl_master_xing_out_a_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [3:0]  auto_tl_master_xing_out_a_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [1:0]  auto_tl_master_xing_out_a_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [31:0] auto_tl_master_xing_out_a_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [7:0]  auto_tl_master_xing_out_a_bits_mask, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [63:0] auto_tl_master_xing_out_a_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_a_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_b_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_b_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  auto_tl_master_xing_out_b_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  auto_tl_master_xing_out_b_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [31:0] auto_tl_master_xing_out_b_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_c_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_c_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [2:0]  auto_tl_master_xing_out_c_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [2:0]  auto_tl_master_xing_out_c_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [3:0]  auto_tl_master_xing_out_c_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [1:0]  auto_tl_master_xing_out_c_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [31:0] auto_tl_master_xing_out_c_bits_address, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [63:0] auto_tl_master_xing_out_c_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_c_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_d_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_d_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [2:0]  auto_tl_master_xing_out_d_bits_opcode, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  auto_tl_master_xing_out_d_bits_param, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [3:0]  auto_tl_master_xing_out_d_bits_size, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  auto_tl_master_xing_out_d_bits_source, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  auto_tl_master_xing_out_d_bits_sink, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_d_bits_denied, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [63:0] auto_tl_master_xing_out_d_bits_data, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_d_bits_corrupt, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input         auto_tl_master_xing_out_e_ready, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output        auto_tl_master_xing_out_e_valid, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  output [1:0]  auto_tl_master_xing_out_e_bits_sink, // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236738.4]
  input  [1:0]  constants_hartid // @[:sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.fir@236739.4]
		       
);

   localparam PROBE_SUPPORT=1'b0; // ignore b_valid
   
   //
tl_master_beh    tl_master(
    .clock			(clock),
   .reset			(reset),
   .tl_master_a_ready 		(auto_tl_master_xing_out_a_ready),
   .tl_master_a_valid 		(auto_tl_master_xing_out_a_valid),
   .tl_master_a_bits_opcode 	(auto_tl_master_xing_out_a_bits_opcode),
   .tl_master_a_bits_param 	(auto_tl_master_xing_out_a_bits_param),
   .tl_master_a_bits_size 	(auto_tl_master_xing_out_a_bits_size),
   .tl_master_a_bits_source 	(auto_tl_master_xing_out_a_bits_source),
   .tl_master_a_bits_address 	(auto_tl_master_xing_out_a_bits_address),
   .tl_master_a_bits_mask 	(auto_tl_master_xing_out_a_bits_mask),
   .tl_master_a_bits_data 	(auto_tl_master_xing_out_a_bits_data),
   .tl_master_a_bits_corrupt 	(auto_tl_master_xing_out_a_bits_corrupt),
   .tl_master_b_ready 		(auto_tl_master_xing_out_b_ready),
   .tl_master_b_valid 		(auto_tl_master_xing_out_b_valid),
   .tl_master_b_bits_opcode 	(3'h6), // auto_tl_master_xing_out_b_bits_opcode),
   .tl_master_b_bits_size 	(4'h6), // auto_tl_master_xing_out_b_bits_size),
   .tl_master_b_bits_mask 	(8'hff), // auto_tl_master_xing_out_b_bits_mask),	
   .tl_master_b_bits_corrupt 	(1'b0), // auto_tl_master_xing_out_b_bits_corrupt),		   
   .tl_master_b_bits_param 	(auto_tl_master_xing_out_b_bits_param),
   .tl_master_b_bits_source 	(auto_tl_master_xing_out_b_bits_source),
   .tl_master_b_bits_address 	(auto_tl_master_xing_out_b_bits_address),
   .tl_master_c_ready 		(auto_tl_master_xing_out_c_ready),
   .tl_master_c_valid 		(auto_tl_master_xing_out_c_valid),
   .tl_master_c_bits_opcode 	(auto_tl_master_xing_out_c_bits_opcode),
   .tl_master_c_bits_param 	(auto_tl_master_xing_out_c_bits_param),
   .tl_master_c_bits_size 	(auto_tl_master_xing_out_c_bits_size),
   .tl_master_c_bits_source 	(auto_tl_master_xing_out_c_bits_source),
   .tl_master_c_bits_address 	(auto_tl_master_xing_out_c_bits_address),
   .tl_master_c_bits_data 	(auto_tl_master_xing_out_c_bits_data),
   .tl_master_c_bits_corrupt 	(auto_tl_master_xing_out_c_bits_corrupt),
   .tl_master_d_ready 		(auto_tl_master_xing_out_d_ready),
   .tl_master_d_valid 		(auto_tl_master_xing_out_d_valid),
   .tl_master_d_bits_opcode 	(auto_tl_master_xing_out_d_bits_opcode),
   .tl_master_d_bits_param 	(auto_tl_master_xing_out_d_bits_param),
   .tl_master_d_bits_size 	(auto_tl_master_xing_out_d_bits_size),
   .tl_master_d_bits_source 	(auto_tl_master_xing_out_d_bits_source),
   .tl_master_d_bits_sink 	(auto_tl_master_xing_out_d_bits_sink),
   .tl_master_d_bits_denied 	(auto_tl_master_xing_out_d_bits_denied),
   .tl_master_d_bits_data 	(auto_tl_master_xing_out_d_bits_data),
   .tl_master_d_bits_corrupt 	(auto_tl_master_xing_out_d_bits_corrupt),
   .tl_master_e_ready 		(auto_tl_master_xing_out_e_ready),
   .tl_master_e_valid 		(auto_tl_master_xing_out_e_valid),
   .tl_master_e_bits_sink 	(auto_tl_master_xing_out_e_bits_sink),
   .constants_hartid 		(constants_hartid)
   );

   
endmodule // RocketTile
