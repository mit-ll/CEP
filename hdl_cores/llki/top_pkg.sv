// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:       top_pkg.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     OpenTitan top_pkg customized for the Common 
//                  Evaluation Plafform
// Notes:           
//************************************************************************

package top_pkg;

// These should match the SROTTilelinkParameters called out in cep_addresses.scala
localparam int TL_DW=64;    // = TL_DBW * 8; TL_DBW must be a power-of-two
localparam int TL_AW=32;
localparam int TL_AIW=4;    // a_source, d_source
localparam int TL_DIW=2;    // d_sink
localparam int TL_SZW=2;	// a_size, d_size


localparam int TL_DUW=16;   // d_user (unused in the CEP)
localparam int TL_DBW=(TL_DW>>3);
//localparam int TL_SZW=$clog2($clog2(TL_DBW)+1);
localparam int FLASH_BANKS=2;
localparam int FLASH_PAGES_PER_BANK=256;
localparam int FLASH_WORDS_PER_PAGE=128;
localparam int FLASH_INFO_TYPES = 2;
localparam int FLASH_INFO_PER_BANK [FLASH_INFO_TYPES] = '{4, 4};
localparam int FLASH_DATA_WIDTH=64;
localparam int FLASH_METADATA_WIDTH=12;
localparam int NUM_AST_ALERTS=7;
localparam int NUM_IO_RAILS=2;
localparam int ENTROPY_STREAM=4;
localparam int ADC_CHANNELS=2;
localparam int ADC_DATAW=10;

endpackage
