//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:       llki_pkg.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     LLKI related parameters and such.
// Notes:           Addresses must reside "within" the corresponding
//                  range as specified in cep_addresses.scala.
//              
//                  Example: For the Surrogate Root of Trust (SRoT)
//                  cep_addresses.scala specifies
//                    val srot_base_addr      = 0x70100000L
//                    val srot_base_depth     = 0x00007fffL
//                  As a result, the SROT_XXXX addresses must
//                  reside in this ranges (due to hierarchical decode)
//
//                  The address and bitmappings here should match
//                  those in CEP.h (to allow for SW access)
//
//************************************************************************

package llki_pkg;

    // SROT Control / Status Register
    localparam SROT_CTRLSTS_ADDR                = 32'h7020_0000;
    localparam SROT_CTRLSTS_MODEBIT_0           = 0;    // If either mode bit is set, TileLink access to the Key and Key Index RAMs are disabled
    localparam SROT_CTRLSTS_MODEBIT_1           = 1;    // These bits are SET ONLY    
    localparam SROT_CTRLSTS_RESP_WAITING        = 2;    // Indicates that a message/response is available in the SROT_RESPONSE_ADDR register
    localparam SROT_LLKIC2_SENDRECV_ADDR        = 32'h7020_0008;
    localparam SROT_LLKIC2_SCRATCHPAD0_ADDR     = 32'h7020_0010;
    localparam SROT_LLKIC2_SCRATCHPAD1_ADDR     = 32'h7020_0018;
    // See KeyIndexRAM explanation below
    localparam SROT_KEYINDEXRAM_ADDR            = 32'h7020_0100;
    localparam SROT_KEYINDEXRAM_SIZE            = 32'h0000_0020;
    // Holds the LLKI keys as referenced by the those words in the KeyIndex RAM
    // Note: This cannot be less than SROT_KEYINDEXRAM_ADDR + (SROT_KEYINDEXRAM_SIZE * 8)!!!
    localparam SROT_KEYRAM_ADDR                 = 32'h7020_0200;
    localparam SROT_KEYRAM_SIZE                 = 32'h0000_0100;

    // All LLKI Protocol Processing blocks will use the same SystemVerilog code, and thus
    // will have their decode addresses uniquified through module parameters.  These will be
    // passed down from the Chisel level of the design (and ths the addresses are contained
    // in cep_addresses.scala).  Common parameters (such as the LLKI_CTRLSTS_RESP_WAITING
    // bit mapped will be contained here)

    localparam  LLKIKL_CTRLSTS_RESP_WAITING     = 0;    // Indicates that a message/response is available 
                                                        // in the LLKI PP control/status register
    localparam  LLKIKL_CTRLSTS_READY_FOR_KEY    = 1;    // The LLKI-PP provides minimal buffering for
                                                        // messages (and key words) received from
                                                        // Thus,  the SRoT can poll the ready for key
                                                        // bit in the LLKI-PP Control/Status register
                                                        // to determine that the LLKI-PP (and TSS) is
                                                        // ready to receive the next key word


    // The Key Index RAM holds single word identifiers that contain
    // metadata for all the keys stored in the Key RAM
    //
    // 63 62                 40 39       32 31            16 15             0
    // +-+---------------------+-----------+----------------+----------------+ 
    // |V|      Reserved       | Core Indx |  High Pointer  |  Low Pointer   |
    // +-+---------------------+-----------+----------------+----------------+ 
    //
    // Field descriptions:
    //  - Valid             : Indicates a valid key index and valid key material in the
    //                        range identified to by the high and low pointers
    //  - Core Indx         : Pointer to an entry in LLKI_CORE_INDEX_ROM that determines
    //                        which LLKI-enabled core is the target of the current operation
    //  - High Pointer      : Upper Key RAM addr
    //  - Low Pointer       : Lower Key RAM addr
    //
    // For a valid key index, the key data words should be located in the
    // Key RAM as described here:  Higher Pointer >= Key#N >= Low Pointer
    //
    // This error condition as well as other are checked i n the ST_RETRIEVE_KEY_INDEX state 
    // of the SRoT state machine (in srot_wrapper.sv)
    //

    // Generate a list of all the valid LLKI Key Load interface
    // address pairs: One for send/receive, one for control/status
    // Ensure these match the equivalent constants in cep_addresses.scala
    localparam AES_LLKIKL_CTRLSTS_ADDR      = 32'h7000_8000;
    localparam AES_LLKIKL_SENDRECV_ADDR     = 32'h7000_8008;
    localparam MD5_LLKIKL_CTRLSTS_ADDR      = 32'h7001_8000;
    localparam MD5_LLKIKL_SENDRECV_ADDR     = 32'h7001_8008;
    localparam SHA256_LLKIKL_CTRLSTS_ADDR   = 32'h7002_8000;
    localparam SHA256_LLKIKL_SENDRECV_ADDR  = 32'h7002_8008;
    localparam RSA_LLKIKL_CTRLSTS_ADDR      = 32'h7003_8000;
    localparam RSA_LLKIKL_SENDRECV_ADDR     = 32'h7003_8008;
    localparam DES3_LLKIKL_CTRLSTS_ADDR     = 32'h7004_8000;
    localparam DES3_LLKIKL_SENDRECV_ADDR    = 32'h7004_8008;
    localparam DFT_LLKIKL_CTRLSTS_ADDR      = 32'h7005_8000;
    localparam DFT_LLKIKL_SENDRECV_ADDR     = 32'h7005_8008;
    localparam IDFT_LLKIKL_CTRLSTS_ADDR     = 32'h7006_8000;
    localparam IDFT_LLKIKL_SENDRECV_ADDR    = 32'h7006_8008;
    localparam FIR_LLKIKL_CTRLSTS_ADDR      = 32'h7007_8000;
    localparam FIR_LLKIKL_SENDRECV_ADDR     = 32'h7007_8008;
    localparam IIR_LLKIKL_CTRLSTS_ADDR      = 32'h7008_8000;
    localparam IIR_LLKIKL_SENDRECV_ADDR     = 32'h7008_8008;
    localparam GPS_LLKIKL_CTRLSTS_ADDR      = 32'h7009_8000;
    localparam GPS_LLKIKL_SENDRECV_ADDR     = 32'h7009_8008;

    localparam LLKI_CTRLSTS_INDEX           = 0;
    localparam LLKI_SENDRECV_INDEX          = 1;

    // The following array provides constants to the Surrogate Root of Trust for indexing the LLKI
    localparam [31:0] LLKI_CORE_INDEX_ARRAY [0:9][0:1] = '{
        {AES_LLKIKL_CTRLSTS_ADDR, AES_LLKIKL_SENDRECV_ADDR},            // Core Index 0 - AES
        {MD5_LLKIKL_CTRLSTS_ADDR, MD5_LLKIKL_SENDRECV_ADDR},            // Core Index 1 - MD5
        {SHA256_LLKIKL_CTRLSTS_ADDR, SHA256_LLKIKL_SENDRECV_ADDR},      // Core Index 2 - SHA256
        {RSA_LLKIKL_CTRLSTS_ADDR, RSA_LLKIKL_SENDRECV_ADDR},            // Core Index 3 - RSA
        {DES3_LLKIKL_CTRLSTS_ADDR, DES3_LLKIKL_SENDRECV_ADDR},          // Core Index 4 - DES3
        {DFT_LLKIKL_CTRLSTS_ADDR, DFT_LLKIKL_SENDRECV_ADDR},            // Core Index 5 - DFT
        {IDFT_LLKIKL_CTRLSTS_ADDR, IDFT_LLKIKL_SENDRECV_ADDR},          // Core Index 6 - IDFT
        {FIR_LLKIKL_CTRLSTS_ADDR, FIR_LLKIKL_SENDRECV_ADDR},            // Core Index 7 - FIR
        {IIR_LLKIKL_CTRLSTS_ADDR, IIR_LLKIKL_SENDRECV_ADDR},            // Core Index 8 - IIR
        {GPS_LLKIKL_CTRLSTS_ADDR, GPS_LLKIKL_SENDRECV_ADDR}             // Core Index 9 - GPS
    };

 
    //
    // The LLKI is comprised of the following three interface types:
    //      LLKI-C2         : Using Tilelink, it supports the following message types:
    //                          C2LoadKeyReq        : Load a key into the specified Core
    //                          C2ClearKeyReq       : Clear the key from the specified Core
    //                          C2KeyStatusReq      : Request key status from the specified Core
    //                          C2LoadKeyAck        : Load key acknowledgement
    //                          C2ClearKeyAck       : Clear key acknowledgement
    //                          C2KeyStatusResp     : Response to the Key Status Requests
    //      LLKI-KL         : Using Tilelink, it supports the following message types:
    //                          KLLoadKeyReq        : Load a key into the specified Core
    //                          KLClearKeyReq       : Clear the key from the specified Core
    //                          KLKeyStatusReq      : Request key status from the specified Core
    //                          KLLoadKeyAck        : Load key acknowledgement
    //                          KLClearKeyAck       : Clear key acknowledgement
    //                          KLKeyStatusResp     : Response to the Key Status Requests    
    //      LLKI-Discrete   : Discrete signals for connecting the LLKI to Technique Specific Shims
    //                          llkid_key_data      : Key Data Bits, greater key widths loaded over multiple clock cycles
    //                          llkid_key_valid     : Indicates that the key data is valid
    //                          llkid_key_ready     : Indicates that the TSS is ready to receive key data
    //                          llkid_key_complete  : Indicates that the TSS has received all the required key bits
    //                          llkid_clear_key     : Assert to instruct the TSS to clear its internal key-state, thus "locking" the core
    //                          llkid_clear_key_ack : When asserted by the TSS, it indicates that the key has been cleared
    //
    //
    // LLKI C2 RISCV -> SRoT Message Format
    //
    //                                                                         Word#
    // 63                               32 31   24 23     16 15     8 7      0  
    // +----------------------------------+-------+---------+--------+--------+ 
    // |             Reserved             |Key Idx| MSG LEN | STATUS | MSG ID |  1
    // +----------------------------------+-------+---------+--------+--------+ 

    //
    // MSG ID : Only the following message IDs are valid on the LLKIC2 request interface:
    //  - LLKI_MID_C2LOADKEYREQ
    //  - LLKI_MID_C2CLEARKEYREQ
    //  - LLKI_MID_C2KEYSTATUSREQ
    //
    // STATUS :
    //  - Unused for C2 RISCV -> SRoT Messages
    //
    // MSG LEN :
    //  - Should be 1 for all C2 RISCV -> SRoT Messages
    //
    // Key Idx : Specifies the index of the key to be referenced by the request.  This
    //  becomes a direct index into the Key Index RAM, which contains all the key
    //  specific metadata.  See Key Index RAM format above for more information.
    // LLKI Message ID constants
    localparam  LLKI_MID_C2LOADKEYREQ           = 8'h00;
    localparam  LLKI_MID_C2CLEARKEYREQ          = 8'h01;
    localparam  LLKI_MID_C2KEYSTATUSREQ         = 8'h02;
    localparam  LLKI_MID_C2LOADKEYACK           = 8'h03;
    localparam  LLKI_MID_C2CLEARKEYACK          = 8'h04;
    localparam  LLKI_MID_C2KEYSTATUSRESP        = 8'h05;
    localparam  LLKI_MID_C2ERRORRESP            = 8'h06;


    // LLKI KL SRoT -> LLKI PP Message Format
    // NOTE: Each word transferred is a SEPERATE Tilelink transaction
    //
    //                                                                         Word#
    // 63                                       24 23     16 15     8 7      0  
    // +------------------------------------------+---------+--------+--------+ 
    // |                 Reserved                 | MSG LEN | STATUS | MSG ID |  1
    // +------------------------------------------+---------+--------+--------+ 
    //                                      ...
    // +----------------------------------------------------------------------+
    // |             Key Word #1 (LLKI_MID_KLLOADKEYREQ message only)         |  2
    // +----------------------------------------------------------------------+
    //                                      ...
    // +----------------------------------------------------------------------+
    // |             Key Word #N (LLKI_MID_KLLOADKEYREQ message only)         |  N + 1
    // +----------------------------------------------------------------------+
    //
    //
    // MSG ID : Only the following message IDs are valid on the LLKIC2 request interface:
    //  - LLKI_MID_C2LOADKEYREQ
    //  - LLKI_MID_C2CLEARKEYREQ
    //  - LLKI_MID_C2KEYSTATUSREQ
    localparam  LLKI_MID_KLLOADKEYREQ           = 8'h07;
    localparam  LLKI_MID_KLCLEARKEYREQ          = 8'h08;
    localparam  LLKI_MID_KLKEYSTATUSREQ         = 8'h09;
    localparam  LLKI_MID_KLLOADKEYACK           = 8'h0A;
    localparam  LLKI_MID_KLCLEARKEYACK          = 8'h0B;
    localparam  LLKI_MID_KLKEYSTATUSRESP        = 8'h0C;
    localparam  LLKI_MID_KLERRORRESP            = 8'h06;

    // LLKI Status Constants
    localparam  LLKI_STATUS_GOOD                = 8'h00;    // Indication by the LLKI PP that a request has been successfully
                                                            // processed
    localparam  LLKI_STATUS_KEY_PRESENT         = 8'h01;    // In response to a LLKI_MID_KL/C2KEYSTATUSREQ, indicates that the
                                                            // target core has a key present
    localparam  LLKI_STATUS_KEY_NOT_PRESENT     = 9'h02;    // In response to a LLKI_MID_KL/C2KEYSTATUSREQ, indicates that the
                                                            // target core does not have a key present

    localparam  LLKI_STATUS_BAD_MSG_ID          = 8'h20;    // A message ID received via the LLKI-C2 interface is not valid
    localparam  LLKI_STATUS_BAD_MSG_LEN         = 8'h21;    // All messages received via the LLKI-C2 interface have an expected
                                                            // message length of 1
    localparam  LLKI_STATUS_KEY_INDEX_EXCEED    = 8'h22;    // Specified key index in the LLKI-KL message exceeds the size of the Key Index
                                                            // RAM and thus cannot be valid
    localparam  LLKI_STATUS_KEY_INDEX_INVALID   = 8'h23;    // The key index referenced by the LLKI-KL message does not have a valid bit set and
                                                            // thus is invalid
    localparam  LLKI_STATUS_BAD_POINTER_PAIR    = 8'h24;    // A Key Index entry that was previously loaded in which either of the pointers were in 
                                                            // excess of the Key RAM size OR the low pointer > high pointer
    localparam  LLKI_STATUS_BAD_CORE_INDEX      = 8'h25;    // The Core Index, which is used as a pointer to of the available cores, exceeeds the
                                                            // the highest entry in LLKI_CORE_INDEX_ARRAY
    localparam  LLKI_STATUS_KL_REQ_BAD_MSG_ID   = 8'h26;    // The LLKI PP received a bad message ID from the SRoT
    localparam  LLKI_STATUS_KL_REQ_BAD_MSG_LEN  = 8'h27;    // The LLKI PP received a message with a bad length from the SRoT
    localparam  LLKI_STATUS_KL_RESP_BAD_MSG_ID  = 8'h28;    // The SRoT has received a response from the selected LLKI PP with a bad message ID
    localparam  LLKI_STATUS_KL_TILELINK_ERROR   = 8'h29;    // A tilelink error was detected on the SRoT -> LLKI PP (LLKI-KL) interface
    localparam  LLKI_STATUS_KL_LOSS_OF_SYNC     = 8'h30;    // There has been a synchronization error between the selected LLKI PP and TSS
    localparam  LLKI_STATUS_KL_BAD_KEY_LEN      = 8'h31;    // A key has been loaded into the specificed LLKI-PP whose length does not match
                                                            // the expected length
    localparam  LLKI_STATUS_KL_KEY_OVERWRITE    = 8'h32;    // A key load has been attempted on a core that already has one
    localparam  LLKI_STATUS_UNKNOWN_ERROR       = 8'hFF;    // An unknown LLKI error has been detected

    // Value is used to initalize the SRoT STM wait state
    // counter.  This mitigates spamming the SRoT host
    // interface while waiting for a response from the
    // selected LLKI-KP
    localparam  SROT_WAIT_STATE_COUNTER_INIT    = 8'h0A;

    // Enumerated type for Surrogate Root of Trust state machine.  State
    // decscriptions can be found in srot_wrapper.sv
    typedef enum {
        ST_SROT_IDLE,
        ST_SROT_MESSAGE_CHECK,
        ST_SROT_RETRIEVE_KEY_INDEX_WAIT_STATE,
        ST_SROT_RETRIEVE_KEY_INDEX,
        
        ST_SROT_KL_REQ_HEADER,
        ST_SROT_KL_REQ_ISSUE,
        ST_SROT_KL_REQ_WAIT_FOR_ACK,

        ST_SROT_KL_READ_READY_STATUS,
        ST_SROT_KL_CHECK_READY_STATUS,
        ST_SROT_KL_READY_WAIT_STATE,
        ST_SROT_KL_LOAD_KEY_WORD,
        ST_SROT_KL_LOAD_KEY_WORD_WAIT_FOR_ACK,

        ST_SROT_KL_READ_RESP_STATUS,
        ST_SROT_KL_CHECK_RESP_STATUS,
        ST_SROT_KL_RESP_WAIT_STATE,

        ST_SROT_KL_RESP_READ,

        ST_SROT_C2_RESPONSE
    } SROT_STATE_TYPE;  

    // Enumerate type for the LLKI Protocol Processing block state machine
    typedef enum {
        ST_LLKIPP_IDLE,
        ST_LLKIPP_MESSAGE_CHECK,
        ST_LLKIPP_LOAD_KEY_WORDS,
        ST_LLKIPP_CLEAR_KEY,
        ST_LLKIPP_RESPONSE
    } LLKIPP_STATE_TYPE;

    // The following parameters are used by the AES instance of the Mock Technique Specific Shim (TSS)
    localparam AES_MOCK_TSS_NUM_KEY_WORDS       = 2;
    localparam [63:0] AES_MOCK_TSS_KEY_WORDS [0:AES_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210
    };

    // The following parameters are used by the DES3 instance of the Mock Technique Specific Shim (TSS)
    localparam DES3_MOCK_TSS_NUM_KEY_WORDS      = 1;
    localparam [63:0] DES3_MOCK_TSS_KEY_WORDS [0:DES3_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the SHA256 instance of the Mock Technique Specific Shim (TSS)
    localparam SHA256_MOCK_TSS_NUM_KEY_WORDS    = 8;
    localparam [63:0] SHA256_MOCK_TSS_KEY_WORDS [0:SHA256_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210
    };

    // The following parameters are used by the MD5 instance of the Mock Technique Specific Shim (TSS)
    localparam MD5_MOCK_TSS_NUM_KEY_WORDS    = 8;
    localparam [63:0] MD5_MOCK_TSS_KEY_WORDS [0:MD5_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210
    };

    // The following parameters are used by the RSA (modexp_core) instance of the Mock Technique Specific Shim (TSS)
    localparam RSA_MOCK_TSS_NUM_KEY_WORDS    = 1;
    localparam [63:0] RSA_MOCK_TSS_KEY_WORDS [0:RSA_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the IIR instance of the Mock Technique Specific Shim (TSS)
    localparam IIR_MOCK_TSS_NUM_KEY_WORDS    = 1;
    localparam [63:0] IIR_MOCK_TSS_KEY_WORDS [0:IIR_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the FIR instance of the Mock Technique Specific Shim (TSS)
    localparam FIR_MOCK_TSS_NUM_KEY_WORDS    = 1;
    localparam [63:0] FIR_MOCK_TSS_KEY_WORDS [0:FIR_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the DFT instance of the Mock Technique Specific Shim (TSS)
    localparam DFT_MOCK_TSS_NUM_KEY_WORDS    = 1;
    localparam [63:0] DFT_MOCK_TSS_KEY_WORDS [0:DFT_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the IDFT instance of the Mock Technique Specific Shim (TSS)
    localparam IDFT_MOCK_TSS_NUM_KEY_WORDS   = 1;
    localparam [63:0] IDFT_MOCK_TSS_KEY_WORDS [0:IDFT_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF
    };

    // The following parameters are used by the GPS instance of the Mock Technique Specific Shim (TSS)
    localparam GPS_MOCK_TSS_NUM_KEY_WORDS    = 5;
    localparam [63:0] GPS_MOCK_TSS_KEY_WORDS [0:GPS_MOCK_TSS_NUM_KEY_WORDS - 1] = '{
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF,
        64'hFEDCBA9876543210,
        64'h0123456789ABCDEF
    };

    // The Mock TSS introduces artificial wait states to demonstrate
    // a delay when loading or clearing keys
    typedef enum {
        ST_MOCKTSS_IDLE,
        ST_MOCKTSS_WAIT_STATE0,
        ST_MOCKTSS_WAIT_FOR_NEXT_KEY_WORD,
        ST_MOCKTSS_CLEAR_KEY,
        ST_MOCKTSS_WAIT_STATE1
    } MOCKTSS_STATE_TYPE;

    localparam  MOCKTSS_WAIT_STATE_COUNTER_INIT     = 8'h0A;

endpackage
