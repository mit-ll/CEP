//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      	gps.v
// Program:        	Common Evaluation Platform (CEP)
// Description:    	GPS top-level module
// Note: 			From the GPS transmitter's perspective
//
//************************************************************************
module gps(
           sys_clk,
           sync_rst_in,
           sv_num,
           startRound,
           aes_key,
           pcode_speeds,
           pcode_initializers,
           ca_code,
           p_code,
           l_code,
           l_code_valid
       );
input  sys_clk;
input  sync_rst_in;
input [5:0] sv_num;
input startRound;
input [191:0] aes_key;
input [30:0] pcode_speeds;
input [47:0] pcode_initializers;
output reg [12:0] ca_code;
output [127:0] p_code;
output [127:0] l_code;
output l_code_valid;


wire gps_clk_fast;

// Start of round edge detection
reg startRound_r;
always @(posedge gps_clk_fast)
    begin
        if(sync_rst_in) begin
            startRound_r <= 0;
        end else begin
            startRound_r <= startRound;
        end
    end
wire startRoundPosEdge = ~startRound_r & startRound;


//
// Tony Duong: Added logic to detect a change in *sv_num* number and ca-code and p-code calculation automatically
// to recompute CA-Code
//
reg [5:0] last_sv_num;
wire      sv_changed = last_sv_num != sv_num;
always @(posedge gps_clk_fast)
    begin
        if (sync_rst_in)
            last_sv_num <= 0;
        else
            last_sv_num <= sv_num;
    end


//
// Control code generators
//
reg p_gen_en;
reg ca_gen_en;
reg [7:0] p_bit_count;
reg [3:0] ca_bit_count;
//Get 128 P-Code bits and 13 CA-Code bits. (Real GPS is exactly a 10:1 ratio, this is close enough for here)
always @(posedge gps_clk_fast)
    begin
        if (sync_rst_in)
            p_gen_en <= 1'b0;
        else if(startRoundPosEdge) begin
            p_gen_en <= startRoundPosEdge;
        end else if(p_bit_count == 8'd127) begin //Stop 1 cycle early due to generator latency.
            p_gen_en <= 1'b0;
        end
    end

always @(posedge gps_clk_fast)
    begin
        if (sync_rst_in)
            ca_gen_en <= 1'b0;
        else if(startRoundPosEdge) begin
            ca_gen_en <= startRoundPosEdge;
        end else if(ca_bit_count == 4'd12) begin //Stop 1 cycle early due to generator latency.
            ca_gen_en <= 1'b0;
        end
    end


// Generate C/A code
wire ca_code_bit;
cacode ca(
          .clk         (gps_clk_slow ),
          .rst         (sync_rst_in  ),
          .prn_num     (sv_num       ),
          .prn_changed (sv_changed   ),
          .enb         (ca_gen_en    ),
          .chip_out    (ca_code_bit  )
         );

// Save 13 ca-code bits
always @(posedge gps_clk_slow)
    begin
        if(sync_rst_in) begin
            ca_bit_count <= 4'b0;
            ca_code  <= 13'b0;
        end else if (startRoundPosEdge) begin
            ca_bit_count <= 4'b0;
            ca_code  <= 13'b0;
        end else if(ca_bit_count < 4'd14) begin //Due to latency of CA generator, record 1 extra bit as first bit received is junk.
            ca_bit_count <= ca_bit_count + 1;
            ca_code <= {ca_code[11:0], ca_code_bit};
        end
    end

// Generate P-code
wire p_code_bit;
//verification connections:
wire [11:0] pcode_xn_cnt_speed; //default must be 1.
wire [18:0] pcode_z_cnt_speed; //default must be 1.
wire [11:0] pcode_ini_x1a;     //default must be 12'b001001001000;
wire [11:0] pcode_ini_x1b;     //default must be 12'b010101010100;
wire [11:0] pcode_ini_x2a;     //default must be 12'b100100100101;
wire [11:0] pcode_ini_x2b;     //default must be 12'b010101010100;
assign pcode_xn_cnt_speed = pcode_speeds[11:0];
assign pcode_z_cnt_speed  = pcode_speeds[30:12];
assign pcode_ini_x1a = pcode_initializers[11:0];
assign pcode_ini_x1b = pcode_initializers[23:12];
assign pcode_ini_x2a = pcode_initializers[35:24];
assign pcode_ini_x2b = pcode_initializers[47:36];
pcode p(
        .clk            (gps_clk_fast  ),
        .reset          (sync_rst_in   ),
        .prn_changed    (sv_changed    ),
        .en             (p_gen_en      ),
        .sat            (sv_num        ),
        .preg           (p_code_bit    ),
        .xn_cnt_speed   (pcode_xn_cnt_speed),
        .z_cnt_speed    (pcode_z_cnt_speed),
        .ini_x1a        (pcode_ini_x1a ),
        .ini_x1b        (pcode_ini_x1b ),
        .ini_x2a        (pcode_ini_x2a ),
        .ini_x2b        (pcode_ini_x2b )
      );

// Save 128 p-code bits, encrypt and send as l_code
reg [127:0] p_pt;
always @(posedge gps_clk_fast)
    begin
        if (sync_rst_in) begin
            p_bit_count <= 0;
            p_pt <= 0; 
        end else if(startRoundPosEdge) begin
            p_bit_count <= 0;
            p_pt <= 0;
        end else if ( p_gen_en) begin
            if(p_bit_count < 8'd128) begin
                p_bit_count <= p_bit_count + 1;
                p_pt <= {p_pt [126:0], p_code_bit};
            end
        end
    end


assign p_code = p_pt;

reg encrypt;
always @(posedge gps_clk_fast)
    begin
        if (sync_rst_in) 
            encrypt <= 0;
        else begin
            encrypt <= (p_bit_count == 8'd127); //Run encryption every 128th bit
        end
    end


//
// Todo : finish removing
assign gps_clk_fast = sys_clk;
assign gps_clk_slow = sys_clk;

//
// in order to remove the reset dependency, we need create a mask to mask out the out_valid from aes_192 module
// while computing CA-code and P-code, the valid is only applied when encryt=1
reg l_valid_mask;
wire l_code_valid_i;
assign l_code_valid = l_code_valid_i & l_valid_mask;
//
always @(posedge gps_clk_fast) begin
    if (sync_rst_in)            l_valid_mask <= 0;
    else if (startRoundPosEdge) l_valid_mask <= 0; // clear the mask
    else if (encrypt)           l_valid_mask <= 1; // this happens             
end


aes_192 aes_192_inst (
    .clk        (gps_clk_fast),
    .rst        (sync_rst_in),
    .start      (encrypt),
    .state      (p_pt),
    .key        (aes_key),
    .out        (l_code),
    .out_valid  (l_code_valid_i)
  );

endmodule
