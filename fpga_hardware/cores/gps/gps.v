// From the GPS transmitter's perspective
module gps(
   sys_clk_in_p,
   sys_clk_in_n,
   sync_rst_in,
   sv_num,
   ca_code,
   p_code,
   py_code,
   py_code_valid
);

   input  sys_clk_in_p;
   input  sys_clk_in_n;
   input  sync_rst_in;
   input [5:0] sv_num;
   output ca_code;
   output p_code;
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

// Generate C/A code
cacode ca(
   gps_clk_slow,
   rst_combined,
   sv_num,
   1'b1,
   ca_code
);

// Generate P-code
pcode p(
    gps_clk_fast,
    rst_combined,
    1'b1,
    sv_num,
    p_code
);

// Save 128 p-code bits, encrypt and send as py_code
reg [6:0] p_bit_count;
reg [127:0] p_pt;
always @(posedge gps_clk_fast)begin
   if(rst_combined)begin
      p_bit_count <= 0;
      p_pt <= 128'h0;
   end
   else begin
      p_bit_count <= p_bit_count + 1;
      p_pt <= {p_pt [126:0], p_code}; 
   end
end

wire encrypt = p_bit_count == 0;

aes_192 aes (
   gps_clk_fast,
   encrypt,
   p_pt,
   192'hAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
   py_code,
   py_code_valid
);

endmodule
