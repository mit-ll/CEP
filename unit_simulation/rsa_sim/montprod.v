//======================================================================
//
// montprod.v
// ---------
// Montgomery product calculator for the modular exponentiantion core.
//
// parameter OPW is operand word width in bits.
// parameter ADW is address width in bits.
//
//
// Author: Peter Magnusson, Joachim Strombergson
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

module montprod #(parameter OPW = 32, parameter ADW = 8)
       (
           input wire                  clk,
           input wire                  reset_n,

           input wire                  calculate,
           output wire                 ready,

           input wire [(ADW - 1) : 0]  length,

           output wire [(ADW - 1) : 0] opa_addr,
           input wire [(OPW - 1) : 0]  opa_data,

           output wire [(ADW - 1) : 0] opb_addr,
           input wire [(OPW - 1) : 0]  opb_data,

           output wire [(ADW - 1) : 0] opm_addr,
           input wire [(OPW - 1) : 0]  opm_data,

           output wire [(ADW - 1) : 0] result_addr,
           output wire [(OPW - 1) : 0] result_data,
           output wire                 result_we
       );


//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------
localparam CTRL_IDLE            = 4'h0;
localparam CTRL_LOOP_ITER       = 4'h1;
localparam CTRL_LOOP_BQ         = 4'h2;
localparam CTRL_CALC_ADD        = 4'h3;
localparam CTRL_STALLPIPE_ADD   = 4'h4;
localparam CTRL_CALC_SDIV2      = 4'h5;
localparam CTRL_STALLPIPE_SDIV2 = 4'h6;
localparam CTRL_L_STALLPIPE_ES  = 4'h7;
localparam CTRL_EMIT_S          = 4'h8;

localparam SMUX_ZERO            = 2'h0;
localparam SMUX_ADD             = 2'h1;
localparam SMUX_SHR             = 2'h2;


//----------------------------------------------------------------
// Registers including update variables and write enable.
//----------------------------------------------------------------
reg                  ready_reg;
reg                  ready_new;
reg                  ready_we;

reg [3 : 0]          montprod_ctrl_reg;
reg [3 : 0]          montprod_ctrl_new;
reg                  montprod_ctrl_we;

reg  [1 : 0]         s_mux_new;
reg  [1 : 0]         s_mux_reg;

reg                  s_mem_we_reg;
reg                  s_mem_we_new;

reg [(ADW - 1) : 0]  s_mem_read_addr_reg;

reg                  q_new;
reg                  q_reg;
reg                  b_new;
reg                  b_reg;
reg                  bq_we;

reg [12 : 0]         loop_ctr_reg;
reg [12 : 0]         loop_ctr_new;
reg                  loop_ctr_we;
reg                  loop_ctr_set;
reg                  loop_ctr_dec;

reg [(13 - ADW - 1) : 0] b_bit_index_reg;
reg [(13 - ADW - 1) : 0] b_bit_index_new;
reg                      b_bit_index_we;

reg [(ADW - 1) : 0]  word_index_reg;
reg [(ADW - 1) : 0]  word_index_new;
reg                  word_index_we;
reg [(ADW - 1) : 0]  word_index_prev_reg;
reg                  reset_word_index_lsw;
reg                  reset_word_index_msw;
reg                  inc_word_index;
reg                  dec_word_index;

reg                  add_carry_in_sa_reg;
reg                  add_carry_in_sa_new;
reg                  add_carry_in_sm_reg;
reg                  add_carry_in_sm_new;

reg                  shr_carry_in_reg;
reg                  shr_carry_in_new;

reg                  first_iteration_reg;
reg                  first_iteration_new;
reg                  first_iteration_we;

reg                  test_reg;
reg                  test_new;

reg [(OPW - 2) : 0]  shr_data_out_reg;
reg                  shr_carry_out_reg;
reg                  shr_carry_out_new;

//----------------------------------------------------------------
// Wires.
//----------------------------------------------------------------
wire [(OPW - 1) : 0] add_result_sa;
wire                 add_carry_out_sa;
wire [(OPW - 1) : 0] add_result_sm;
wire                 add_carry_out_sm;
reg [(ADW - 1) : 0]  b_word_index; //loop counter as a word index
/* verilator lint_off UNOPTFLAT */
reg  [(OPW - 1) : 0] shr_data_in;
/* verilator lint_on UNOPTFLAT */
wire                 shr_carry_out;
wire [(OPW - 1) : 0] shr_data_out;

reg [(ADW - 1) : 0]  tmp_opa_addr;
reg                  tmp_result_we;

reg [(ADW - 1) : 0]  s_mem_read_addr;
wire [(OPW - 1) : 0] s_mem_read_data;
reg [(ADW - 1) : 0]  s_mem_write_addr;
reg [(OPW - 1) : 0]  s_mem_write_data;
reg [(OPW - 1) : 0]  tmp_s_mem_write_data;

reg [(OPW - 1) : 0]  sa_adder_data_in;
/* verilator lint_off UNOPTFLAT */
reg [(OPW - 1) : 0]  muxed_s_mem_read_data;
/* verilator lint_on UNOPTFLAT */
reg [(OPW - 1) : 0]  shifted_s_mem_write_data;

wire [(ADW - 1) : 0] length_m1;

// Temporary debug wires.
reg [1 : 0] state_trace;
reg [1 : 0] mux_trace;

//----------------------------------------------------------------
// Concurrent connectivity for ports etc.
//----------------------------------------------------------------
assign length_m1   = length - 1'b1;

assign opa_addr    = tmp_opa_addr;
assign opb_addr    = b_word_index;
assign opm_addr    = word_index_reg;

assign result_addr = word_index_prev_reg;
assign result_data = s_mem_read_data;
assign result_we   = tmp_result_we;

assign ready       = ready_reg;


//----------------------------------------------------------------
// Instantions
//----------------------------------------------------------------
blockmem1r1w #(.OPW(OPW), .ADW(ADW)) s_mem(
                 .clk(clk),
                 .read_addr(s_mem_read_addr),
                 .read_data(s_mem_read_data),
                 .wr(s_mem_we_reg),
                 .write_addr(s_mem_write_addr),
                 .write_data(s_mem_write_data)
             );

adder #(.OPW(OPW)) s_adder_sm(
          .a(muxed_s_mem_read_data),
          .b(opm_data),
          .carry_in(add_carry_in_sm_reg),
          .sum(add_result_sm),
          .carry_out(add_carry_out_sm)
      );


adder #(.OPW(OPW)) s_adder_sa(
          .a(sa_adder_data_in),
          .b(opa_data),
          .carry_in(add_carry_in_sa_reg),
          .sum(add_result_sa),
          .carry_out(add_carry_out_sa)
      );

shr #(.OPW(OPW)) shifter(
        .a(shr_data_in),
        .carry_in(shr_carry_in_reg),
        .adiv2(shr_data_out),
        .carry_out(shr_carry_out)
    );


//----------------------------------------------------------------
// reg_update
//
// Update functionality for all registers in the core.
// All registers are positive edge triggered with asynchronous
// active low reset. All registers have write enable.
//----------------------------------------------------------------
always @ (posedge clk or negedge reset_n)
    begin : reg_update
        if (!reset_n)
            begin
                test_reg            <= 1'b1;
                ready_reg           <= 1'b1;
                loop_ctr_reg        <= 13'h0;
                word_index_reg      <= {ADW{1'b0}};
                word_index_prev_reg <= {ADW{1'b0}};
                add_carry_in_sa_reg <= 1'b0;
                add_carry_in_sm_reg <= 1'b0;
                shr_data_out_reg    <= {(OPW - 1){1'b0}};
                shr_carry_in_reg    <= 1'b0;
                b_reg               <= 1'b0;
                q_reg               <= 1'b0;
                s_mux_reg           <= SMUX_ZERO;
                s_mem_we_reg        <= 1'b0;
                s_mem_read_addr_reg <= {ADW{1'b0}};
                b_bit_index_reg     <= {(13 - ADW){1'b0}};
                first_iteration_reg <= 1'b0;
                montprod_ctrl_reg   <= CTRL_IDLE;
            end
        else
            begin
                test_reg            <= test_new;

                s_mem_read_addr_reg <= s_mem_read_addr;
                s_mem_we_reg        <= s_mem_we_new;
                s_mux_reg           <= s_mux_new;

                word_index_prev_reg <= word_index_reg;

                shr_carry_in_reg    <= shr_carry_in_new;
                add_carry_in_sa_reg <= add_carry_in_sa_new;
                add_carry_in_sm_reg <= add_carry_in_sm_new;
                shr_data_out_reg    <= shr_data_out[(OPW - 2) : 0];

                if (word_index_we)
                    word_index_reg <= word_index_new;

                if (first_iteration_we)
                    first_iteration_reg <= first_iteration_new;

                if (b_bit_index_we)
                    b_bit_index_reg <= b_bit_index_new;

                if (bq_we)
                    begin
                        b_reg <= b_new;
                        q_reg <= q_new;
                    end

                if (ready_we)
                    ready_reg <= ready_new;

                if (loop_ctr_we)
                    loop_ctr_reg <= loop_ctr_new;

                if (montprod_ctrl_we)
                    begin
                        montprod_ctrl_reg <= montprod_ctrl_new;
                    end
            end
    end // reg_update


//----------------------------------------------------------------
// s_logic
//
// Logic to calculate S memory updates including address
// and write enable. This is the main montprod datapath.
//----------------------------------------------------------------
always @*
    begin : s_logic
        shr_carry_in_new      = 1'b0;
        muxed_s_mem_read_data = {OPW{1'b0}};
        sa_adder_data_in      = {OPW{1'b0}};
        add_carry_in_sa_new   = 1'b0;
        add_carry_in_sm_new   = 1'b0;
        s_mem_read_addr       = word_index_reg;
        s_mem_write_addr      = s_mem_read_addr_reg;
        s_mem_write_data      = {OPW{1'b0}};
        s_mem_we_new          = 1'b0;
        state_trace           = 0;
        mux_trace             = 0;
        tmp_s_mem_write_data  = {OPW{1'b0}};
        test_new              = 1'b0;
       // Added missing default statement to avoid latches!! Tony Duong 04/01/2020
        shr_data_in = 0;
       
        case (montprod_ctrl_reg)
            CTRL_LOOP_ITER:
                begin
                    s_mem_read_addr = length_m1;
                end

            CTRL_CALC_ADD:
                begin
                    //s = (s + q*M + b*A) >>> 1;, if(b==1) S+= A. Takes (1..length) cycles.
                    s_mem_we_new = b_reg | q_reg | first_iteration_reg;
                    state_trace  = 1;
                    test_new     = 1'b1;
                end

            CTRL_CALC_SDIV2:
                begin
                    //s = (s + q*M + b*A) >>> 1; s>>=1.  Takes (1..length) cycles.
                    s_mem_we_new = 1'b1;
                end

            default:
                begin
                end
        endcase


        case (s_mux_reg)
            SMUX_ADD:
                begin
                    mux_trace = 1;

                    if (first_iteration_reg)
                        muxed_s_mem_read_data = {OPW{1'b0}};
                    else
                        muxed_s_mem_read_data = s_mem_read_data;


                    if (q_reg)
                        sa_adder_data_in = add_result_sm;
                    else
                        sa_adder_data_in = muxed_s_mem_read_data;


                    if (b_reg)
                        tmp_s_mem_write_data = add_result_sa;
                    else if (q_reg)
                        tmp_s_mem_write_data = add_result_sm;
                    else if (first_iteration_reg)
                        tmp_s_mem_write_data = {OPW{1'b0}};

                    s_mem_write_data = tmp_s_mem_write_data;
                    add_carry_in_sa_new = add_carry_out_sa;
                    add_carry_in_sm_new = add_carry_out_sm;


                    // Experimental integration of shift in add.
                    shr_data_in              = tmp_s_mem_write_data;
                    shifted_s_mem_write_data = {shr_carry_out, shr_data_out_reg};
                end


            SMUX_SHR:
                begin
                    shr_data_in      = s_mem_read_data;
                    s_mem_write_data = shr_data_out;
                    shr_carry_in_new = shr_carry_out;
                end

            default:
                begin
                end
        endcase
    end // s_logic


//----------------------------------------------------------------
// bq
//
// Extract b and q bits.
// b: current bit of B used.
// q = (s - b * A) & 1
// update the b bit and word indices based on loop counter.
//----------------------------------------------------------------
always @*
    begin : bq
        b_new = opb_data[b_bit_index_reg];

        if (first_iteration_reg)
            q_new = 1'b0 ^ (opa_data[0] & b_new);
        else
            q_new = s_mem_read_data[0] ^ (opa_data[0] & b_new);

        // B_bit_index      = 5'h1f - loop_counter[4:0];
        b_bit_index_new = ((2**(13 - ADW)) - 1'b1) - loop_ctr_reg[(13 - ADW - 1) : 0];
        b_word_index    = loop_ctr_reg[12 : (13 - ADW)];
    end // bq


//----------------------------------------------------------------
// word_index
//
// Logic that implements the word index used to drive
// addresses for operands.
//----------------------------------------------------------------
always @*
    begin : word_index
        word_index_new = {ADW{1'b0}};
        word_index_we  = 1'b0;

        if (reset_word_index_lsw)
            begin
                word_index_new = length_m1;
                word_index_we  = 1'b1;
            end

        if (reset_word_index_msw)
            begin
                word_index_new = {ADW{1'b0}};
                word_index_we  = 1'b1;
            end

        if (inc_word_index)
            begin
                word_index_new = word_index_reg + 1'b1;
                word_index_we  = 1'b1;
            end

        if (dec_word_index)
            begin
                word_index_new = word_index_reg - 1'b1;
                word_index_we  = 1'b1;
            end
    end // word_index


//----------------------------------------------------------------
// loop_ctr
// Logic for updating the loop counter.
//----------------------------------------------------------------
always @*
    begin : loop_ctr
        loop_ctr_new = 13'h0;
        loop_ctr_we  = 1'b0;

        if (loop_ctr_set)
            begin
                loop_ctr_new = {length, {(13 - ADW){1'b0}}} - 1'b1;
                loop_ctr_we  = 1'b1;
            end

        if (loop_ctr_dec)
            begin
                loop_ctr_new = loop_ctr_reg - 1'b1;
                loop_ctr_we  = 1'b1;
            end
    end


//----------------------------------------------------------------
// montprod_ctrl
//
// Control FSM for the montgomery product calculator.
//----------------------------------------------------------------
always @*
    begin : montprod_ctrl
        ready_new            = 1'b0;
        ready_we             = 1'b0;
        loop_ctr_set         = 1'b0;
        loop_ctr_dec         = 1'b0;
        b_bit_index_we       = 1'b0;
        bq_we                = 1'b0;
        s_mux_new            = SMUX_ZERO;
        dec_word_index       = 1'b0;
        inc_word_index       = 1'b0;
        reset_word_index_lsw = 1'b0;
        reset_word_index_msw = 1'b0;
        first_iteration_new  = 1'b0;
        first_iteration_we   = 1'b0;
        tmp_opa_addr         = word_index_reg;
        tmp_result_we        = 1'b0;
        montprod_ctrl_new    = CTRL_IDLE;
        montprod_ctrl_we     = 1'b0;


        case (montprod_ctrl_reg)
            CTRL_IDLE:
                begin
                    if (calculate)
                        begin
                            first_iteration_new  = 1'b1;
                            first_iteration_we   = 1'b1;
                            ready_new            = 1'b0;
                            ready_we             = 1'b1;
                            reset_word_index_lsw = 1'b1;
                            loop_ctr_set         = 1'b1;
                            montprod_ctrl_new    = CTRL_LOOP_ITER;
                            montprod_ctrl_we     = 1'b1;
                        end
                end

            //calculate q = (s - b * A) & 1;.
            // Also abort loop if done.
            CTRL_LOOP_ITER:
                begin
                    tmp_opa_addr         = length_m1;
                    b_bit_index_we       = 1'b1;
                    montprod_ctrl_new    = CTRL_LOOP_BQ;
                    montprod_ctrl_we     = 1'b1;
                end

            CTRL_LOOP_BQ:
                begin
                    reset_word_index_lsw = 1'b1;
                    bq_we                = 1'b1;
                    montprod_ctrl_new    = CTRL_CALC_ADD;
                    montprod_ctrl_we     = 1'b1;
                end

            CTRL_CALC_ADD:
                begin
                    s_mux_new      = SMUX_ADD;

                    if (word_index_reg == 0)
                        begin
                            reset_word_index_lsw = 1'b1;
                            montprod_ctrl_new    = CTRL_STALLPIPE_ADD;
                            montprod_ctrl_we     = 1'b1;
                        end
                    else
                        begin
                            dec_word_index = 1'b1;
                        end
                end

            CTRL_STALLPIPE_ADD:
                begin
                    first_iteration_new  = 1'b0;
                    first_iteration_we   = 1'b1;
                    reset_word_index_msw = 1'b1;
                    montprod_ctrl_new    = CTRL_CALC_SDIV2;
                    montprod_ctrl_we     = 1'b1;
                end

            CTRL_CALC_SDIV2:
                begin
                    s_mux_new      = SMUX_SHR;

                    if (word_index_reg == length_m1)
                        begin
                            montprod_ctrl_new = CTRL_STALLPIPE_SDIV2;
                            montprod_ctrl_we  = 1'b1;
                        end
                    else
                        inc_word_index = 1'b1;
                end

            CTRL_STALLPIPE_SDIV2:
                begin
                    loop_ctr_dec         = 1'b1;
                    montprod_ctrl_new    = CTRL_LOOP_ITER;
                    montprod_ctrl_we     = 1'b1;
                    reset_word_index_lsw = 1'b1;

                    if (loop_ctr_reg == 0)
                        begin
                            montprod_ctrl_new = CTRL_L_STALLPIPE_ES;
                            montprod_ctrl_we  = 1'b1;
                        end
                end

            CTRL_L_STALLPIPE_ES:
                begin
                    montprod_ctrl_new = CTRL_EMIT_S;
                    montprod_ctrl_we  = 1'b1;
                end

            CTRL_EMIT_S:
                begin
                    dec_word_index = 1'b1;
                    tmp_result_we  = 1'b1;

                    if (word_index_prev_reg == 0)
                        begin
                            ready_new         = 1'b1;
                            ready_we          = 1'b1;
                            montprod_ctrl_new = CTRL_IDLE;
                            montprod_ctrl_we  = 1'b1;
                        end
                end

            default:
                begin
                end

        endcase // case (montprod_ctrl_reg)
    end // montprod_ctrl

endmodule // montprod

          //======================================================================
          // EOF montprod.v
          //======================================================================
