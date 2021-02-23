//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      llki_rom.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
`include "llki_struct.h"
//
// struct to store the keys, convert to ROM to hardwired into SRoT module
//
llki_s llki_rom[`MAX_CORES] = '{default:0};

//
// Constant struct
// Change the key pattern and its size to match the real one as they become available...
// NOTE: max keysize is 256-bits for now but can be extend as big as we want..See llki_struct.h
//
parameter llki_s AES_LLKI_STRUCT    ='{core_id    : `AES_ID,
				       attributes : 0,
				       key_size   : 2,
				       base       : 'h7000FF00,
				       key        : '{'h12345678_9ABCDEF0,'h600dbabe_baddade0,'h0,'h0}};

parameter llki_s MD5_LLKI_STRUCT    ='{core_id    : `MD5_ID,
				       attributes : 0,
				       key_size   : 3,
				       base       : 'h7000FF00,
				       key        : '{'h33333333_44444444,'h12345678_9ABCDEF0,'h600dbabe_baddade0,'h0}};

parameter llki_s DES3_LLKI_STRUCT   ='{core_id    : `DES3_ID,
				       attributes : 0,
				       key_size   : 4,
				       base       : 'h7000FF00,
				       key        : '{'h11111111_22222222,'h33333333_44444444,'h12345678_9ABCDEF0,'h600dbabe_baddade0}};

parameter llki_s SHA256_LLKI_STRUCT ='{core_id    : `SHA256_ID,
				       attributes : 0,
				       key_size   : 1,
				       base       : 'h7000FF00,
				       key        : '{'h600dbabe_baddade0,'h0,'h0,'h0}};

parameter llki_s GPS_LLKI_STRUCT    ='{core_id    : `GPS_ID,
				       attributes : 0,
				       key_size   : 2,
				       base       : 'h7000FF00,
				       key        : '{'h99999999_aaaaaaaa,'hbbbbbbbb_bbbbbbbb,'h0,'h0}};

parameter llki_s DFT_LLKI_STRUCT    ='{core_id    : `DFT_ID,
				       attributes : 0,
				       key_size   : 3,
				       base       : 'h7000FF00,
				       key        : '{'hcccccccc_dddddddd,'heeeeeeee_ffffffff,'h11223344_55667788,'h0}};

parameter llki_s IDFT_LLKI_STRUCT   ='{core_id    : `IDFT_ID,
				       attributes : 0,
				       key_size   : 4,
				       base       : 'h7000FF00,
				       key        : '{'h99aabbcc_ddeeff00,'h11112222_33334444,'h55556666_77778888,'h9999aaaa_bbbbcccc}};

parameter llki_s FIR_LLKI_STRUCT    ='{core_id    : `FIR_ID,
				       attributes : 0,
				       key_size   : 2,
				       base       : 'h7000FF00,
				       key        : '{'hddddeeee_ffff0000,'h12345678_9ABCDEF0,'h600dbabe_baddade0,'h0}};

parameter llki_s IIR_LLKI_STRUCT    ='{core_id    : `IIR_ID,
				       attributes : 0,
				       key_size   : 2,
				       base       : 'h7000FF00,
				       key        : '{'h33333333_44444444,'h12345678_9ABCDEF0,'h600dbabe_baddade0,'h0}};

parameter llki_s RSA_LLKI_STRUCT    ='{core_id    : `RSA_ID,
				       attributes : 0,
				       key_size   : 3,
				       base       : 'h7000FF00,
				       key        : '{'h5555aaaa_44444444,'h12345678_9ABCDEF0,'h600dbabe_baddade0,'h0}};


//
// Pre-init the struct for all cores
//
initial begin
   //
   // All cores key structures
   //
   llki_rom[`AES_ID]    = AES_LLKI_STRUCT;
   llki_rom[`MD5_ID]    = MD5_LLKI_STRUCT;
   llki_rom[`DES3_ID]   = DES3_LLKI_STRUCT;   
   llki_rom[`SHA256_ID] = SHA256_LLKI_STRUCT;
   llki_rom[`GPS_ID]    = GPS_LLKI_STRUCT;
   llki_rom[`DFT_ID]    = DFT_LLKI_STRUCT;         
   llki_rom[`IDFT_ID]   = IDFT_LLKI_STRUCT;         
   llki_rom[`FIR_ID]    = FIR_LLKI_STRUCT;         
   llki_rom[`IIR_ID]    = IIR_LLKI_STRUCT;         
   llki_rom[`RSA_ID]    = RSA_LLKI_STRUCT;            
   //
   // more cores....
   //

   //
   // convert to SRoT's ROM memory
   //
end
