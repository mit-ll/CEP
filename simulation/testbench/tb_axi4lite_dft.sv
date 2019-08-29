//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : tb_axi4lite_dft.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based DFT cores
//
// Derived from : https://github.com/pulp-platform/axi.git
// 
// Licensing info can be found in the root directory of the CEP in licenseLog.txt
//

`include "axi/assign.svh"

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

import axi_test_extended::*;

module tb_axi4lite_dft;

    parameter [15:0] exp_1[0:31][0:3] = '{
        {16'h0fc0, 16'h1000, 16'hfaa3, 16'h04cf},
        {16'hfd33, 16'h0246, 16'hfe0b, 16'h0165},
        {16'hfe7d, 16'h0100, 16'hfec0, 16'h00b9},
        {16'hfeeb, 16'h008e, 16'hff08, 16'h0070},
        {16'hff25, 16'h005a, 16'hff39, 16'h0045},
        {16'hff49, 16'h0035, 16'hff55, 16'h0025},
        {16'hff60, 16'h001e, 16'hff6b, 16'h0013},
        {16'hff70, 16'h000c, 16'hff79, 16'h0004},
        {16'hff80, 16'h0000, 16'hff87, 16'hfff8},
        {16'hff8c, 16'hfff4, 16'hff93, 16'hffee},
        {16'hff96, 16'hffea, 16'hff9b, 16'hffe5},
        {16'hff9f, 16'hffe0, 16'hffa3, 16'hffdc},
        {16'hffa6, 16'hffda, 16'hffab, 16'hffd7},
        {16'hffae, 16'hffd3, 16'hffb2, 16'hffcf},
        {16'hffb3, 16'hffcc, 16'hffb8, 16'hffc9},
        {16'hffba, 16'hffc5, 16'hffbd, 16'hffc2},
        {16'hffc0, 16'hffc0, 16'hffc3, 16'hffbd},
        {16'hffc7, 16'hffba, 16'hffcb, 16'hffb7},
        {16'hffcd, 16'hffb4, 16'hffd2, 16'hffb1},
        {16'hffd5, 16'hffae, 16'hffd8, 16'hffa8},
        {16'hffdb, 16'hffa6, 16'hffdf, 16'hffa3},
        {16'hffe3, 16'hff9f, 16'hffe9, 16'hff9b},
        {16'hffec, 16'hff96, 16'hfff1, 16'hff93},
        {16'hfff6, 16'hff8c, 16'hfffb, 16'hff86},
        {16'h0000, 16'hff80, 16'h0007, 16'hff7c},
        {16'h000e, 16'hff74, 16'h0017, 16'hff6e},
        {16'h0020, 16'hff62, 16'h002b, 16'hff59},
        {16'h0039, 16'hff4c, 16'h0049, 16'hff3c},
        {16'h005a, 16'hff26, 16'h0071, 16'hff11},
        {16'h0092, 16'hfef1, 16'h00c0, 16'hfec9},
        {16'h0101, 16'hfe80, 16'h016c, 16'hfe19},
        {16'h0248, 16'hfd3b, 16'h04d3, 16'hfab4}
    };

    parameter [15:0] exp_2[0:31][0:3] = '{
        {16'h2fc0, 16'h3000, 16'hfaa3, 16'h04cf},
        {16'hfd33, 16'h0246, 16'hfe0b, 16'h0165},
        {16'hfe7d, 16'h0100, 16'hfec0, 16'h00b9},
        {16'hfeeb, 16'h008e, 16'hff08, 16'h0070},
        {16'hff25, 16'h005a, 16'hff39, 16'h0045},
        {16'hff49, 16'h0035, 16'hff55, 16'h0025},
        {16'hff60, 16'h001e, 16'hff6b, 16'h0013},
        {16'hff70, 16'h000c, 16'hff79, 16'h0004},
        {16'hff80, 16'h0000, 16'hff87, 16'hfff8},
        {16'hff8c, 16'hfff4, 16'hff93, 16'hffee},
        {16'hff96, 16'hffea, 16'hff9b, 16'hffe5},
        {16'hff9f, 16'hffe0, 16'hffa3, 16'hffdc},
        {16'hffa6, 16'hffda, 16'hffab, 16'hffd7},
        {16'hffae, 16'hffd3, 16'hffb2, 16'hffcf},
        {16'hffb3, 16'hffcc, 16'hffb8, 16'hffc9},
        {16'hffba, 16'hffc5, 16'hffbd, 16'hffc2},
        {16'hffc0, 16'hffc0, 16'hffc3, 16'hffbd},
        {16'hffc7, 16'hffba, 16'hffcb, 16'hffb7},
        {16'hffcd, 16'hffb4, 16'hffd2, 16'hffb1},
        {16'hffd5, 16'hffae, 16'hffd8, 16'hffa8},
        {16'hffdb, 16'hffa6, 16'hffdf, 16'hffa3},
        {16'hffe3, 16'hff9f, 16'hffe9, 16'hff9b},
        {16'hffec, 16'hff96, 16'hfff1, 16'hff93},
        {16'hfff6, 16'hff8c, 16'hfffb, 16'hff86},
        {16'h0000, 16'hff80, 16'h0007, 16'hff7c},
        {16'h000e, 16'hff74, 16'h0017, 16'hff6e},
        {16'h0020, 16'hff62, 16'h002b, 16'hff59},
        {16'h0039, 16'hff4c, 16'h0049, 16'hff3c},
        {16'h005a, 16'hff26, 16'h0071, 16'hff11},
        {16'h0092, 16'hfef1, 16'h00c0, 16'hfec9},
        {16'h0101, 16'hfe80, 16'h016c, 16'hfe19},
        {16'h0248, 16'hfd3b, 16'h04d3, 16'hfab4}
    };

    logic clk = 0;
    logic rst = 1;
    logic done = 0;
    logic pass = 1;

    AXI_LITE_DV #(
        .AXI_ADDR_WIDTH(AW),
        .AXI_DATA_WIDTH(DW)
    ) axi_lite_dv(clk);

    AXI_LITE #(
        .AXI_ADDR_WIDTH(AW),
        .AXI_DATA_WIDTH(DW)
    ) axi_lite();

    `AXI_LITE_ASSIGN(axi_lite, axi_lite_dv);

    // Instantiate the DUT
    dft_top_axi4lite dft_top_axi4lite_inst (
        .clk_i          ( clk               ),
        .rst_ni         ( rst               ),

        .o_axi_awready  ( axi_lite.aw_ready ),
        .i_axi_awaddr   ( axi_lite.aw_addr  ), 
        .i_axi_awcache  ( 4'b0000           ), 
        .i_axi_awprot   ( 3'b000            ), 
        .i_axi_awvalid  ( axi_lite.aw_valid ),

        .o_axi_wready   ( axi_lite.w_ready  ), 
        .i_axi_wdata    ( axi_lite.w_data   ), 
        .i_axi_wstrb    ( axi_lite.w_strb   ), 
        .i_axi_wvalid   ( axi_lite.w_valid  ),

        .o_axi_bresp    ( axi_lite.b_resp   ), 
        .o_axi_bvalid   ( axi_lite.b_valid  ), 
        .i_axi_bready   ( axi_lite.b_ready  ),

        .o_axi_arready  ( axi_lite.ar_ready ),
        .i_axi_araddr   ( axi_lite.ar_addr  ),
        .i_axi_arcache  ( 4'b0000           ),
        .i_axi_arprot   ( 3'b000            ),
        .i_axi_arvalid  ( axi_lite.ar_valid ),

        .o_axi_rresp    ( axi_lite.r_resp   ),
        .o_axi_rvalid   ( axi_lite.r_valid  ),
        .o_axi_rdata    ( axi_lite.r_data   ),
        .i_axi_rready   ( axi_lite.r_ready  )
    );

    // Create a new AXI4 lite object
    axi_test_extended::axi_lite_driver_plus #(.AW(AW), .DW(DW)) axi_lite_drv = new(axi_lite_dv);

    // Reset the Device under Test
    task reset_dut();
        repeat (4) #tCK;
        rst <= 0;
        repeat (4) #tCK;
        rst <= 1;
        #tCK;
    endtask : reset_dut

    // Clock is always running
    initial begin
        while (!done) begin
            clk <= 1;
            #(tCK/2);
            clk <= 0;
            #(tCK/2);
        end
        $stop;
    end

    initial begin
        automatic logic [AW-1:0]    addr;
        automatic logic [DW-1:0]    data;
        automatic logic [DW/8-1:0]  strb;
        automatic axi_pkg::resp_t   resp;

        automatic logic [15:0]      y_results[0:3];

         // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Set the DFT inputs
        for (int i = 0; i < 2; i++) begin
            // The 64 complex data points enter the system over 32 cycles
            for (int j = 0; j < 32; j++) begin
                case (i)
                    0   : axi_lite_drv.dft_setX(j, j * 4, j * 4 + 1, j * 4 + 2, j * 4 + 3);
                    1   : axi_lite_drv.dft_setX(j, 128 + j * 4, 128 + j * 4 + 1, 128 + j * 4 + 2, 128 + j * 4 + 3);
                endcase
            end // for (int j = 0; j < 32; j++)

            // Instruct the core to start
            axi_lite_drv.axi_lite_slave_write(  .addr(32'(DFT_START)),
                                                .data(32'h0000_0001),
                                                .strb('1),
                                                .resp(resp));
            axi_lite_drv.axi_lite_slave_write(  .addr(32'(DFT_START)),
                                                .data(32'h0000_0000),
                                                .strb('1),
                                                .resp(resp));
            
            // Wait for valid output
            do begin
                axi_lite_drv.axi_lite_slave_read(   .addr(32'(DFT_DONE)), 
                                                    .data(data), 
                                                    .resp(resp));
            end 
            while (data[0] == 1'b0);

            // Get and compare the output
            for (int j =0; j < 32; j++) begin
                axi_lite_drv.dft_getY(j, y_results[0], y_results[1], y_results[2], y_results[3]);
                case (i)
                    0   :
                        if (    exp_1[j][0] !== y_results[0] &&
                                exp_1[j][1] !== y_results[1] &&
                                exp_1[j][2] !== y_results[2] &&
                                exp_1[j][3] !== y_results[3]) begin
                            $error("tb_axi_lite_dft: Bad DFT result exp/act - index = %0d (%4h/%4h) (%4h/%4h) (%4h/%4h) (%4h/%4h)",
                                i, exp_1[j][0], y_results[0], exp_1[j][1], y_results[1], exp_1[j][2], y_results[2], exp_1[j][3], y_results[3]);
                            pass = 0;
                        end
                    1   : 
                        if (    exp_2[j][0] !== y_results[0] &&
                                exp_2[j][1] !== y_results[1] &&
                                exp_2[j][2] !== y_results[2] &&
                                exp_2[j][3] !== y_results[3]) begin
                            $error("tb_axi_lite_dft: Bad DFT result exp/act - index = %0d (%4h/%4h) (%4h/%4h) (%4h/%4h) (%4h/%4h)",
                                i, exp_2[j][0], y_results[0], exp_2[j][1], y_results[1], exp_2[j][2], y_results[2], exp_2[j][3], y_results[3]);
                            pass = 0;
                        end
                endcase
            end // for (int j =0; j < 32; j++)
        end // for (int i = 0; i < 2; i++)  

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Display test results
        if (pass) begin
            $display("");
            $display("===================================================================================");
            $display("                             ALL TEST VECTORS PASSED");
            $display("===================================================================================");
            $display("");
        end else begin
            $display("");
            $display("===================================================================================");
            $error  ("                                  TEST FAILED");
            $display("===================================================================================");
            $display("");
        end

        // Tell the simulation to complete
        done = 1;
         
    end // end of perform the test

endmodule
