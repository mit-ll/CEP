//======================================================================
//
// residue.v
// ---------
// Modulus 2**2N residue calculator for montgomery calculations.
//
// m_residue_2_2N_array( N, M, Nr)
//   Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
//   for (int i = 0; i < 2 * N; i++)
//     Nr = Nr shift left 1
//     if (Nr less than M) continue;
//     Nr = Nr - M
// return Nr
//
//
//
// Author: Peter Magnusson
// Copyright (c) 2015, NORDUnet A/S All rights reserved.
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
//======================================================================

module residue #(parameter OPW = 32, parameter ADW = 8)
       (
           input wire                  clk,
           input wire                  reset_n,

           input wire                  calculate,
           output wire                 ready,

           input wire  [14 : 0]        nn, //MAX(2*N)=8192*2 (14 bit)
           input wire  [(ADW - 1) : 0] length,

           output wire [(ADW - 1) : 0] opa_rd_addr,
           input wire  [(OPW - 1) : 0] opa_rd_data,
           output wire [(ADW - 1) : 0] opa_wr_addr,
           output wire [(OPW - 1) : 0] opa_wr_data,
           output wire                 opa_wr_we,

           output wire [(ADW - 1) : 0] opm_addr,
           input wire  [(OPW - 1) : 0] opm_data
       );

//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------
localparam CTRL_IDLE          = 4'h0;
localparam CTRL_INIT          = 4'h1;
localparam CTRL_INIT_STALL    = 4'h2;
localparam CTRL_SHL           = 4'h3;
localparam CTRL_SHL_STALL     = 4'h4;
localparam CTRL_COMPARE       = 4'h5;
localparam CTRL_COMPARE_STALL = 4'h6;
localparam CTRL_SUB           = 4'h7;
localparam CTRL_SUB_STALL     = 4'h8;
localparam CTRL_LOOP          = 4'h9;


//----------------------------------------------------------------
// Registers including update variables and write enable.
//----------------------------------------------------------------
reg [(ADW - 1) : 0]  opa_rd_addr_reg;
reg [(ADW - 1) : 0]  opa_wr_addr_reg;
reg [(OPW - 1) : 0]  opa_wr_data_reg;
reg                  opa_wr_we_reg;
reg [(ADW - 1) : 0]  opm_addr_reg;
reg                  ready_reg;
reg                  ready_new;
reg                  ready_we;
reg [03 : 0]         residue_ctrl_reg;
reg [03 : 0]         residue_ctrl_new;
reg                  residue_ctrl_we;
reg                  reset_word_index;
reg                  reset_n_counter;
reg [14 : 0]         loop_counter_1_to_nn_reg; //for i = 1 to nn (2*N)
reg [14 : 0]         loop_counter_1_to_nn_new;
reg                  loop_counter_1_to_nn_we;
reg [14 : 0]         nn_reg;
reg                  nn_we;
reg [(ADW - 1) : 0]  length_m1_reg;
reg [(ADW - 1) : 0]  length_m1_new;
reg                  length_m1_we;
reg [(ADW - 1) : 0]  word_index_reg;
reg [(ADW - 1) : 0]  word_index_new;
reg                  word_index_we;

reg  [(OPW - 1) : 0] one_data;
wire [(OPW - 1) : 0] sub_data;
wire [(OPW - 1) : 0] shl_data;
reg                  sub_carry_in_new;
reg                  sub_carry_in_reg;
wire                 sub_carry_out;
reg                  shl_carry_in_new;
reg                  shl_carry_in_reg;
wire                 shl_carry_out;


//----------------------------------------------------------------
// Concurrent connectivity for ports etc.
//----------------------------------------------------------------
assign opa_rd_addr = opa_rd_addr_reg;
assign opa_wr_addr = opa_wr_addr_reg;
assign opa_wr_data = opa_wr_data_reg;
assign opa_wr_we   = opa_wr_we_reg;
assign opm_addr    = opm_addr_reg;
assign ready       = ready_reg;


//----------------------------------------------------------------
// Instantions
//----------------------------------------------------------------
adder #(.OPW(OPW)) add_inst(
          .a(opa_rd_data),
          .b( ~ opm_data),
          .carry_in(sub_carry_in_reg),
          .sum(sub_data),
          .carry_out(sub_carry_out)
      );

shl #(.OPW(OPW)) shl_inst(
        .a(opa_rd_data),
        .carry_in(shl_carry_in_reg),
        .amul2(shl_data),
        .carry_out(shl_carry_out)
    );


//----------------------------------------------------------------
// reg_update
//----------------------------------------------------------------
always @ (posedge clk or negedge reset_n)
    begin
        if (!reset_n)
            begin
                residue_ctrl_reg         <= CTRL_IDLE;
                word_index_reg           <= {ADW{1'b1}};
                length_m1_reg            <= {ADW{1'b1}};
                nn_reg                   <= 15'h0;
                loop_counter_1_to_nn_reg <= 15'h0;
                ready_reg                <= 1'b1;
                sub_carry_in_reg         <= 1'b0;
                shl_carry_in_reg         <= 1'b0;
            end
        else
            begin
                if (residue_ctrl_we)
                    residue_ctrl_reg <= residue_ctrl_new;

                if (word_index_we)
                    word_index_reg <= word_index_new;

                if (length_m1_we)
                    length_m1_reg <= length_m1_new;

                if (nn_we)
                    nn_reg <= nn;

                if (loop_counter_1_to_nn_we)
                    loop_counter_1_to_nn_reg <= loop_counter_1_to_nn_new;

                if (ready_we)
                    ready_reg <= ready_new;

                sub_carry_in_reg <= sub_carry_in_new;
                shl_carry_in_reg <= shl_carry_in_new;
            end
    end // reg_update


//----------------------------------------------------------------
// loop counter process. implements for (int i = 0; i < 2 * N; i++)
//
// m_residue_2_2N_array( N, M, Nr)
//   Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
//   for (int i = 0; i < 2 * N; i++)
//     Nr = Nr shift left 1
//     if (Nr less than M) continue;
//     Nr = Nr - M
// return Nr
//
//----------------------------------------------------------------
always @*
    begin : process_1_to_2n
        loop_counter_1_to_nn_new = loop_counter_1_to_nn_reg + 15'h1;
        loop_counter_1_to_nn_we  = 1'b0;

        if (reset_n_counter)
            begin
                loop_counter_1_to_nn_new = 15'h1;
                loop_counter_1_to_nn_we  = 1'b1;
            end

        if (residue_ctrl_reg == CTRL_LOOP)
            loop_counter_1_to_nn_we  = 1'b1;
    end


//----------------------------------------------------------------
// implements looping over words in a multiword operation
//----------------------------------------------------------------
always @*
    begin : word_index_process
        word_index_new = word_index_reg - 1'b1;
        word_index_we  = 1'b1;

        if (reset_word_index)
            word_index_new = length_m1_reg;

        if (residue_ctrl_reg == CTRL_IDLE)
            //reduce a pipeline stage with early read
            word_index_new = length_m1_new;
    end


//----------------------------------------------------------------
// writer process. implements:
//   Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
//   Nr = Nr shift left 1
//   Nr = Nr - M
//
// m_residue_2_2N_array( N, M, Nr)
//   Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
//   for (int i = 0; i < 2 * N; i++)
//     Nr = Nr shift left 1
//     if (Nr less than M) continue;
//     Nr = Nr - M
// return Nr
//----------------------------------------------------------------
always @*
    begin : writer_process
        opa_wr_addr_reg = word_index_reg;
        case (residue_ctrl_reg)
            CTRL_INIT:
                begin
                    opa_wr_data_reg = one_data;
                    opa_wr_we_reg   = 1'b1;
                end

            CTRL_SUB:
                begin
                    opa_wr_data_reg = sub_data;
                    opa_wr_we_reg   = 1'b1;
                end

            CTRL_SHL:
                begin
                    opa_wr_data_reg = shl_data;
                    opa_wr_we_reg   = 1'b1;
                end

            default:
                begin
                    opa_wr_data_reg = 32'h0;
                    opa_wr_we_reg   = 1'b0;
                end
        endcase
    end

//----------------------------------------------------------------
// reader process. reads from new value because it occurs one
// cycle earlier than the writer.
//----------------------------------------------------------------
always @*
    begin : reader_process
        opa_rd_addr_reg = word_index_new;
        opm_addr_reg    = word_index_new;
    end


//----------------------------------------------------------------
// carry process. "Ripple carry awesomeness!"
//----------------------------------------------------------------
always @*
    begin : carry_process
        case (residue_ctrl_reg)
            CTRL_COMPARE:
                sub_carry_in_new = sub_carry_out;
            CTRL_SUB:
                sub_carry_in_new = sub_carry_out;
            default:
                sub_carry_in_new = 1'b1;
        endcase

        case (residue_ctrl_reg)
            CTRL_SHL:
                shl_carry_in_new = shl_carry_out;
            default:
                shl_carry_in_new = 1'b0;
        endcase
    end


//----------------------------------------------------------------
// Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
//----------------------------------------------------------------
always @*
    begin : one_process
        one_data = 32'h0;
        if (residue_ctrl_reg == CTRL_INIT)
            if (word_index_reg == length_m1_reg)
                one_data = {{(OPW - 1){1'b0}}, 1'b1};
    end


//----------------------------------------------------------------
// residue_ctrl
//
// Control FSM for residue
//----------------------------------------------------------------
always @*
    begin : residue_ctrl
        ready_new        = 1'b0;
        ready_we         = 1'b0;
        reset_word_index = 1'b0;
        reset_n_counter  = 1'b0;
        length_m1_new    = length - 1'b1;
        length_m1_we     = 1'b0;
        nn_we            = 1'b0;
        residue_ctrl_new = CTRL_IDLE;
        residue_ctrl_we  = 1'b0;

        case (residue_ctrl_reg)
            CTRL_IDLE:
                if (calculate)
                    begin
                        ready_new        = 1'b0;
                        ready_we         = 1'b1;
                        reset_word_index = 1'b1;
                        length_m1_we     = 1'b1;
                        nn_we            = 1'b1;
                        residue_ctrl_new = CTRL_INIT;
                        residue_ctrl_we  = 1'b1;
                    end

            // Nr = 00...01 ; Nr = 1 == 2**(2N-2N)
            CTRL_INIT:
                if (word_index_reg == 0)
                    begin
                        residue_ctrl_new = CTRL_INIT_STALL;
                        residue_ctrl_we  = 1'b1;
                    end

            CTRL_INIT_STALL:
                begin
                    reset_word_index = 1'b1;
                    reset_n_counter  = 1'b1;
                    residue_ctrl_new = CTRL_SHL;
                    residue_ctrl_we  = 1'b1;
                end

            // Nr = Nr shift left 1
            CTRL_SHL:
                begin
                    if (word_index_reg == 0)
                        begin
                            residue_ctrl_new = CTRL_SHL_STALL;
                            residue_ctrl_we  = 1'b1;
                        end
                end

            CTRL_SHL_STALL:
                begin
                    reset_word_index = 1'b1;
                    residue_ctrl_new = CTRL_COMPARE;
                    residue_ctrl_we  = 1'b1;
                end

            //if (Nr less than M) continue
            CTRL_COMPARE:
                if (word_index_reg == 0)
                    begin
                        residue_ctrl_new = CTRL_COMPARE_STALL;
                        residue_ctrl_we  = 1'b1;
                    end

            CTRL_COMPARE_STALL:
                begin
                    reset_word_index = 1'b1;
                    residue_ctrl_we  = 1'b1;
                    if (sub_carry_in_reg == 1'b1)
                        //TODO: Bug! detect CF to detect less than, but no detect ZF to detect equal to.
                        residue_ctrl_new = CTRL_SUB;
                    else
                        residue_ctrl_new = CTRL_LOOP;
                end

            //Nr = Nr - M
            CTRL_SUB:
                if (word_index_reg == 0)
                    begin
                        residue_ctrl_new = CTRL_SUB_STALL;
                        residue_ctrl_we  = 1'b1;
                    end

            CTRL_SUB_STALL:
                begin
                    residue_ctrl_new = CTRL_LOOP;
                    residue_ctrl_we  = 1'b1;
                end

            //for (int i = 0; i < 2 * N; i++)
            CTRL_LOOP:
                begin
                    if (loop_counter_1_to_nn_reg == nn_reg)
                        begin
                            ready_new = 1'b1;
                            ready_we  = 1'b1;
                            residue_ctrl_new = CTRL_IDLE;
                            residue_ctrl_we  = 1'b1;
                        end
                    else
                        begin
                            reset_word_index = 1'b1;
                            residue_ctrl_new = CTRL_SHL;
                            residue_ctrl_we  = 1'b1;
                        end
                end

            default:
                begin
                end

        endcase
    end

endmodule // residue

          //======================================================================
          // EOF residue.v
          //======================================================================
