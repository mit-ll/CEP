//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_sha256.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based SHA256 core
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

module tb_axi4lite_sha256;

    logic                   clk = 0;
    logic                   rst = 1;
    logic                   done = 0;
    logic                   pass = 1;

    parameter number_of_test_vectors = 3;

    string input_messages [number_of_test_vectors - 1 : 0] = '{
        "abc",
        "a",
        ""
    };

    logic [SHA256_HASH_BITS - 1:0] expected_hashs [number_of_test_vectors - 1 : 0] = '{
        256'hBA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD,
        256'hCA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB,
        256'hE3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855
    };

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
    sha256_top_axi4lite sha256_top_axi4lite_inst (
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
    // A simple function to compare two strings
    function int compareHash (
        input logic [SHA256_HASH_BITS - 1:0] received_hash,
        input logic [SHA256_HASH_BITS - 1:0] expected_hash
    );
        if (received_hash !== expected_hash) begin
            $error("compareHash: received = 0x%64h, expected = 0x%64h", received_hash, expected_hash);
            return 0;
        end else
            return 1;
    endfunction

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

    // Perform the test
    initial begin
        automatic logic [AW-1:0]                    addr;
        automatic logic [DW-1:0]                    data;
        automatic logic [DW/8-1:0]                  strb;
        automatic axi_pkg::resp_t                   resp;
        automatic logic [SHA256_HASH_BITS - 1:0]    hash;

        // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Loop through all the test vectors
        for (int i = 0; i < number_of_test_vectors; i++) begin
            
            // Perform a hash
            axi_lite_drv.hashString_sha256(input_messages[i], hash);

            // Compare the hash
            if (!compareHash(hash, expected_hashs[i]))
                pass = 0;
            else
                `ifdef DEBUG1
                    $display("DEBUG1: tb_axi_lite_sha256: Hash passed = 0x%64h", hash);
                `endif
                ;
        end

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
         
    end // end of Perform the test

endmodule
