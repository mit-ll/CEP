module gps_test ();

reg clk = 0;

always
     #10 clk <= ~clk;

reg start = 0;
reg rst_o = 0;
initial begin
   #1;
   repeat (32) @(negedge clk)
      rst_o <= 0;
   repeat (32) @(negedge clk)
      rst_o <= 1;
   repeat (32) @(negedge clk)
      rst_o <= 0;
   repeat (32) @(negedge clk)
      start <= 1;
end

wire [12:0] ca_code;
wire [127:0] p_code, l_code;
wire l_code_valid;
gps gps(clk,~clk,rst_o,6'd12,start,ca_code,p_code,l_code,l_code_valid);

endmodule
