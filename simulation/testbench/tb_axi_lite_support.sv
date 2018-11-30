//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_support.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Support parameters and such for the CEP testbench(es)
//l
// Derived from : https://github.com/pulp-platform/axi.git
// 
// Licensing info can be found in the root directory of the CEP in licenseLog.txt
//

// Used for the various levels of debug (higher levels define lower levels)
`ifdef DEBUG5
    `define DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG2
    `define DEBUG1
`endif

// Import some of the axi related packages
import axi_pkg::*;
import axi_test::*;

// Some extensions to the axi_test packaage
package axi_test_extended;

    // General Constants
    parameter BITS_PER_BYTE     = 8;
    parameter BYTES_PER_WORD    = 4;

    // Testbench clock period constant
    parameter tCK               = 10ns;

    // Address width and data width constants (for Wishbone and AXI4-lite buses)
    parameter AW                = 32;
    parameter DW                = 32;

    // Address base constants for CEP Cores
    parameter RAM_BASE          = 32'h0000_0000;
    parameter UART_BASE         = 32'h9000_0000;
    parameter AES_BASE          = 32'h9300_0000;
    parameter MD5_BASE          = 32'h9400_0000;
    parameter SHA256_BASE       = 32'h9500_0000;
    parameter RSA_ADDR_BASE     = 32'h9600_0000;
    parameter DES3_BASE         = 32'h9700_0000;
    parameter DFT_BASE          = 32'h9800_0000;
    parameter IDFT_BASE         = 32'h9900_0000;
    parameter FIR_BASE          = 32'h9a00_0000;
    parameter IIR_BASE          = 32'h9b00_0000;
    parameter GPS_BASE          = 32'h9C00_0000;

    // Constants copied from AES.h
    parameter AES_KEY_BITS = 192;
    parameter AES_BLOCK_BITS = 128;
    parameter AES_START_BYTES = BYTES_PER_WORD;
    parameter AES_DONE_BYTES = BYTES_PER_WORD;

    parameter AES_KEY_BYTES = AES_KEY_BITS / BITS_PER_BYTE;
    parameter AES_KEY_WORDS = AES_KEY_BYTES / BYTES_PER_WORD;
    parameter AES_BLOCK_BYTES = AES_BLOCK_BITS / BITS_PER_BYTE;
    parameter AES_BLOCK_WORDS = AES_BLOCK_BYTES / BYTES_PER_WORD;

    parameter AES_START = AES_BASE;
    parameter AES_PT_BASE = AES_START + AES_START_BYTES;
    parameter AES_KEY_BASE = AES_PT_BASE + AES_BLOCK_BYTES;
    parameter AES_DONE = AES_KEY_BASE + AES_KEY_BYTES; 
    parameter AES_CT_BASE = AES_DONE + AES_DONE_BYTES;

    // Constants copied from DES.h
    parameter DES3_KEY_BITS = 64*3;
    parameter DES3_BLOCK_BITS = 64;
    parameter DES3_START_BYTES = BYTES_PER_WORD;
    parameter DES3_DECRYPT_BYTES = BYTES_PER_WORD;
    parameter DES3_DONE_BYTES = BYTES_PER_WORD;

    // Automatically generated constants
    parameter DES3_KEY_BYTES = DES3_KEY_BITS / BITS_PER_BYTE;
    parameter DES3_KEY_WORDS = DES3_KEY_BYTES / BYTES_PER_WORD;
    parameter DES3_BLOCK_BYTES = DES3_BLOCK_BITS / BITS_PER_BYTE;
    parameter DES3_BLOCK_WORDS = DES3_BLOCK_BYTES / BYTES_PER_WORD;

    // Offset of DES3 data and control registers in device memory map
    parameter DES3_START = DES3_BASE;
    parameter DES3_DECRYPT = DES3_START + DES3_START_BYTES;
    parameter DES3_IN_BASE = DES3_DECRYPT + DES3_DECRYPT_BYTES;
    parameter DES3_KEY_BASE = DES3_IN_BASE + DES3_BLOCK_BYTES;
    parameter DES3_DONE = DES3_KEY_BASE + DES3_KEY_BYTES;
    parameter DES3_CT_BASE = DES3_DONE + DES3_DONE_BYTES;

    // Constants from MD5.h
    parameter MD5_HASH_BITS = 128;  
    parameter MD5_MESSAGE_BITS = 512;
    parameter MD5_READY_BYTES = 4;
    parameter MD5_HASH_DONE_BYTES = 4;

    // Automatically generated constants
    parameter MD5_HASH_BYTES = MD5_HASH_BITS / BITS_PER_BYTE;
    parameter MD5_HASH_WORDS = MD5_HASH_BYTES / BYTES_PER_WORD;
    parameter MD5_MESSAGE_BYTES = MD5_MESSAGE_BITS / BITS_PER_BYTE;
    parameter MD5_MESSAGE_WORDS = MD5_MESSAGE_BYTES / BYTES_PER_WORD;

    // Offset of MD5 data and control registers in device memory map
    parameter MD5_READY = MD5_BASE;
    parameter MD5_MSG_BASE = MD5_READY + MD5_READY_BYTES;
    parameter MD5_HASH_DONE = MD5_MSG_BASE + MD5_MESSAGE_BYTES;
    parameter MD5_HASH_BASE = MD5_HASH_DONE + MD5_HASH_DONE_BYTES;
    parameter MD5_RST = MD5_HASH_BASE + MD5_HASH_BYTES;

    // Constants from SHA256.h
    parameter SHA256_HASH_BITS = 256;
    parameter SHA256_MESSAGE_BITS = 512;
    parameter SHA256_READY_BYTES = BYTES_PER_WORD;
    parameter SHA256_HASH_DONE_BYTES = BYTES_PER_WORD;

    // Automatically generated constants
    parameter SHA256_HASH_BYTES = SHA256_HASH_BITS / BITS_PER_BYTE;
    parameter SHA256_HASH_WORDS = SHA256_HASH_BYTES / BYTES_PER_WORD;
    parameter SHA256_MESSAGE_BYTES = SHA256_MESSAGE_BITS / BITS_PER_BYTE;
    parameter SHA256_MESSAGE_WORDS = SHA256_MESSAGE_BYTES / BYTES_PER_WORD;

    // Offset of SHA256 data and control registers in device memory map
    parameter SHA256_READY = SHA256_BASE;
    parameter SHA256_MSG_BASE = SHA256_READY + SHA256_READY_BYTES;
    parameter SHA256_HASH_DONE = SHA256_MSG_BASE + SHA256_MESSAGE_BYTES;
    parameter SHA256_HASH_BASE = SHA256_HASH_DONE + SHA256_HASH_DONE_BYTES;
    parameter SHA256_NEXT_INIT = SHA256_BASE;

    // Constants from the RSA core's define.h
    parameter RSA_ADDR_NAME1            = RSA_ADDR_BASE | (8'h01<<2);
    parameter RSA_ADDR_VERSION          = RSA_ADDR_BASE | (8'h02<<2);
    parameter RSA_ADDR_CTRL             = RSA_ADDR_BASE | (8'h08<<2);
    parameter RSA_ADDR_STATUS           = RSA_ADDR_BASE | (8'h09<<2);   
    parameter RSA_ADDR_CYCLES_HIGH      = RSA_ADDR_BASE | (8'h10<<2);
    parameter RSA_ADDR_CYCLES_LOW       = RSA_ADDR_BASE | (8'h11<<2);
    parameter RSA_ADDR_MODULUS_LENGTH   = RSA_ADDR_BASE | (8'h20<<2);
    parameter RSA_ADDR_EXPONENT_LENGTH  = RSA_ADDR_BASE | (8'h21<<2);
    parameter RSA_ADDR_MODULUS_PTR_RST  = RSA_ADDR_BASE | (8'h30<<2);
    parameter RSA_ADDR_MODULUS_DATA     = RSA_ADDR_BASE | (8'h31<<2);
    parameter RSA_ADDR_EXPONENT_PTR_RST = RSA_ADDR_BASE | (8'h40<<2);
    parameter RSA_ADDR_EXPONENT_DATA    = RSA_ADDR_BASE | (8'h41<<2);
    parameter RSA_ADDR_MESSAGE_PTR_RST  = RSA_ADDR_BASE | (8'h50<<2);
    parameter RSA_ADDR_MESSAGE_DATA     = RSA_ADDR_BASE | (8'h51<<2);
    parameter RSA_ADDR_RESULT_PTR_RST   = RSA_ADDR_BASE | (8'h60<<2);
    parameter RSA_ADDR_RESULT_DATA      = RSA_ADDR_BASE | (8'h61<<2);

    // Constants from GPS.h
    parameter GPS_GEN_NEXT_BYTES = BYTES_PER_WORD;
    parameter GPS_CA_BYTES = BYTES_PER_WORD;
    parameter GPS_P_BYTES = 128 / BITS_PER_BYTE;
    parameter GPS_L_BYTES = 128 / BITS_PER_BYTE;
    parameter GPS_GEN_NEXT = GPS_BASE;
    parameter GPS_GEN_DONE = GPS_BASE;
    parameter GPS_CA_BASE = GPS_GEN_NEXT + GPS_GEN_NEXT_BYTES;
    parameter GPS_P_BASE = GPS_CA_BASE + GPS_CA_BYTES;
    parameter GPS_L_BASE = GPS_P_BASE + GPS_P_BYTES;

    // Constants from DFT.h
    parameter DFT_BLOCK_BITS    = 32;
    parameter DFT_BLOCK_BYTES   = DFT_BLOCK_BITS / BITS_PER_BYTE;
    parameter DFT_BLOCK_WORDS   = DFT_BLOCK_BYTES / BYTES_PER_WORD; 
    parameter DFT_START         = DFT_BASE;
    parameter DFT_IN_WRITE      = DFT_START    + BYTES_PER_WORD;
    parameter DFT_IN_ADDR       = DFT_IN_WRITE + BYTES_PER_WORD;
    parameter DFT_IN_DATA       = DFT_IN_ADDR  + DFT_BLOCK_BYTES;
    parameter DFT_OUT_ADDR      = DFT_IN_DATA  + DFT_BLOCK_BYTES + DFT_BLOCK_BYTES;
    parameter DFT_OUT_DATA      = DFT_OUT_ADDR + DFT_BLOCK_BYTES;
    parameter DFT_DONE          = DFT_OUT_DATA + DFT_BLOCK_BYTES + DFT_BLOCK_BYTES;

    // Constants from IDFT.h
    parameter IDFT_BLOCK_BITS   = 32;
    parameter IDFT_BLOCK_BYTES  = IDFT_BLOCK_BITS / BITS_PER_BYTE;
    parameter IDFT_BLOCK_WORDS  = IDFT_BLOCK_BYTES / BYTES_PER_WORD; 
    parameter IDFT_START        = IDFT_BASE;
    parameter IDFT_IN_WRITE     = IDFT_START    + BYTES_PER_WORD;
    parameter IDFT_IN_ADDR      = IDFT_IN_WRITE + BYTES_PER_WORD;
    parameter IDFT_IN_DATA      = IDFT_IN_ADDR  + IDFT_BLOCK_BYTES;
    parameter IDFT_OUT_ADDR     = IDFT_IN_DATA  + IDFT_BLOCK_BYTES + IDFT_BLOCK_BYTES;
    parameter IDFT_OUT_DATA     = IDFT_OUT_ADDR + IDFT_BLOCK_BYTES;
    parameter IDFT_DONE         = IDFT_OUT_DATA + IDFT_BLOCK_BYTES + IDFT_BLOCK_BYTES;

    // Constants from IIR.h
    parameter IIR_BLOCK_BITS    = 32;
    parameter IIR_BLOCK_BYTES   = IIR_BLOCK_BITS / BITS_PER_BYTE;
    parameter IIR_BLOCK_WORDS   = IIR_BLOCK_BYTES / BYTES_PER_WORD;
    parameter IIR_START         = IIR_BASE;
    parameter IIR_IN_WRITE      = IIR_START    + BYTES_PER_WORD;
    parameter IIR_IN_ADDR       = IIR_IN_WRITE + BYTES_PER_WORD;
    parameter IIR_IN_DATA       = IIR_IN_ADDR  + IIR_BLOCK_BYTES;
    parameter IIR_OUT_ADDR      = IIR_IN_DATA  + IIR_BLOCK_BYTES;
    parameter IIR_OUT_DATA      = IIR_OUT_ADDR + IIR_BLOCK_BYTES;
    parameter IIR_DONE          = IIR_OUT_DATA + IIR_BLOCK_BYTES;

    // Constants from FIR.h
    parameter FIR_BLOCK_BITS    = 32;
    parameter FIR_BLOCK_BYTES   = FIR_BLOCK_BITS / BITS_PER_BYTE;
    parameter FIR_BLOCK_WORDS   = FIR_BLOCK_BYTES / BYTES_PER_WORD;
    parameter FIR_START         = FIR_BASE;
    parameter FIR_IN_WRITE      = FIR_START    + BYTES_PER_WORD;
    parameter FIR_IN_ADDR       = FIR_IN_WRITE + BYTES_PER_WORD;
    parameter FIR_IN_DATA       = FIR_IN_ADDR  + FIR_BLOCK_BYTES;
    parameter FIR_OUT_ADDR      = FIR_IN_DATA  + FIR_BLOCK_BYTES;
    parameter FIR_OUT_DATA      = FIR_OUT_ADDR + FIR_BLOCK_BYTES;
    parameter FIR_DONE          = FIR_OUT_DATA + FIR_BLOCK_BYTES;
    
    // The following class extends the existing axi_lite_driver to provide some additional utility functions without
    // altering the base class (which is a submodule)
    class axi_lite_driver_plus #(
        parameter int  AW       ,
        parameter int  DW       ,
        parameter time TA = 0ns , // stimuli application time
        parameter time TT = 0ns   // stimuli test time
    ) extends axi_test::axi_lite_driver #(.AW(AW), .DW(DW));

        virtual AXI_LITE #(
            .AXI_ADDR_WIDTH(AW),
            .AXI_DATA_WIDTH(DW)
        ) axi;

        function new(
            virtual AXI_LITE #(
                .AXI_ADDR_WIDTH(AW),
                .AXI_DATA_WIDTH(DW)
            ) axi
        );
            super.new(axi);
        endfunction

        // Compositve function for writing to the AXI4-lite bus
        task axi_lite_slave_write(
            input [AW - 1:0]        addr, 
            input [DW - 1:0]        data, 
            input [DW/8 - 1:0]      strb,
            output axi_pkg::resp_t  resp
        );

            fork
                send_aw    (addr);
                send_w     (data, strb);
            join
            recv_b         (resp);

            `ifdef DEBUG2
                $display("DEBUG2: axi_lite_slave_write: (addr = %8h, data = %8h", addr, data);
            `endif

        endtask // end task axi_lite_slave_write

        // Compositve function for reading from the AXI4-lite bus
        task axi_lite_slave_read(
            input [AW - 1:0]        addr, 
            output [DW - 1:0]       data, 
            output axi_pkg::resp_t  resp
        );

            send_ar     (.addr(addr));
            recv_r      (.data(data),.resp(resp));

            `ifdef DEBUG2
                $display("DEBUG2: axi_lite_slave_read: (addr = %8h, data = %8h", addr, data);
            `endif

        endtask : axi_lite_slave_read

        // The following function will take a string as input and load it into the MD5 core in
        // 512-bit chunks.  If an odd number of chunks is detected, then the message will be
        // padded with zeroes
        task hashString_md5(
            input string                        msg,
            output logic[MD5_HASH_BITS - 1:0]   hash
        );

            int                             number_of_blocks;
            logic [DW-1:0]                  data;
            logic [63:0]                    message_length;
            axi_pkg::resp_t                 resp;

            // Initialize the hash output
            hash = "";

            // Copy the message length in bits
            message_length = 64'(msg.len * 8);

            `ifdef DEBUG1
                $display("DEBUG1: hashString_md5: Message length = %16h", message_length);
            `endif

            // Loop through all the blocks within the message.. understanding 
            // the need for padding and a 64-bit message length
            if ((msg.len + 8) % 64 == 0)
                number_of_blocks = (msg.len + 8) / 64;
            else
                number_of_blocks = ((msg.len + 8)) / 64 + 1;

            // Loop through all the blocks
            for (int i = 0; i < number_of_blocks; i++) begin

                // Each block is made up of sixteen 32-bit words
                for (int j = 0; j < 16; j++) begin

                    // First, copy 4-bytes from the input msg (if they exist),
                    // otherwise pad per the MD5 spec
                    for (int k = 0; k < 4; k++)
                        if (i * 64 + j * 4 + k < msg.len)
                            data[k*8+:8] = msg[i * 64 + j * 4 + k];
                        // Add a single 1-bit after the last byte of the message
                        else if (i * 64 + j * 4 + k == msg.len)
                            data[k*8+:8] = 8'h80;
                        else
                            data[k*8+:8] = 8'h00;

                    // Write the data
                    axi_lite_slave_write(   .addr(32'(32'(MD5_MSG_BASE) + (j * 4))),
                                            .data(data),
                                            .strb('1),
                                            .resp(resp));

                end // for (int j = 0; j < 16; j++)

                // Now that the block has been loaded, be sure to load the message length
                axi_lite_slave_write(   .addr(32'(32'(MD5_MSG_BASE) + 56)),
                                        .data(message_length[31:0]),
                                        .strb('1),
                                        .resp(resp));
                axi_lite_slave_write(   .addr(32'(32'(MD5_MSG_BASE) + 60)),
                                        .data(message_length[63:32]),
                                        .strb('1),
                                        .resp(resp));

                // A block of data has been loaded, now time to tell the MD5 core to hash
                axi_lite_slave_write(   .addr(32'(MD5_READY)),
                                        .data(32'h0000_0001),
                                        .strb('1),
                                        .resp(resp));

                // Poll the MD5 ready bit to see when the hash is done
                do begin
                    axi_lite_slave_read(    .addr(32'(MD5_READY)), 
                                            .data(data), 
                                            .resp(resp));
                end 
                while (data[0] == 1'b0);

            end // for (int i = 0; i < msg.len / 64; i++)

            // The entire block has been hashed, now to read the result
            for (int i = 0; i < 4; i++) begin

                // Read a word of the hash
                axi_lite_slave_read(    .addr(32'(MD5_HASH_BASE + (i * 4))), 
                                        .data(hash[i*32+:32]), 
                                        .resp(resp));

            end // for (int i = 0; i < 4; i++)

        endtask : hashString_md5

        // The following function will take a string as input and load it into the SHA256 core in
        // 512-bit chunks.  If an odd number of chunks is detected, then the message will be
        // padded with zeroes
        task hashString_sha256(
            input string                            msg,
            output logic[SHA256_HASH_BITS - 1:0]    hash
        );

            int                             number_of_blocks;
            logic [DW-1:0]                  data;
            logic [63:0]                    message_length;
            axi_pkg::resp_t                 resp;

            // Initialize the hash output
            hash = "";

            // Copy the message length in bits
            message_length = 64'(msg.len * 8);

            `ifdef DEBUG1
                $display("DEBUG1: hashString_sha256: Message length = %16h", message_length);
            `endif

            // Loop through all the blocks within the message.. understanding 
            // the need for padding and a 64-bit message length
            if ((msg.len + 8) % 64 == 0)
                number_of_blocks = (msg.len + 8) / 64;
            else
                number_of_blocks = ((msg.len + 8)) / 64 + 1;

            // Loop through all the blocks
            for (int i = 0; i < number_of_blocks; i++) begin

                // Each block is made up of sixteen 32-bit words
                for (int j = 0; j < 16; j++) begin

                    // First, copy 4-bytes from the input msg (if they exist),
                    // otherwise pad per the MD5 spec
                    for (int k = 0; k < 4; k++)
                        if (i * 64 + j * 4 + k < msg.len)
                            data[(3-k)*8+:8] = msg[i * 64 + j * 4 + k];
                        // Add a single 1-bit after the last byte of the message
                        else if (i * 64 + j * 4 + k == msg.len)
                            data[(3-k)*8+:8] = 8'h80;
                        else
                            data[(3-k)*8+:8] = 8'h00;

                    // Write the data
                    axi_lite_slave_write(   .addr(32'(32'(SHA256_MSG_BASE) + ((15 - j) * 4))),
                                            .data(data),
                                            .strb('1),
                                            .resp(resp));

                end // for (int j = 0; j < 16; j++)

                // Now that the block has been loaded, be sure to load the message length
                axi_lite_slave_write(   .addr(32'(32'(SHA256_MSG_BASE) + 0)),
                                        .data(message_length[31:0]),
                                        .strb('1),
                                        .resp(resp));
                axi_lite_slave_write(   .addr(32'(32'(SHA256_MSG_BASE) + 4)),
                                        .data(message_length[63:32]),
                                        .strb('1),
                                        .resp(resp));

                // A block of data has been loaded, now time to tell the SHA256 core to hash
                // If this is the first had in a message, strobe "init" else strobe "next"
                if (i == 0)
                    axi_lite_slave_write(   .addr(32'(SHA256_READY)),
                                            .data(32'h0000_0001),
                                            .strb('1),
                                            .resp(resp));
                else
                    axi_lite_slave_write(   .addr(32'(SHA256_READY)),
                                            .data(32'h0000_0002),
                                            .strb('1),
                                            .resp(resp));

                // Poll the MD5 ready bit to see when the hash is done
                do begin
                    axi_lite_slave_read(    .addr(32'(SHA256_HASH_DONE)), 
                                            .data(data), 
                                            .resp(resp));
                end 
                while (data[0] == 1'b0);

            end // for (int i = 0; i < msg.len / 64; i++)

            // The entire block has been hashed, now to read the result
            for (int i = 0; i < 8; i++) begin

                // Read a word of the hash
                axi_lite_slave_read(    .addr(32'(SHA256_HASH_BASE + (i * 4))), 
                                        .data(hash[i*32+:32]), 
                                        .resp(resp));

            end // for (int i = 0; i < 4; i++)

        endtask : hashString_sha256

        // Perform a 32-bit modulo exponentiation using the RSA core
        task modexp_32bits (
            input logic [31:0]  Wmsg,
            input logic [31:0]  Wexp,
            input logic [31:0]  Wmod,
            output logic [31:0] Wres
        );

            logic [DW-1:0]                  data;
            axi_pkg::resp_t                 resp;

        `ifdef DEBUG1
            $display("DEBUG1: modexp_32bits: Writing -> MES: %08h, EXP: %08h, MOD: %08h", Wmsg, Wexp, Wmod);
        `endif

            // Load the exponent
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_EXPONENT_PTR_RST)),
                                    .data(32'h0000_0000),
                                    .strb('1),
                                    .resp(resp));
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_EXPONENT_DATA)),
                                    .data(Wexp),
                                    .strb('1),
                                    .resp(resp));

            // Load the modulus
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_MODULUS_PTR_RST)),
                                    .data(32'h0000_0000),
                                    .strb('1),
                                    .resp(resp));
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_MODULUS_DATA)),
                                    .data(Wmod),
                                    .strb('1),
                                    .resp(resp));

            // Load the message
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_MESSAGE_PTR_RST)),
                                    .data(32'h0000_0000),
                                    .strb('1),
                                    .resp(resp));
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_MESSAGE_DATA)),
                                    .data(Wmsg),
                                    .strb('1),
                                    .resp(resp));

            // Load the exponent and modulus lengths
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_EXPONENT_LENGTH)),
                                    .data(32'h0000_0001),
                                    .strb('1),
                                    .resp(resp));
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_MODULUS_LENGTH)),
                                    .data(32'h0000_0001),
                                    .strb('1),
                                    .resp(resp));

            // Start processing
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_CTRL)),
                                    .data(32'h0000_0001),
                                    .strb('1),
                                    .resp(resp));

            // Wait until the core indicates it is not read
            do begin
                axi_lite_slave_read(    .addr(32'(RSA_ADDR_STATUS)), 
                                        .data(data), 
                                        .resp(resp));
            end 
            while (data[0] == 1'b0);

            // Read the results
            axi_lite_slave_write(   .addr(32'(RSA_ADDR_RESULT_PTR_RST)),
                                    .data(32'h0000_0000),
                                    .strb('1),
                                    .resp(resp));
            axi_lite_slave_read(    .addr(32'(RSA_ADDR_RESULT_DATA)), 
                                    .data(Wres), 
                                    .resp(resp));

        endtask : modexp_32bits

        // Set parameters for the DFT core
        task dft_setX (
            input logic [15:0] i,
            input logic [15:0] pX0,
            input logic [15:0] pX1,
            input logic [15:0] pX2,
            input logic [15:0] pX3
        );

            automatic axi_pkg::resp_t   resp;

            axi_lite_slave_write(  .addr(32'(DFT_IN_DATA)),     // Write Data
                                   .data(32'(pX1 << 16 | pX0)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));

            axi_lite_slave_write(  .addr(32'(DFT_IN_DATA + DFT_BLOCK_BYTES)),     // Write Data
                                   .data(32'(pX3 << 16 | pX2)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(DFT_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));


        endtask : dft_setX

        // Set parameters for the IDFT core
        task idft_setX (
            input logic [15:0] i,
            input logic [15:0] pX0,
            input logic [15:0] pX1,
            input logic [15:0] pX2,
            input logic [15:0] pX3
        );

            automatic axi_pkg::resp_t   resp;

            axi_lite_slave_write(  .addr(32'(IDFT_IN_DATA)),     // Write Data
                                   .data(32'(pX1 << 16 | pX0)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));

            axi_lite_slave_write(  .addr(32'(IDFT_IN_DATA + IDFT_BLOCK_BYTES)),     // Write Data
                                   .data(32'(pX3 << 16 | pX2)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IDFT_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));


        endtask : idft_setX

        // Read results from the DFT core
        task dft_getY (
            input logic [15:0] i,
            output logic [15:0] pY0,
            output logic [15:0] pY1,
            output logic [15:0] pY2,
            output logic [15:0] pY3
        );

            logic [DW-1:0]          data;
            axi_pkg::resp_t         resp;

            axi_lite_slave_write(  .addr(32'(DFT_OUT_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));


            axi_lite_slave_read(    .addr(32'(DFT_OUT_DATA)),    // Read data
                                    .data(data), 
                                    .resp(resp));
            pY0 = data;
            pY1 = data >> 16;

            axi_lite_slave_read(    .addr(32'(DFT_OUT_DATA + DFT_BLOCK_BYTES)),    // Read data
                                    .data(data), 
                                    .resp(resp));
            pY2 = data;
            pY3 = data >> 16;

        endtask : dft_getY


        // Read results from the IDFT core
        task idft_getY (
            input logic [15:0] i,
            output logic [15:0] pY0,
            output logic [15:0] pY1,
            output logic [15:0] pY2,
            output logic [15:0] pY3
        );

            logic [DW-1:0]          data;
            axi_pkg::resp_t         resp;

            axi_lite_slave_write(  .addr(32'(IDFT_OUT_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));


            axi_lite_slave_read(    .addr(32'(IDFT_OUT_DATA)),    // Read data
                                    .data(data), 
                                    .resp(resp));
            pY0 = data;
            pY1 = data >> 16;

            axi_lite_slave_read(    .addr(32'(IDFT_OUT_DATA + IDFT_BLOCK_BYTES)),    // Read data
                                    .data(data), 
                                    .resp(resp));
            pY2 = data;
            pY3 = data >> 16;

        endtask : idft_getY

        // Set parameters for the IIR core
        task iir_setInData (
            input logic [31:0] i,
            input logic [31:0] idata
        );

            automatic axi_pkg::resp_t   resp;

            axi_lite_slave_write(  .addr(32'(IIR_IN_DATA)),     // Write Data
                                   .data(idata),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IIR_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IIR_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(IIR_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));

        endtask : iir_setInData

        // Read results from the IIR core
        task iir_getOutData (
            input logic [31:0] i,
            output logic [31:0] odata
        );

            axi_pkg::resp_t         resp;

            axi_lite_slave_write(  .addr(32'(IIR_OUT_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));


            axi_lite_slave_read(    .addr(32'(IIR_OUT_DATA)),    // Read data
                                    .data(odata), 
                                    .resp(resp));

        endtask : iir_getOutData

        // Set parameters for the FIR core
        task fir_setInData (
            input logic [31:0] i,
            input logic [31:0] idata
        );

            automatic axi_pkg::resp_t   resp;

            axi_lite_slave_write(  .addr(32'(FIR_IN_DATA)),     // Write Data
                                   .data(idata),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(FIR_IN_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(FIR_IN_WRITE)),    // Load data
                                   .data(32'h0000_0001),
                                   .strb('1),
                                   .resp(resp));
            axi_lite_slave_write(  .addr(32'(FIR_IN_WRITE)),    // Stop
                                   .data(32'h0000_0000),
                                   .strb('1),
                                   .resp(resp));

        endtask : fir_setInData

        // Read results from the FIR core
        task fir_getOutData (
            input logic [31:0] i,
            output logic [31:0] odata
        );

            axi_pkg::resp_t         resp;

            axi_lite_slave_write(  .addr(32'(FIR_OUT_ADDR)),     // Write Address
                                   .data(32'(i)),
                                   .strb('1),
                                   .resp(resp));


            axi_lite_slave_read(    .addr(32'(FIR_OUT_DATA)),    // Read data
                                    .data(odata), 
                                    .resp(resp));

        endtask : fir_getOutData

    endclass // end class axi_lite_driver_plus
    
endpackage // end axi_test_extended
