//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      llki_struct.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

`ifndef LLKI_STRUCT_H
`define LLKI_STRUCT_H

//
// Used for both Command&Control (C2) and Key Load (KL)
//
typedef enum { NoOp, LoadKeyReq, ClearKeyReq, KeyStatusReq, LoadKeyAck, ClearKeyAck, KeyStatusResp} LLKI_CMDS;
//
// LLKI ROM table structrue in SRoT
// 256-bit for now. Must be multiple of 64-bits. The actual core's locking logic can use as much as it wants upto keySize
//
`define MAX_KEY 4

//
typedef bit [63:0] keyUnit;
typedef struct packed {
  int 	  core_id;
  int 	  attributes;        	// place holder
  int 	  key_size;          	// in unit of 64-bits
  longint base;              	// used by TL to access the core's TL logic to unlock
  // Cadence tools can't handle [`MAX_KEY]
  keyUnit [0:`MAX_KEY-1] key ;  // contains the keys. Word[0] = MSB, word[size-1]=LSB (big endian)
} llki_s;

//
// CORE's ID to index to llki_rom structures
//
`define AES_ID              0
`define MD5_ID              1
`define SHA256_ID           2
`define RSA_ID              3
`define DES3_ID             4
`define DFT_ID              5
`define IDFT_ID             6
`define FIR_ID              7
`define IIR_ID              8
`define GPS_ID              9
`define MAX_CORES           10


// ===============
`endif
