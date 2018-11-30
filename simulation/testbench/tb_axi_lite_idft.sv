//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_idft.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based IDFT cores
//
// Derived from : https://github.com/pulp-platform/axi.git
// 
// Licensing info can be found in the root directory of the CEP in licenseLog.txt
//

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

module tb_axi_lite_idft;


    parameter [15:0] exp_1[0:31][0:3] = '{
        {16'h0fc0, 16'h1000, 16'h04de, 16'hfa9e},
        {16'h024d, 16'hfd31, 16'h0173, 16'hfe06},
        {16'h0103, 16'hfe7c, 16'h00c0, 16'hfeb9},
        {16'h0094, 16'hfee8, 16'h0078, 16'hff08},
        {16'h005b, 16'hff25, 16'h0047, 16'hff36},
        {16'h0037, 16'hff45, 16'h0028, 16'hff51},
        {16'h0020, 16'hff5e, 16'h0016, 16'hff67},
        {16'h0010, 16'hff6f, 16'h0008, 16'hff76},
        {16'h0000, 16'hff80, 16'hfff8, 16'hff85},
        {16'hfff4, 16'hff8b, 16'hffef, 16'hff8f},
        {16'hffea, 16'hff94, 16'hffe5, 16'hff98},
        {16'hffe0, 16'hff9d, 16'hffdd, 16'hff9f},
        {16'hffda, 16'hffa5, 16'hffd5, 16'hffa8},
        {16'hffd3, 16'hffac, 16'hffcf, 16'hffaf},
        {16'hffcd, 16'hffb2, 16'hffc8, 16'hffb6},
        {16'hffc6, 16'hffb8, 16'hffc2, 16'hffbc},
        {16'hffc0, 16'hffc0, 16'hffbc, 16'hffc4},
        {16'hffb9, 16'hffc7, 16'hffb5, 16'hffca},
        {16'hffb3, 16'hffce, 16'hffae, 16'hffd1},
        {16'hffac, 16'hffd4, 16'hffa8, 16'hffd6},
        {16'hffa5, 16'hffdb, 16'hffa1, 16'hffe0},
        {16'hff9d, 16'hffe3, 16'hff98, 16'hffe9},
        {16'hff94, 16'hffec, 16'hff90, 16'hfff1},
        {16'hff8a, 16'hfff5, 16'hff84, 16'hfffa},
        {16'hff80, 16'h0000, 16'hff7a, 16'h0009},
        {16'hff72, 16'h0011, 16'hff69, 16'h0019},
        {16'hff60, 16'h0022, 16'hff55, 16'h002e},
        {16'hff48, 16'h003b, 16'hff37, 16'h004b},
        {16'hff26, 16'h005b, 16'hff0f, 16'h007a},
        {16'hfeed, 16'h0098, 16'hfec1, 16'h00c7},
        {16'hfe7f, 16'h0104, 16'hfe12, 16'h017a},
        {16'hfd38, 16'h0250, 16'hfaae, 16'h04e4}
    };

    parameter [15:0] exp_2[0:31][0:3] = '{
        {16'h2fc0, 16'h3000, 16'h04de, 16'hfa9e},
        {16'h024d, 16'hfd31, 16'h0173, 16'hfe06},
        {16'h0103, 16'hfe7c, 16'h00c0, 16'hfeb9},
        {16'h0094, 16'hfee8, 16'h0078, 16'hff08},
        {16'h005b, 16'hff25, 16'h0047, 16'hff36},
        {16'h0037, 16'hff45, 16'h0028, 16'hff51},
        {16'h0020, 16'hff5e, 16'h0016, 16'hff67},
        {16'h0010, 16'hff6f, 16'h0008, 16'hff76},
        {16'h0000, 16'hff80, 16'hfff8, 16'hff85},
        {16'hfff4, 16'hff8b, 16'hffef, 16'hff8f},
        {16'hffea, 16'hff94, 16'hffe5, 16'hff98},
        {16'hffe0, 16'hff9d, 16'hffdd, 16'hff9f},
        {16'hffda, 16'hffa5, 16'hffd5, 16'hffa8},
        {16'hffd3, 16'hffac, 16'hffcf, 16'hffaf},
        {16'hffcd, 16'hffb2, 16'hffc8, 16'hffb6},
        {16'hffc6, 16'hffb8, 16'hffc2, 16'hffbc},
        {16'hffc0, 16'hffc0, 16'hffbc, 16'hffc4},
        {16'hffb9, 16'hffc7, 16'hffb5, 16'hffca},
        {16'hffb3, 16'hffce, 16'hffae, 16'hffd1},
        {16'hffac, 16'hffd4, 16'hffa8, 16'hffd6},
        {16'hffa5, 16'hffdb, 16'hffa1, 16'hffe0},
        {16'hff9d, 16'hffe3, 16'hff98, 16'hffe9},
        {16'hff94, 16'hffec, 16'hff90, 16'hfff1},
        {16'hff8a, 16'hfff5, 16'hff84, 16'hfffa},
        {16'hff80, 16'h0000, 16'hff7a, 16'h0009},
        {16'hff72, 16'h0011, 16'hff69, 16'h0019},
        {16'hff60, 16'h0022, 16'hff55, 16'h002e},
        {16'hff48, 16'h003b, 16'hff37, 16'h004b},
        {16'hff26, 16'h005b, 16'hff0f, 16'h007a},
        {16'hfeed, 16'h0098, 16'hfec1, 16'h00c7},
        {16'hfe7f, 16'h0104, 16'hfe12, 16'h017a},
        {16'hfd38, 16'h0250, 16'hfaae, 16'h04e4}
    };

    logic clk = 0;
    logic rst = 1;
    logic done = 0;
    logic pass = 1;

    AXI_LITE #(
        .AXI_ADDR_WIDTH(AW),
        .AXI_DATA_WIDTH(DW)
    ) axi_lite(clk);

    // Instantiate the DUT
    idft_top_axi4lite idft_top_axi4lite_inst (
        .clk_i  ( clk      ),
        .rst_ni ( rst      ),
        .slave  ( axi_lite )
    );

    // Create a new AXI4 lite object
    axi_test_extended::axi_lite_driver_plus #(.AW(AW), .DW(DW)) axi_lite_drv = new(axi_lite);

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
                    0   : axi_lite_drv.idft_setX(j, j * 4, j * 4 + 1, j * 4 + 2, j * 4 + 3);
                    1   : axi_lite_drv.idft_setX(j, 128 + j * 4, 128 + j * 4 + 1, 128 + j * 4 + 2, 128 + j * 4 + 3);
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
                axi_lite_drv.idft_getY(j, y_results[0], y_results[1], y_results[2], y_results[3]);
                case (i)
                    0   :
                        if (    exp_1[j][0] !== y_results[0] &&
                                exp_1[j][1] !== y_results[1] &&
                                exp_1[j][2] !== y_results[2] &&
                                exp_1[j][3] !== y_results[3]) begin
                            $error("tb_axi_lite_idft: Bad IDFT result exp/act - index = %0d (%4h/%4h) (%4h/%4h) (%4h/%4h) (%4h/%4h)",
                                i, exp_1[j][0], y_results[0], exp_1[j][1], y_results[1], exp_1[j][2], y_results[2], exp_1[j][3], y_results[3]);
                            pass = 0;
                        end
                    1   : 
                        if (    exp_2[j][0] !== y_results[0] &&
                                exp_2[j][1] !== y_results[1] &&
                                exp_2[j][2] !== y_results[2] &&
                                exp_2[j][3] !== y_results[3]) begin
                            $error("tb_axi_lite_idft: Bad IDFT result exp/act - index = %0d (%4h/%4h) (%4h/%4h) (%4h/%4h) (%4h/%4h)",
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
