//======================================================================
//
// modexp.v
// --------
// Top level wrapper for the modula exponentiation core. The core
// is used to implement public key algorithms such as RSA,
// DH, ElGamal etc.
//
// The core calculates the following function:
//
//   C = M ** e mod N
//
//   M is a message with a length of n bits
//   e is the exponent with a length of m bits
//   N is the modulus  with a length of n bits
//
//   n can be 32 and up to and including 8192 bits in steps
//   of 32 bits.
//   m can be one and up to and including 8192 bits in steps
//   of 32 bits.
//
// The core has a 32-bit memory like interface, but provides
// status signals to inform the system that a given operation
// has is done. Additionally, any errors will also be asserted.
//
//
// Author: Joachim Strombergson, Peter Magnusson
// Copyright (c) 2015, NORDUnet A/S
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the name of the NORDUnet nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//======================================================================

module modexp(
           input wire           clk,
           input wire           reset_n,

           input wire           cs,
           input wire           we,

           input wire  [ 7 : 0] address,
           input wire  [31 : 0] write_data,
           output wire [31 : 0] read_data
       );


//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------

// The operand width is the internal operand width in bits.
// The address width is the size of the address space used. This
// value must be balances with OPERAND_WIDTH to allow a total
// of 8192 bits of data. OPERAND_WIDTH * (ADDRESS_WIDTH ** 2)
// is the formula. Note that the API data with is always 32 bits.
localparam OPERAND_WIDTH         = 32;
localparam ADDRESS_WIDTH         = 8;

localparam ADDR_NAME0            = 8'h00;
localparam ADDR_NAME1            = 8'h01;
localparam ADDR_VERSION          = 8'h02;

localparam ADDR_CTRL             = 8'h08;
localparam CTRL_INIT_BIT         = 0;
localparam CTRL_NEXT_BIT         = 1;

localparam ADDR_STATUS           = 8'h09;
localparam STATUS_READY_BIT      = 0;

localparam ADDR_CYCLES_HIGH      = 8'h10;
localparam ADDR_CYCLES_LOW       = 8'h11;

localparam ADDR_MODULUS_LENGTH   = 8'h20;
localparam ADDR_EXPONENT_LENGTH  = 8'h21;

localparam ADDR_MODULUS_PTR_RST  = 8'h30;
localparam ADDR_MODULUS_DATA     = 8'h31;

localparam ADDR_EXPONENT_PTR_RST = 8'h40;
localparam ADDR_EXPONENT_DATA    = 8'h41;

localparam ADDR_MESSAGE_PTR_RST  = 8'h50;
localparam ADDR_MESSAGE_DATA     = 8'h51;

localparam ADDR_RESULT_PTR_RST   = 8'h60;
localparam ADDR_RESULT_DATA      = 8'h61;

localparam DEFAULT_MODLENGTH     = 8'h80; // 2048 bits.
localparam DEFAULT_EXPLENGTH     = 8'h80;

localparam CORE_NAME0            = 32'h6d6f6465; // "mode"
localparam CORE_NAME1            = 32'h78702020; // "xp  "
localparam CORE_VERSION          = 32'h302e3532; // "0.52"


//----------------------------------------------------------------
// Registers including update variables and write enable.
//----------------------------------------------------------------
reg [07 : 0] exponent_length_reg;
reg [07 : 0] exponent_length_new;
reg          exponent_length_we;

reg [07 : 0] modulus_length_reg;
reg [07 : 0] modulus_length_new;
reg          modulus_length_we;

reg          start_reg;
reg          start_new;


//----------------------------------------------------------------
// Wires.
//----------------------------------------------------------------
reg           exponent_mem_api_rst;
reg           exponent_mem_api_cs;
reg           exponent_mem_api_wr;
wire [31 : 0] exponent_mem_api_read_data;

reg           modulus_mem_api_rst;
reg           modulus_mem_api_cs;
reg           modulus_mem_api_wr;
wire [31 : 0] modulus_mem_api_read_data;

reg           message_mem_api_rst;
reg           message_mem_api_cs;
reg           message_mem_api_wr;
wire [31 : 0] message_mem_api_read_data;

reg           result_mem_api_rst;
reg           result_mem_api_cs;
wire [31 : 0] result_mem_api_read_data;

wire          ready;
wire [63 : 0] cycles;

reg [31 : 0]  tmp_read_data;


//----------------------------------------------------------------
// Concurrent connectivity for ports etc.
//----------------------------------------------------------------
assign read_data = tmp_read_data;


//----------------------------------------------------------------
// core instantiations.
//----------------------------------------------------------------
modexp_core #(.OPW(OPERAND_WIDTH), .ADW(ADDRESS_WIDTH))
            core_inst(
                .clk(clk),
                .reset_n(reset_n),

                .start(start_reg),
                .ready(ready),

                .exponent_length(exponent_length_reg),
                .modulus_length(modulus_length_reg),

                .cycles(cycles),

                .exponent_mem_api_cs(exponent_mem_api_cs),
                .exponent_mem_api_wr(exponent_mem_api_wr),
                .exponent_mem_api_rst(exponent_mem_api_rst),
                .exponent_mem_api_write_data(write_data),
                .exponent_mem_api_read_data(exponent_mem_api_read_data),

                .modulus_mem_api_cs(modulus_mem_api_cs),
                .modulus_mem_api_wr(modulus_mem_api_wr),
                .modulus_mem_api_rst(modulus_mem_api_rst),
                .modulus_mem_api_write_data(write_data),
                .modulus_mem_api_read_data(modulus_mem_api_read_data),

                .message_mem_api_cs(message_mem_api_cs),
                .message_mem_api_wr(message_mem_api_wr),
                .message_mem_api_rst(message_mem_api_rst),
                .message_mem_api_write_data(write_data),
                .message_mem_api_read_data(message_mem_api_read_data),

                .result_mem_api_cs(result_mem_api_cs),
                .result_mem_api_rst(result_mem_api_rst),
                .result_mem_api_read_data(result_mem_api_read_data)
            );


//----------------------------------------------------------------
// reg_update
//
// Update functionality for all registers in the core.
// All registers are positive edge triggered with asynchronous
// active low reset. All registers have write enable.
//----------------------------------------------------------------
always @ (posedge clk or negedge reset_n)
    begin
        if (!reset_n)
            begin
                start_reg           <= 1'b0;
                exponent_length_reg <= DEFAULT_EXPLENGTH;
                modulus_length_reg  <= DEFAULT_MODLENGTH;
            end
        else
            begin
                start_reg <= start_new;

                if (exponent_length_we)
                    begin
                        exponent_length_reg <= write_data[7 : 0];
                    end

                if (modulus_length_we)
                    begin
                        modulus_length_reg <= write_data[7 : 0];
                    end
            end
    end // reg_update


//----------------------------------------------------------------
// api
//
// The interface command decoding logic.
//----------------------------------------------------------------
always @*
    begin : api
        modulus_length_we    = 1'b0;
        exponent_length_we   = 1'b0;
        start_new            = 1'b0;

        modulus_mem_api_rst  = 1'b0;
        modulus_mem_api_cs   = 1'b0;
        modulus_mem_api_wr   = 1'b0;

        exponent_mem_api_rst = 1'b0;
        exponent_mem_api_cs  = 1'b0;
        exponent_mem_api_wr  = 1'b0;

        message_mem_api_rst  = 1'b0;
        message_mem_api_cs   = 1'b0;
        message_mem_api_wr   = 1'b0;

        result_mem_api_rst   = 1'b0;
        result_mem_api_cs    = 1'b0;

        tmp_read_data        = 32'h00000000;

        if (cs)
            begin
                if (we)
                    begin
                        case (address)
                            ADDR_CTRL:
                                begin
                                    start_new = write_data[0];
                                end

                            ADDR_MODULUS_LENGTH:
                                begin
                                    modulus_length_we = 1'b1;
                                end

                            ADDR_EXPONENT_LENGTH:
                                begin
                                    exponent_length_we = 1'b1;
                                end

                            ADDR_MODULUS_PTR_RST:
                                begin
                                    modulus_mem_api_rst = 1'b1;
                                end

                            ADDR_MODULUS_DATA:
                                begin
                                    modulus_mem_api_cs = 1'b1;
                                    modulus_mem_api_wr = 1'b1;
                                end

                            ADDR_EXPONENT_PTR_RST:
                                begin
                                    exponent_mem_api_rst = 1'b1;
                                end

                            ADDR_EXPONENT_DATA:
                                begin
                                    exponent_mem_api_cs = 1'b1;
                                    exponent_mem_api_wr = 1'b1;
                                end

                            ADDR_MESSAGE_PTR_RST:
                                begin
                                    message_mem_api_rst = 1'b1;
                                end

                            ADDR_MESSAGE_DATA:
                                begin
                                    message_mem_api_cs = 1'b1;
                                    message_mem_api_wr = 1'b1;
                                end

                            ADDR_RESULT_PTR_RST:
                                begin
                                    result_mem_api_rst = 1'b1;
                                end

                            default:
                                begin
                                end
                        endcase // case (address[7 : 0])
                    end // if (we)
                else
                    begin
                        case (address)
                            ADDR_NAME0:
                                tmp_read_data = CORE_NAME0;

                            ADDR_NAME1:
                                tmp_read_data = CORE_NAME1;

                            ADDR_VERSION:
                                tmp_read_data = CORE_VERSION;

                            ADDR_CTRL:
                                tmp_read_data = {31'h00000000, start_reg};

                            ADDR_STATUS:
                                tmp_read_data = {31'h00000000, ready};

                            ADDR_CYCLES_HIGH:
                                tmp_read_data = cycles[63 : 32];

                            ADDR_CYCLES_LOW:
                                tmp_read_data = cycles[31 : 0];

                            ADDR_MODULUS_LENGTH:
                                tmp_read_data = {24'h000000, modulus_length_reg};

                            ADDR_EXPONENT_LENGTH:
                                tmp_read_data = {24'h000000, exponent_length_reg};

                            ADDR_MODULUS_DATA:
                                begin
                                    modulus_mem_api_cs = 1'b1;
                                    tmp_read_data      = modulus_mem_api_read_data;
                                end

                            ADDR_EXPONENT_DATA:
                                begin
                                    exponent_mem_api_cs = 1'b1;
                                    tmp_read_data       = exponent_mem_api_read_data;
                                end

                            ADDR_MESSAGE_DATA:
                                begin
                                    message_mem_api_cs = 1'b1;
                                    tmp_read_data      = message_mem_api_read_data;
                                end

                            ADDR_RESULT_DATA:
                                begin
                                    result_mem_api_cs = 1'b1;
                                    tmp_read_data     = result_mem_api_read_data;
                                end

                            default:
                                begin
                                end
                        endcase // case (address)
                    end // else: !if(we)
            end // if (cs)
    end // block: api

endmodule // modexp

          //======================================================================
          // EOF modexp.v
          //======================================================================
