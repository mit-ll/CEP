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

wire ca_code, p_code, py_code, py_code_valid;
gps gps(clk,~clk,rst_o,6'd12,ca_code,p_code,py_code,py_code_valid);

endmodule
