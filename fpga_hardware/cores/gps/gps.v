// From the GPS transmitter's perspective
module gps(
   sys_clk_in_p,
   sys_clk_in_n,
   sync_rst_in,
   sv_num,
   startRound,
   ca_code,
   p_code,
   py_code,
   py_code_valid
);

   input  sys_clk_in_p;
   input  sys_clk_in_n;
   input  sync_rst_in;
   input [5:0] sv_num;
   input startRound;
   output reg [12:0] ca_code;
   output [127:0] p_code;
   output [127:0] py_code;
   output py_code_valid;

// Generate the clocks for the C/A code and P-code generators
// Assumes 200 MHz input clock
wire gps_clk_fast;
wire gps_clk_slow;
wire gps_clk_rst;
gps_clkgen gps_clk(
   sys_clk_in_p,
   sys_clk_in_n,
   sync_rst_in,
   gps_clk_fast,
   gps_clk_slow,
   gps_clk_rst
);

// Combine SoC and clkgen resets
wire rst_combined = sync_rst_in | gps_clk_rst;

// Control code generators
reg code_gen_en;
always @(posedge gps_clk_fast)begin
   if(rst_combined | startRound)begin
      code_gen_en <= startRound;
   end
   else if(p_bit_count == 8'd128)begin
      code_gen_en <= 1'b0;
   end
end

// Generate C/A code
wire ca_code_bit;
cacode ca(
   gps_clk_slow,
   rst_combined,
   sv_num,
   code_gen_en,
   ca_code_bit
);

// Save 13 ca-code bits
reg [3:0] ca_bit_count;
always @(posedge gps_clk_slow)begin
   if(rst_combined | startRound)begin
      ca_bit_count <= 0;
      ca_code <= 13'h0;
   end
   else begin
      if(ca_bit_count == 4'd13)begin
         ;
      end
      else begin
         ca_bit_count <= ca_bit_count + 1;
         ca_code <= {ca_code[11:0], ca_code_bit};
      end
   end
end

// Generate P-code
wire p_code_bit;
pcode p(
    gps_clk_fast,
    rst_combined,
    code_gen_en,
    sv_num,
    p_code_bit
);

// Save 128 p-code bits, encrypt and send as py_code
reg [7:0] p_bit_count;
reg [127:0] p_pt;
always @(posedge gps_clk_fast)begin
   if(rst_combined | startRound)begin
      p_bit_count <= 0;
      p_pt <= 128'h0;
   end
   else begin
      if(p_bit_count == 8'd128)begin
         ;
      end
      else begin
         p_bit_count <= p_bit_count + 1;
         p_pt <= {p_pt [126:0], p_code_bit};
      end
   end
end

assign p_code = p_pt;

reg encrypt;
always @(posedge gps_clk_fast)begin
   encrypt <= 0;
   if(p_bit_count == 8'd127)
      encrypt <= 1'b1;
end

aes_192 aes (
   gps_clk_fast,
   encrypt,
   p_pt,
   192'hAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
   py_code,
   py_code_valid
);

endmodule
