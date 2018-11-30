//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_md5.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based MD5 core
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

module tb_axi_lite_md5;

    logic                   clk = 0;
    logic                   rst = 1;
    logic                   done = 0;
    logic                   pass = 1;

    parameter number_of_test_vectors = 3;

    string input_messages [number_of_test_vectors - 1 : 0] = '{
        "The quick brown fox jumps over the lazy dog",
        "Test vector from febooti.com",
        ""
    };

    logic [MD5_HASH_BITS - 1:0] expected_hashs [number_of_test_vectors - 1 : 0] = '{
        128'h9e107d9d372bb6826bd81d3542a419d6,
        128'h500ab6613c6db7fbd30c62f5ff573d0f,
        128'hd41d8cd98f00b204e9800998ecf8427e
    };
    

    AXI_LITE #(
        .AXI_ADDR_WIDTH(AW),
        .AXI_DATA_WIDTH(DW)
    ) axi_lite(clk);

    // Instantiate the DUT
    md5_top_axi4lite md5_top_axi4lite_inst (
        .clk_i  ( clk      ),
        .rst_ni ( rst      ),
        .slave  ( axi_lite )
    );

    // A simple function to compare two strings
    function int compareHash (
        input logic [MD5_HASH_BITS - 1:0] received_hash,
        input logic [MD5_HASH_BITS - 1:0] expected_hash
    );
        if (received_hash !== expected_hash) begin
            $error("compareHash: received = 0x%32h, expected = 0x%32h", received_hash, expected_hash);
            return 0;
        end else
            return 1;
    endfunction


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

    // Perform the test
    initial begin
        automatic logic [AW-1:0]                addr;
        automatic logic [DW-1:0]                data;
        automatic logic [DW/8-1:0]              strb;
        automatic axi_pkg::resp_t               resp;
        automatic logic [MD5_HASH_BITS - 1:0]   hash;

        // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Loop through all the test vectors
        for (int i = 0; i < number_of_test_vectors; i++) begin
            
            // Perform a hash
            axi_lite_drv.hashString_md5(input_messages[i], hash);

            // Compare the hash
            if (!compareHash(hash, expected_hashs[i]))
                pass = 0;

            // Reset the core
            axi_lite_drv.axi_lite_slave_write(  .addr(32'(MD5_RST)),
                                                .data(32'h0000_0001),
                                                .strb('1),
                                                .resp(resp));

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
