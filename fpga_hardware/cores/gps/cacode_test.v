
module cacode_test (chip);

output chip;

reg clk = 0;
reg rst = 0;
reg [5:0] prn_num = 1;
reg enb = 0;



always
     #10 clk <= ~clk;

initial begin
   #1;
   repeat (32) @(negedge clk)
      rst <= 0;
   repeat (32) @(negedge clk)
      rst <= 1;
   repeat (32) @(negedge clk)
      rst <= 0;
end


initial begin
   #1;
   repeat (100) @(negedge clk)
      enb <= 0;
   repeat (1200) @(negedge clk)
      enb <= 1;
   repeat (100) @(negedge clk)
      enb <= 0;
end



cacode dut(clk, rst, prn_num, enb, chip);


endmodule
