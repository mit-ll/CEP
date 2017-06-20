//======================================================================
//
// modexp_core.v
// -------------
// Modular exponentiation core for implementing public key algorithms
// such as RSA, DH, ElGamal etc.
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
// The core has access ports for the exponent, modulus, message and
// result memories.
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

module modexp_core #(parameter OPW = 32, parameter ADW = 8)
                  (
                   input wire           clk,
                   input wire           reset_n,

                   input wire           start,
                   output wire          ready,

                   input wire [07 : 0]  exponent_length,
                   input wire [07 : 0]  modulus_length,

                   output wire [63 : 0] cycles,

                   input wire           exponent_mem_api_cs,
                   input wire           exponent_mem_api_wr,
                   input wire           exponent_mem_api_rst,
                   input wire [31 : 0]  exponent_mem_api_write_data,
                   output wire [31 : 0] exponent_mem_api_read_data,

                   input wire           modulus_mem_api_cs,
                   input wire           modulus_mem_api_wr,
                   input wire           modulus_mem_api_rst,
                   input wire [31 : 0]  modulus_mem_api_write_data,
                   output wire [31 : 0] modulus_mem_api_read_data,

                   input wire           message_mem_api_cs,
                   input wire           message_mem_api_wr,
                   input wire           message_mem_api_rst,
                   input wire [31 : 0]  message_mem_api_write_data,
                   output wire [31 : 0] message_mem_api_read_data,

                   input wire           result_mem_api_cs,
                   input wire           result_mem_api_rst,
                   output wire [31 : 0] result_mem_api_read_data
                  );


  //----------------------------------------------------------------
  // Internal constant and parameter definitions.
  //----------------------------------------------------------------
  localparam MONTPROD_SELECT_ONE_NR = 3'h0;
  localparam MONTPROD_SELECT_X_NR   = 3'h1;
  localparam MONTPROD_SELECT_Z_P    = 3'h2;
  localparam MONTPROD_SELECT_P_P    = 3'h3;
  localparam MONTPROD_SELECT_Z_ONE  = 3'h4;

  localparam MONTPROD_DEST_Z        = 2'b00;
  localparam MONTPROD_DEST_P        = 2'b01;
  localparam MONTPROD_DEST_NOWHERE  = 2'b10;

  localparam CTRL_IDLE              = 4'h0;
  localparam CTRL_RESIDUE           = 4'h1;
  localparam CTRL_CALCULATE_Z0      = 4'h2;
  localparam CTRL_CALCULATE_P0      = 4'h3;
  localparam CTRL_ITERATE           = 4'h4;
  localparam CTRL_ITERATE_Z_P       = 4'h5;
  localparam CTRL_ITERATE_P_P       = 4'h6;
  localparam CTRL_ITERATE_END       = 4'h7;
  localparam CTRL_CALCULATE_ZN      = 4'h8;
  localparam CTRL_DONE              = 4'h9;

  //for rsa, c=M^65537 etc, there is no need to slow down to hide the exponent
  localparam EXPONATION_MODE_SECRET_SECURE = 1'b0;
  localparam EXPONATION_MODE_PUBLIC_FAST   = 1'b1;


  //----------------------------------------------------------------
  // Registers including update variables and write enable.
  //----------------------------------------------------------------
  reg          ready_reg;
  reg          ready_new;
  reg          ready_we;

  reg [2 : 0]  montprod_select_reg;
  reg [2 : 0]  montprod_select_new;
  reg          montprod_select_we;
  reg [1 : 0]  montprod_dest_reg;
  reg [1 : 0]  montprod_dest_new;
  reg          montprod_dest_we;

  reg [3 : 0]  modexp_ctrl_reg;
  reg [3 : 0]  modexp_ctrl_new;
  reg          modexp_ctrl_we;

  reg [31 : 0] one_reg;
  reg [31 : 0] one_new;
  reg [31 : 0] b_one_reg;
  reg [31 : 0] b_one_new;

  reg [12 : 0] loop_counter_reg;
  reg [12 : 0] loop_counter_new;
  reg          loop_counter_we;

  reg [07 : 0] E_word_index;
  reg [04 : 0] E_bit_index;
  reg          last_iteration;

  reg          ei_reg;
  reg          ei_new;
  reg          ei_we;

  reg          exponation_mode_reg;
  reg          exponation_mode_new;
  reg          exponation_mode_we;

  reg [31 : 0] cycle_ctr_low_reg;
  reg [31 : 0] cycle_ctr_low_new;
  reg          cycle_ctr_low_we;
  reg [31 : 0] cycle_ctr_high_reg;
  reg [31 : 0] cycle_ctr_high_new;
  reg          cycle_ctr_high_we;
  reg          cycle_ctr_state_reg;
  reg          cycle_ctr_state_new;
  reg          cycle_ctr_state_we;
  reg          cycle_ctr_start;
  reg          cycle_ctr_stop;


  //----------------------------------------------------------------
  // Wires.
  //----------------------------------------------------------------
  reg [07 : 0]  modulus_mem_int_rd_addr;
  wire [31 : 0] modulus_mem_int_rd_data;

  reg [07 : 0]  message_mem_int_rd_addr;
  wire [31 : 0] message_mem_int_rd_data;

  reg [07 : 0]  exponent_mem_int_rd_addr;
  wire [31 : 0] exponent_mem_int_rd_data;

  reg  [07 : 0] result_mem_int_rd_addr;
  wire [31 : 0] result_mem_int_rd_data;
  reg  [07 : 0] result_mem_int_wr_addr;
  reg  [31 : 0] result_mem_int_wr_data;
  reg           result_mem_int_we;

  reg  [07 : 0] p_mem_rd0_addr;
  wire [31 : 0] p_mem_rd0_data;
  reg  [07 : 0] p_mem_rd1_addr;
  wire [31 : 0] p_mem_rd1_data;
  reg  [07 : 0] p_mem_wr_addr;
  reg  [31 : 0] p_mem_wr_data;
  reg           p_mem_we;

  reg [31 : 0]  tmp_read_data;

  reg           montprod_calc;
  wire          montprod_ready;
  reg [07 : 0]  montprod_length;

  wire [07 : 0] montprod_opa_addr;
  reg [31 : 0]  montprod_opa_data;

  wire [07 : 0] montprod_opb_addr;
  reg [31 : 0]  montprod_opb_data;

  wire [07 : 0] montprod_opm_addr;
  reg [31 : 0]  montprod_opm_data;

  wire [07 : 0] montprod_result_addr;
  wire [31 : 0] montprod_result_data;
  wire          montprod_result_we;

  reg           residue_calculate;
  wire          residue_ready;
  reg [14 : 0]  residue_nn;
  reg  [07 : 0] residue_length;
  wire [07 : 0] residue_opa_rd_addr;
  wire [31 : 0] residue_opa_rd_data;
  wire [07 : 0] residue_opa_wr_addr;
  wire [31 : 0] residue_opa_wr_data;
  wire          residue_opa_wr_we;
  wire [07 : 0] residue_opm_addr;
  reg [31 : 0]  residue_opm_data;

  reg  [07 : 0] residue_mem_montprod_read_addr;
  wire [31 : 0] residue_mem_montprod_read_data;

  reg           residue_valid_reg;
  reg           residue_valid_new;
  reg           residue_valid_int_validated;

  wire [7 : 0]  modulus_length_m1;
  wire [7 : 0]  exponent_length_m1;


  //----------------------------------------------------------------
  // Concurrent connectivity for ports etc.
  //----------------------------------------------------------------
  assign ready  = ready_reg;
  assign cycles = {cycle_ctr_high_reg, cycle_ctr_low_reg};

  assign modulus_length_m1  = modulus_length - 8'h1;
  assign exponent_length_m1 = exponent_length - 8'h1;


  //----------------------------------------------------------------
  // core instantiations.
  //----------------------------------------------------------------
  montprod #(.OPW(OPW), .ADW(ADW))
  montprod_inst(
                .clk(clk),
                .reset_n(reset_n),

                .calculate(montprod_calc),
                .ready(montprod_ready),

                .length(montprod_length),

                .opa_addr(montprod_opa_addr),
                .opa_data(montprod_opa_data),

                .opb_addr(montprod_opb_addr),
                .opb_data(montprod_opb_data),

                .opm_addr(montprod_opm_addr),
                .opm_data(montprod_opm_data),

                .result_addr(montprod_result_addr),
                .result_data(montprod_result_data),
                .result_we(montprod_result_we)
               );


  residue #(.OPW(OPW), .ADW(ADW))
  residue_inst(
               .clk(clk),
               .reset_n(reset_n),
               .calculate(residue_calculate),
               .ready(residue_ready),
               .nn(residue_nn),
               .length(residue_length),
               .opa_rd_addr(residue_opa_rd_addr),
               .opa_rd_data(residue_opa_rd_data),
               .opa_wr_addr(residue_opa_wr_addr),
               .opa_wr_data(residue_opa_wr_data),
               .opa_wr_we(residue_opa_wr_we),
               .opm_addr(residue_opm_addr),
               .opm_data(residue_opm_data)
              );


  blockmem2r1w #(.OPW(OPW), .ADW(ADW))
  residue_mem(
              .clk(clk),
              .read_addr0(residue_opa_rd_addr),
              .read_data0(residue_opa_rd_data),
              .read_addr1(residue_mem_montprod_read_addr),
              .read_data1(residue_mem_montprod_read_data),
              .wr(residue_opa_wr_we),
              .write_addr(residue_opa_wr_addr),
              .write_data(residue_opa_wr_data)
             );


  blockmem2r1w #(.OPW(OPW), .ADW(ADW))
  p_mem(
        .clk(clk),
        .read_addr0(p_mem_rd0_addr),
        .read_data0(p_mem_rd0_data),
        .read_addr1(p_mem_rd1_addr),
        .read_data1(p_mem_rd1_data),
        .wr(p_mem_we),
        .write_addr(p_mem_wr_addr),
        .write_data(p_mem_wr_data)
       );


  blockmem2r1wptr #(.OPW(OPW), .ADW(ADW))
  exponent_mem(
               .clk(clk),
               .reset_n(reset_n),
               .read_addr0(exponent_mem_int_rd_addr),
               .read_data0(exponent_mem_int_rd_data),
               .read_data1(exponent_mem_api_read_data),
               .rst(exponent_mem_api_rst),
               .cs(exponent_mem_api_cs),
               .wr(exponent_mem_api_wr),
               .write_data(exponent_mem_api_write_data)
              );


  blockmem2r1wptr #(.OPW(OPW), .ADW(ADW))
  modulus_mem(
              .clk(clk),
              .reset_n(reset_n),
              .read_addr0(modulus_mem_int_rd_addr),
              .read_data0(modulus_mem_int_rd_data),
              .read_data1(modulus_mem_api_read_data),
              .rst(modulus_mem_api_rst),
              .cs(modulus_mem_api_cs),
              .wr(modulus_mem_api_wr),
              .write_data(modulus_mem_api_write_data)
             );


  blockmem2r1wptr #(.OPW(OPW), .ADW(ADW))
  message_mem(
              .clk(clk),
              .reset_n(reset_n),
              .read_addr0(message_mem_int_rd_addr),
              .read_data0(message_mem_int_rd_data),
              .read_data1(message_mem_api_read_data),
              .rst(message_mem_api_rst),
              .cs(message_mem_api_cs),
              .wr(message_mem_api_wr),
              .write_data(message_mem_api_write_data)
             );


  blockmem2rptr1w #(.OPW(OPW), .ADW(ADW))
  result_mem(
             .clk(clk),
             .reset_n(reset_n),
             .read_addr0(result_mem_int_rd_addr[7 : 0]),
             .read_data0(result_mem_int_rd_data),
             .read_data1(result_mem_api_read_data),
             .rst(result_mem_api_rst),
             .cs(result_mem_api_cs),
             .wr(result_mem_int_we),
             .write_addr(result_mem_int_wr_addr),
             .write_data(result_mem_int_wr_data)
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
          ready_reg           <= 1'b1;
          montprod_select_reg <= MONTPROD_SELECT_ONE_NR;
          montprod_dest_reg   <= MONTPROD_DEST_NOWHERE;
          modexp_ctrl_reg     <= CTRL_IDLE;
          one_reg             <= 32'h0;
          b_one_reg           <= 32'h0;
          loop_counter_reg    <= 13'b0;
          ei_reg              <= 1'b0;
          residue_valid_reg   <= 1'b0;
          exponation_mode_reg <= EXPONATION_MODE_SECRET_SECURE;
          cycle_ctr_low_reg   <= 32'h00000000;
          cycle_ctr_high_reg  <= 32'h00000000;
          cycle_ctr_state_reg <= 1'b0;
        end
      else
        begin
          one_reg             <= one_new;
          b_one_reg           <= b_one_new;
          residue_valid_reg   <= residue_valid_new;

          if (ready_we)
            ready_reg <= ready_new;

          if (montprod_select_we)
            montprod_select_reg <= montprod_select_new;

          if (montprod_dest_we)
            montprod_dest_reg <= montprod_dest_new;

          if (loop_counter_we)
            loop_counter_reg <= loop_counter_new;

          if (ei_we)
            ei_reg <= ei_new;

          if (exponation_mode_we)
            exponation_mode_reg <= exponation_mode_new;

          if (cycle_ctr_low_we)
            cycle_ctr_low_reg <= cycle_ctr_low_new;

          if (cycle_ctr_high_we)
            cycle_ctr_high_reg <= cycle_ctr_high_new;

          if (cycle_ctr_state_we)
            cycle_ctr_state_reg <= cycle_ctr_state_new;

          if (modexp_ctrl_we)
            modexp_ctrl_reg <= modexp_ctrl_new;
        end
    end // reg_update


  //----------------------------------------------------------------
  // cycle_ctr
  //
  // Implementation of the cycle counter
  //----------------------------------------------------------------
  always @*
    begin : cycle_ctr
      cycle_ctr_low_new   = 32'h00000000;
      cycle_ctr_low_we    = 1'b0;
      cycle_ctr_high_new  = 32'h00000000;
      cycle_ctr_high_we   = 1'b0;
      cycle_ctr_state_new = 1'b0;
      cycle_ctr_state_we  = 1'b0;

      if (cycle_ctr_start)
        begin
          cycle_ctr_low_new   = 32'h00000000;
          cycle_ctr_low_we    = 1'b1;
          cycle_ctr_high_new  = 32'h00000000;
          cycle_ctr_high_we   = 1'b1;
          cycle_ctr_state_new = 1'b1;
          cycle_ctr_state_we  = 1'b1;
        end

      if (cycle_ctr_stop)
        begin
          cycle_ctr_state_new = 1'b0;
          cycle_ctr_state_we  = 1'b1;
        end

      if (cycle_ctr_state_reg)
        begin
          cycle_ctr_low_new = cycle_ctr_low_reg + 1'b1;
          cycle_ctr_low_we  = 1'b1;

          if (cycle_ctr_low_new == 32'h00000000)
            begin
              cycle_ctr_high_new  = cycle_ctr_high_reg + 1'b1;
              cycle_ctr_high_we   = 1'b1;
            end
        end
    end // cycle_ctr


  //----------------------------------------------------------------
  // one
  //
  // generates the big integer one ( 00... 01 )
  //----------------------------------------------------------------
  always @*
    begin : one_process
      one_new   = 32'h00000000;
      b_one_new = 32'h00000000;

      if (montprod_opa_addr == modulus_length_m1)
        one_new = 32'h00000001;

      if (montprod_opb_addr == modulus_length_m1)
        b_one_new = 32'h00000001;
    end


  //----------------------------------------------------------------
  // Read mux for modulus. Needed since it is being
  // addressed by two sources.
  //----------------------------------------------------------------
  always @*
    begin : modulus_mem_reader_process
      if (modexp_ctrl_reg == CTRL_RESIDUE)
        modulus_mem_int_rd_addr = residue_opm_addr;
      else
        modulus_mem_int_rd_addr = montprod_opm_addr;
    end


  //----------------------------------------------------------------
  // Feeds residue calculator.
  //----------------------------------------------------------------
  always @*
    begin : residue_process
      //N*2, N=length*32, *32 = shl5, *64 = shl6
      residue_nn = { 1'b0, modulus_length, 6'h0 };
      residue_length = modulus_length;
      residue_opm_data = modulus_mem_int_rd_data;
    end


  //----------------------------------------------------------------
  // Detects if modulus has been updated and we need to
  // recalculate the residue
  // and we need residue is valid or not.
  //----------------------------------------------------------------
  always @*
    begin : residue_valid_process
      residue_valid_new = residue_valid_reg;

      if (modulus_mem_api_cs & modulus_mem_api_wr)
        residue_valid_new = 1'b0;
      else if ( residue_valid_int_validated == 1'b1)
        residue_valid_new = 1'b1;
    end


  //----------------------------------------------------------------
  // montprod_op_select
  //
  // Select operands used during montprod calculations depending
  // on what operation we want to do.
  //----------------------------------------------------------------
  always @*
    begin : montprod_op_select

      montprod_length          = modulus_length;

      result_mem_int_rd_addr   = montprod_opa_addr;
      message_mem_int_rd_addr  = montprod_opa_addr;
      p_mem_rd0_addr           = montprod_opa_addr;

      residue_mem_montprod_read_addr = montprod_opb_addr;
      p_mem_rd1_addr                 = montprod_opb_addr;

      montprod_opm_data = modulus_mem_int_rd_data;
      case (montprod_select_reg)
        MONTPROD_SELECT_ONE_NR:
          begin
            montprod_opa_data  = one_reg;
            montprod_opb_data  = residue_mem_montprod_read_data;
          end

        MONTPROD_SELECT_X_NR:
          begin
            montprod_opa_data  = message_mem_int_rd_data;
            montprod_opb_data  = residue_mem_montprod_read_data;
          end

        MONTPROD_SELECT_Z_P:
          begin
            montprod_opa_data  = result_mem_int_rd_data;
            montprod_opb_data  = p_mem_rd1_data;
          end

        MONTPROD_SELECT_P_P:
          begin
            montprod_opa_data  = p_mem_rd0_data;
            montprod_opb_data  = p_mem_rd1_data;
          end

        MONTPROD_SELECT_Z_ONE:
          begin
            montprod_opa_data  = result_mem_int_rd_data;
            montprod_opb_data  = b_one_reg;
          end

        default:
          begin
            montprod_opa_data  = 32'h00000000;
            montprod_opb_data  = 32'h00000000;
          end
      endcase // case (montprod_selcect_reg)
    end


  //----------------------------------------------------------------
  // memory write mux
  //
  // Direct memory write signals to correct memory.
  //----------------------------------------------------------------
  always @*
    begin : memory_write_process
      result_mem_int_wr_addr = montprod_result_addr;
      result_mem_int_wr_data = montprod_result_data;
      result_mem_int_we      = 1'b0;

      p_mem_wr_addr = montprod_result_addr;
      p_mem_wr_data = montprod_result_data;
      p_mem_we      = 1'b0;

      case (montprod_dest_reg)
        MONTPROD_DEST_Z:
          result_mem_int_we = montprod_result_we;
        MONTPROD_DEST_P:
          p_mem_we = montprod_result_we;
        default:
          begin
          end
      endcase

      // inhibit Z=Z*P when ei = 0
      if (modexp_ctrl_reg == CTRL_ITERATE_Z_P)
        result_mem_int_we = result_mem_int_we & ei_reg;
    end


  //----------------------------------------------------------------
  // loop_counter
  //
  // Calculate the loop counter and related variables.
  //----------------------------------------------------------------
  always @*
    begin : loop_counters_process
      loop_counter_new = 13'b0;
      loop_counter_we  = 1'b0;

      if (loop_counter_reg == {exponent_length_m1, 5'b11111})
        last_iteration = 1'b1;
      else
        last_iteration = 1'b0;

      case (modexp_ctrl_reg)
        CTRL_CALCULATE_P0:
          begin
            loop_counter_new = 13'b0;
            loop_counter_we  = 1'b1;
          end

        CTRL_ITERATE_END:
          begin
            loop_counter_new = loop_counter_reg + 1'b1;
            loop_counter_we  = 1'b1;
          end

        default:
          begin
          end
      endcase
    end


  //----------------------------------------------------------------
  // exponent
  //
  // Reads the exponent.
  //----------------------------------------------------------------
  always @*
    begin : exponent_process
      // Accessing new instead of reg - pick up update at
      // CTRL_ITERATE_NEW to remove a pipeline stall.
      E_word_index  = exponent_length_m1 - loop_counter_new[ 12 : 5 ];

      E_bit_index   = loop_counter_reg[ 04 : 0 ];

      exponent_mem_int_rd_addr = E_word_index;

      ei_new = exponent_mem_int_rd_data[ E_bit_index ];

      if (modexp_ctrl_reg == CTRL_ITERATE)
        ei_we = 1'b1;
      else
        ei_we = 1'b0;
    end


  //----------------------------------------------------------------
  // modexp_ctrl
  //
  // Control FSM logic needed to perform the modexp operation.
  //----------------------------------------------------------------
  always @*
    begin
      ready_new           = 1'b0;
      ready_we            = 1'b0;
      montprod_select_new = MONTPROD_SELECT_ONE_NR;
      montprod_select_we  = 0;
      montprod_dest_new   = MONTPROD_DEST_NOWHERE;
      montprod_dest_we    = 0;
      montprod_calc       = 0;
      modexp_ctrl_new     = CTRL_IDLE;
      modexp_ctrl_we      = 1'b0;
      cycle_ctr_start     = 1'b0;
      cycle_ctr_stop      = 1'b0;

      residue_calculate = 1'b0;

      residue_valid_int_validated = 1'b0;

      case (modexp_ctrl_reg)
        CTRL_IDLE:
          begin
            if (start)
              begin
                ready_new       = 1'b0;
                ready_we        = 1'b1;
                cycle_ctr_start = 1'b1;

                if (residue_valid_reg)
                  begin
                    //residue has alrady been calculated, start with MONTPROD( 1, Nr, MODULUS )
                    montprod_select_new = MONTPROD_SELECT_ONE_NR;
                    montprod_select_we  = 1;
                    montprod_dest_new   = MONTPROD_DEST_Z;
                    montprod_dest_we    = 1;
                    montprod_calc       = 1;
                    modexp_ctrl_new     = CTRL_CALCULATE_Z0;
                    modexp_ctrl_we      = 1;
                  end
                else
                  begin
                    //modulus has been written and residue (Nr) must be calculated
                    modexp_ctrl_new = CTRL_RESIDUE;
                    modexp_ctrl_we  = 1;
                    residue_calculate = 1'b1;
                  end
              end
          end

        CTRL_RESIDUE:
          begin
            if (residue_ready)
              begin
                montprod_select_new = MONTPROD_SELECT_ONE_NR;
                montprod_select_we  = 1;
                montprod_dest_new   = MONTPROD_DEST_Z;
                montprod_dest_we    = 1;
                montprod_calc       = 1;
                modexp_ctrl_new     = CTRL_CALCULATE_Z0;
                modexp_ctrl_we      = 1;
                residue_valid_int_validated = 1'b1; //update registers telling residue is valid
              end
          end

        CTRL_CALCULATE_Z0:
          begin
            if (montprod_ready)
              begin
                montprod_select_new = MONTPROD_SELECT_X_NR;
                montprod_select_we  = 1;
                montprod_dest_new   = MONTPROD_DEST_P;
                montprod_dest_we    = 1;
                montprod_calc       = 1;
                modexp_ctrl_new = CTRL_CALCULATE_P0;
                modexp_ctrl_we  = 1;
              end
          end

        CTRL_CALCULATE_P0:
          begin
            if (montprod_ready == 1'b1)
              begin
                modexp_ctrl_new = CTRL_ITERATE;
                modexp_ctrl_we  = 1;
              end
          end

        CTRL_ITERATE:
          begin
            montprod_select_new = MONTPROD_SELECT_Z_P;
            montprod_select_we  = 1;
            montprod_dest_new   = MONTPROD_DEST_Z;
            montprod_dest_we    = 1;
            montprod_calc       = 1;
            modexp_ctrl_new     = CTRL_ITERATE_Z_P;
            modexp_ctrl_we      = 1;

            if (ei_new == 1'b0 && exponation_mode_reg == EXPONATION_MODE_PUBLIC_FAST)
              begin
                //Skip the fake montgomery calculation, exponation_mode_reg optimizing for speed not blinding.
                montprod_select_new = MONTPROD_SELECT_P_P;
                montprod_dest_new   = MONTPROD_DEST_P;
                modexp_ctrl_new     = CTRL_ITERATE_P_P;
              end
          end

        CTRL_ITERATE_Z_P:
            if (montprod_ready)
              begin
                montprod_select_new = MONTPROD_SELECT_P_P;
                montprod_select_we  = 1;
                montprod_dest_new   = MONTPROD_DEST_P;
                montprod_dest_we    = 1;
                montprod_calc       = 1;
                modexp_ctrl_new     = CTRL_ITERATE_P_P;
                modexp_ctrl_we      = 1;
              end

        CTRL_ITERATE_P_P:
            if (montprod_ready == 1'b1)
              begin
                modexp_ctrl_new = CTRL_ITERATE_END;
                modexp_ctrl_we  = 1;
              end

        CTRL_ITERATE_END:
          begin
            if (!last_iteration)
              begin
                modexp_ctrl_new = CTRL_ITERATE;
                modexp_ctrl_we  = 1;
              end
            else
              begin
                montprod_select_new = MONTPROD_SELECT_Z_ONE;
                montprod_select_we  = 1;
                montprod_dest_new   = MONTPROD_DEST_Z;
                montprod_dest_we    = 1;
                montprod_calc       = 1;
                modexp_ctrl_new     = CTRL_CALCULATE_ZN;
                modexp_ctrl_we      = 1;
              end
          end

        CTRL_CALCULATE_ZN:
          begin
            if (montprod_ready)
              begin
                modexp_ctrl_new = CTRL_DONE;
                modexp_ctrl_we  = 1;
              end
          end

        CTRL_DONE:
          begin
            cycle_ctr_stop  = 1'b1;
            ready_new       = 1'b1;
            ready_we        = 1'b1;
            modexp_ctrl_new = CTRL_IDLE;
            modexp_ctrl_we  = 1;
          end

        default:
          begin
          end

      endcase // case (modexp_ctrl_reg)
    end

endmodule // modexp_core

//======================================================================
// EOF modexp_core.v
//======================================================================
