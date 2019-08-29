//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_aes.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based AES cores
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

module tb_axi4lite_aes;

    // AES Test vector(s)
    parameter [31:0]  key1[0:5] = { 32'h2b7e1516, 
                                    32'h28aed2a6, 
                                    32'habf71588, 
                                    32'h09cf4f3c, 
                                    32'h2b7e1516, 
                                    32'h28aed2a6};

    parameter [31:0]  pt1[0:3] = {  32'h3243f6a8, 
                                    32'h885a308d, 
                                    32'h313198a2, 
                                    32'he0370734};

    parameter [31:0]  ct1[0:3] = {  32'h4fcb8db8,
                                    32'h5784a2c1,
                                    32'hbb77db7e,
                                    32'hde3217ac};

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
    aes_top_axi4lite aes_top_axi4lite_inst (
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

         // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Load the Key
        for (int i = 0; i < AES_KEY_WORDS; ++i) begin
            axi_lite_drv.axi_lite_slave_write( .addr(32'(AES_KEY_BASE + (((AES_KEY_WORDS - 1) - i) * BYTES_PER_WORD))),
                                               .data(key1[i]),
                                               .strb('1),
                                               .resp(resp));
        end // end for    
        repeat (4) @(posedge clk);

        // Load the Plaintext
        for (int i = 0; i < AES_BLOCK_WORDS; ++i) begin
            axi_lite_drv.axi_lite_slave_write( .addr(32'(AES_PT_BASE + (((AES_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD))),
                                               .data(pt1[i]),
                                               .strb('1),
                                               .resp(resp));
        end // end for

        // Tell the AES core to start
        axi_lite_drv.axi_lite_slave_write(  .addr(32'(AES_BASE)),
                                            .data(32'h0000_0001),
                                            .strb('1),
                                            .resp(resp));

        // Poll the AES Core to see when it has completed operations
        do begin
            axi_lite_drv.axi_lite_slave_read(   .addr(32'(AES_DONE)), 
                                                .data(data), 
                                                .resp(resp));
        end 
        while (data[0] == 1'b0);

        // Read the Ciphertext and compare the results
        for (int i = 0; i < AES_BLOCK_WORDS; ++i) begin
            axi_lite_drv.axi_lite_slave_read(   .addr(32'(AES_CT_BASE + (i * BYTES_PER_WORD))), 
                                                .data(data), 
                                                .resp(resp));

            if (data !== ct1[i]) begin
                $error("Unexpected Ciphertext - word #[%0d], expected %h, got %h", i, ct1[i], data);
                pass = 0;
            end // if (data != ct1[i])

        end // end for

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
