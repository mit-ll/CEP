//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
/*
 *
 * Clock generation for GPS modules
 * 10.23 MHz for P-code and 1.023 MHz for C/A code
 * 
 */

module gps_clkgen (
           sys_clk_50,
           sync_rst_in,

           gps_clk_fast,
           gps_clk_slow,
           gps_rst
       );

input sys_clk_50;
input  sync_rst_in;
output gps_clk_fast;
output gps_clk_slow;
output gps_rst;


`ifdef SYNTHESIS
wire       clk_fb;
wire       clk_out0_prebufg, clk_out0;
wire       locked_int;

MMCME2_ADV
    #(
        .BANDWIDTH            ("OPTIMIZED"),
        .CLKOUT4_CASCADE      ("FALSE"),
        .COMPENSATION         ("ZHOLD"),
        .STARTUP_WAIT         ("FALSE"),
        .DIVCLK_DIVIDE        (1),
        .CLKFBOUT_MULT_F      (20), // CLKIN = 50 MHz, 50 Mhz * 20 / 100 = 10 MHz 
        .CLKFBOUT_PHASE       (0.000),
        .CLKFBOUT_USE_FINE_PS ("FALSE"),
        .CLKOUT0_DIVIDE_F     (100),
        .CLKOUT0_PHASE        (0.000),
        .CLKOUT0_DUTY_CYCLE   (0.500),
        .CLKOUT0_USE_FINE_PS  ("FALSE"),
        .CLKIN1_PERIOD        (20.000))
    mmcm_adv_inst (
        .CLKFBOUT            (clk_fb),
        .CLKOUT0             (clk_out0_prebufg),
        .CLKFBIN             (clk_fb),
        .CLKIN1              (sys_clk_50),
        .CLKIN2              (1'b0),
        .CLKINSEL            (1'b1),
        .DADDR               (7'h0),
        .DCLK                (1'b0),
        .DEN                 (1'b0),
        .DI                  (16'h0),
        .DWE                 (1'b0),
        .PSCLK               (1'b0),
        .PSEN                (1'b0),
        .PSINCDEC            (1'b0),
        .LOCKED              (locked_int),
        .PWRDWN              (1'b0),
        .RST                 (1'b0)
    );

  BUFG gps_clk_fast_buf (
     .O (gps_clk_fast),
     .I (clk_out0_prebufg));

//assign gps_clk_fast = clk_out0;
assign gps_rst = sync_rst_in | ~locked_int;


// Generate slow clock using CE
BUFGCE gps_clk_slow_buf(
           .O   (gps_clk_slow),
           .CE  (enable_slow_clk),
           .I   (clk_out0_prebufg)
       );

reg [2:0] count = 3'h0;
reg enable_slow_clk;
always @(posedge gps_clk_fast)
    begin
        enable_slow_clk <= 1'b0;
        if(count == 3'h4)
            begin
                count <= 3'h0;
                enable_slow_clk <= 1'b1;
            end
        else
            begin
                count <= count + 3'h1;
            end
    end




`else
assign gps_clk_fast = sys_clk_50;

reg [2:0] count = 3'h0;
reg gps_clk_slow_r = 1'b0;
always @(posedge gps_clk_fast)
    begin
        if(count == 3'h4)
            begin
                count <= 3'h0;
                gps_clk_slow_r <= ~gps_clk_slow_r;
            end
        else
            begin
                count <= count + 3'h1;
            end
    end

assign gps_clk_slow = gps_clk_slow_r;
assign gps_rst = sync_rst_in;
`endif

endmodule // gps_clkgen

