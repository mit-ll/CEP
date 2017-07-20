module gps_test ();

reg clk = 0;

always
     #10 clk <= ~clk;

reg rst_o = 0;
initial begin
   #1;
   repeat (32) @(negedge clk)
      rst_o <= 0;
   repeat (32) @(negedge clk)
      rst_o <= 1;
   repeat (32) @(negedge clk)
      rst_o <= 0;
end

wire [12:0] ca_code;
wire [127:0] p_code, py_code;
wire py_code_valid;
gps gps(clk,~clk,rst_o,6'd12,1'b1,ca_code,p_code,py_code,py_code_valid);

endmodule
