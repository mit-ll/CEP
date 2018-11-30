//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : tb_orpsoc_top.sv
// Project      : Common Evaluation Platform (CEP)
// Description  : Top-level testbench file for the AXI4-Lite / MOR1KX variant of the CEP
// Notes        : Core licensing information may be found in licenseLog.txt
//

//////////////////////////////////////////////////////////////////////
///                                                               ////
/// ORPSoC VC707 testbench                                        ////
///                                                               ////
/// Instantiate ORPSoC, monitors, provide stimulus                ////
///                                                               ////
/// Julius Baxter, julius@opencores.org                           ////
///                                                               ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2009, 2010 Authors and OPENCORES.ORG           ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////

// Used for the various levels of debug (higher levels define lower levels)
`ifdef DEBUG5
    `define DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG2
    `define DEBUG1
`endif

`include "timescale.v"
`include "orpsoc-defines.sv"
`include "mor1kx-defines.v"
`include "test-defines.v"

module tb_orpsoc_top;

// Clock and reset signal registers
reg     clk     = 0;
reg     rst_n   = 1; // Active LOW

always
    #((`BOARD_CLOCK_PERIOD)/2) clk <= ~clk;

wire    clk_n, clk_p;
assign  clk_p = clk;
assign  clk_n = ~clk;

// Reset, ACTIVE LOW
initial
    begin
        #1;
        repeat (32) @(negedge clk)
            rst_n <= 1;
        repeat (32) @(negedge clk)
            rst_n <= 0;
        repeat (32) @(negedge clk)
            rst_n <= 1;
    end

wire        uart_stx_pad_o;
wire        uart_srx_pad_i;
wire        uart_cts_pad_i;
wire        uart_rts_pad_o;

wire        button_W;
wire        button_N;
wire        button_E;
wire        button_S;

// Device Under Test - The CEP
orpsoc_top orpsoc_top_inst (
    .sys_clk_in_p       (clk_p),
    .sys_clk_in_n       (clk_n),

    .uart_stx_pad_o     (uart_stx_pad_o),
    .uart_srx_pad_i     (uart_srx_pad_i),
    .uart_rts_pad_o     (uart_rts_pad_o),
    .uart_cts_pad_i     (uart_cts_pad_i),

    .button_W           (button_W),
    .button_N           (button_N),
    .button_E           (button_E),
    .button_S           (button_S),
    .GPIO_LED           (),

`ifdef RESET_HIGH
    .rst_pad_i          (!rst_n)      
`else
    .rst_n_pad_i        (rst_n)      
`endif     
);

//
// Instantiate the mor1k monitor and traceport monitor
//
mor1kx_monitor #(.LOG_DIR(`TB_LOG_OUT)) monitor();

`ifdef VCD
reg vcd_go = 0;
always @(vcd_go)
    begin : fjfk
        //    integer r, t;
        // `ifdef VCD_DELAY
        // #(`VCD_DELAY);
        /*for(r = 0; r < 1000; r = r + 1)begin
          for(t = 0; t < 10; t = t + 1) begin
            #(331900000);
          end
        end*/
        // `endif

        // Delay by x insns
`ifdef VCD_DELAY_INSNS

        #10; // Delay until after the value becomes valid
        while (monitor.insns < `VCD_DELAY_INSNS)
            @(posedge clk);
`endif

`ifdef SIMULATOR_MODELSIM
        // Modelsim can GZip VCDs on the fly if given in the suffix
`define VCD_SUFFIX   ".vcd.gz"

`else
`define VCD_SUFFIX   ".vcd"
 `endif
        $display("* VCD in %s\n", {"",`TEST_NAME_STRING,`VCD_SUFFIX});
        $dumpfile({"",`TEST_NAME_STRING,`VCD_SUFFIX});
        `ifndef VCD_DEPTH
 `endif
                $dumpvars(`VCD_DEPTH, `VCD_PATH);

        /*for(r = 0; r < 1000; r = r + 1)begin
          for(t = 0; t < 10; t = t + 1) begin
            #(2000000);
          end
        end
         $finish;
        */
    end
`endif //  `ifdef VCD

initial
    begin
        $display("\n* Starting simulation of design RTL.\n* Test: %s\n",
                 `TEST_NAME_STRING );

`ifdef VCD

        vcd_go = 1;
`endif

    end // initial begin

`ifdef END_TIME
initial
    begin
        #(`END_TIME);
        $display("* Finish simulation due to END_TIME being set at %t", $time);
        $finish;
    end
`endif

`ifdef END_INSNS
initial
    begin
        #10
         while (monitor.insns < `END_INSNS)
             @(posedge clk);
        $display("* Finish simulation due to END_INSNS count (%d) reached at %t",
                 `END_INSNS, $time);
        $finish;
    end
`endif

//
// UART0 decoder
//
uart_decoder
    #(
        .uart_baudrate_period_ns(8680) // 115200 baud = period 8.68uS
    )
    uart_decoder
    (
        .clk(clk),
        .uart_tx(uart_stx_pad_o)
    );

// Loopback UART lines
assign uart_srx_pad_i = uart_stx_pad_o;
assign uart_cts_pad_i = 1'b0;

// Assign the button inputs
assign button_N = 1'b0;
assign button_E = 1'b0;
assign button_S = 1'b0;
assign button_W = 1'b0;

endmodule // tb_orpsoc_top

