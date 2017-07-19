module gps_clkgen_test (fast, slow, rst);

output fast, slow, rst;

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

gps_clkgen dut(clk, ~clk, rst_o, fast, slow, rst);

endmodule
