//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      tlul_pkg.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

package tlul_pkg ;
   
   
//
// TL-UL Interface
//
  
`ifndef TL_OPCODES_H
`define TL_OPCODES_H
//
// TileLink Opcodes
//
`define TL_A_PUTFULLDATA	0 	
`define TL_A_PUTPARTIALDATA 	1 
`define TL_A_ARITHMETICDATA 	2 
`define TL_A_LOGICALDATA 	3 
`define TL_A_GET 		4 
`define TL_A_INTENT 		5 
`define TL_A_ACQUIRE 		6 	

`define TL_B_PUTFULLDATA 	0 
`define TL_B_PUTPARTIALDATA 	1 
`define TL_B_ARITHMETICDATA 	2 
`define TL_B_LOGICALDATA 	3 
`define TL_B_GET 		4 
`define TL_B_INTENT 		5 
`define TL_B_PROBE 		6 

`define TL_C_ACCESSACK 		0 	
`define TL_C_ACCESSACKDATA 	1 
`define TL_C_HINTACK 		2 	
`define TL_C_PROBEACK 		4 	
`define TL_C_PROBEACKDATA 	5 
`define TL_C_RELEASE RELEASE 	6 	
`define TL_C_RELEASEDATA 	7 

`define TL_D_ACCESSACK 		0 	
`define TL_D_ACCESSACKDATA 	1 
`define TL_D_HINTACK 		2 	
`define TL_D_GRANT 		4 
`define TL_D_GRANTDATA 		5 	
`define TL_D_RELEASEACK 	6 	
//
// Parameters
// 
// Cap types (Grant = new permissions, Probe = permisions <= target)
`define CAP_toT    0
`define CAP_toB    1
`define CAP_toN    2

// Grow types (Acquire = permissions >= target)
`define GROW_NtoB  0
`define GROW_NtoT  1
`define GROW_BtoT  2
 
// Shrink types (ProbeAck, Release)
`define SHRINK_TtoB 0
`define SHRINK_TtoN 1
`define SHRINK_BtoN 2

// Report types (ProbeAck, Release)
`define REPORT_TtoT 3
`define REPORT_BtoB 4
`define REPORT_NtoN 5
//
`endif //  `ifdef TL_OPCODES_H
//

//
// Channel-A/D for UL only for now!!!
//

typedef struct packed {
   logic [31:0] a_address;
   logic [7:0] 	a_mask;
   logic [63:0] a_data;
   logic [7:0] 	a_source;
   logic 	a_corrupt;
   logic 	a_valid;
   logic [2:0] 	a_opcode;
   logic [2:0] 	a_param;
   logic [1:0] 	a_size;
   //
   logic 	d_ready; // from other channel
} tl_chA_t;

typedef struct packed {
   logic [7:0] d_source;
   logic [1:0] d_sink;     
   logic [63:0] d_data;
   logic 	d_corrupt;
   logic 	d_valid;
   logic [2:0] 	d_opcode;
   logic [2:0] 	d_param;
   logic [1:0] 	d_size;
   logic 	d_denied;
   //
   logic 	a_ready; // from other channel
} tl_chD_t;


`define TICK_DELAY #1
`define MAX_TIMEOUT 100
   
endpackage : tlul_pkg // tlul_pkg
   
