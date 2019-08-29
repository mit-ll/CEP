//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : tb_axi4lite_des3.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based Triple-DES cores
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

module tb_axi4lite_des3;

    // Test Vectors copied from DES3.h
    parameter [63:0]    des3_test_vectors[0:9][0:4] = '{
	//       key1                  key2                    key3                  PT Data             CT Data
	'{64'h0101010101010101, 64'h0101010101010101, 64'h0101010101010101, 64'h95F8A5E5DD31D900, 64'h8000000000000000},
	'{64'h0101010101010101, 64'h0101010101010101, 64'h0101010101010101, 64'h9D64555A9A10B852, 64'h0000001000000000},
	'{64'h3849674C2602319E, 64'h3849674C2602319E, 64'h3849674C2602319E, 64'h51454B582DDF440A, 64'h7178876E01F19B2A},
	'{64'h04B915BA43FEB5B6, 64'h04B915BA43FEB5B6, 64'h04B915BA43FEB5B6, 64'h42FD443059577FA2, 64'hAF37FB421F8C4095},
	'{64'h0123456789ABCDEF, 64'h0123456789ABCDEF, 64'h0123456789ABCDEF, 64'h736F6D6564617461, 64'h3D124FE2198BA318},
	'{64'h0123456789ABCDEF, 64'h5555555555555555, 64'h0123456789ABCDEF, 64'h736F6D6564617461, 64'hFBABA1FF9D05E9B1},
	'{64'h0123456789ABCDEF, 64'h5555555555555555, 64'hFEDCBA9876543210, 64'h736F6D6564617461, 64'h18d748e563620572},
	'{64'h0352020767208217, 64'h8602876659082198, 64'h64056ABDFEA93457, 64'h7371756967676C65, 64'hc07d2a0fa566fa30},
	'{64'h0101010101010101, 64'h8001010101010101, 64'h0101010101010102, 64'h0000000000000000, 64'he6e6dd5b7e722974},
	'{64'h1046103489988020, 64'h9107D01589190101, 64'h19079210981A0101, 64'h0000000000000000, 64'he1ef62c332fe825b}};

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
    des3_top_axi4lite des_top_axi4lite_inst (
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

    // Perform the test
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

        // Execute the Plaintext Tests
        for (int i = 0; i < 10; i++) begin
 
            `ifdef DEBUG1
                $display("DEBUG1: Running Encryption test %0d", i);
            `endif

            // Load the keys (there are a total of six 32-bit key words)
            for (int j = 0; j < 6; j++) begin          
                axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_KEY_BASE + (((DES3_KEY_WORDS - 1) - j) * BYTES_PER_WORD))),
                                                   .data(des3_test_vectors[i][j / 2][(1 - (j % 2)) * 32 +: 32]),
                                                   .strb('1),
                                                   .resp(resp));            
            end // end for

            // Load the plaintext
            for (int j = 0; j < DES3_BLOCK_WORDS; j++) begin
                axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_IN_BASE + (((DES3_BLOCK_WORDS - 1) - j) * BYTES_PER_WORD))),
                                                   .data(des3_test_vectors[i][3][(1-j) * 32 +: 32]),
                                                   .strb('1),
                                                   .resp(resp));
            end // for (int j = 6; j < 8; j++)

            // Initiate a start
            axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_START)),
                                               .data(32'h0000_0001),
                                               .strb('1),
                                               .resp(resp));

            // Poll the CT Valid bit to determine when encryption is complete
            do begin
                axi_lite_drv.axi_lite_slave_read(   .addr(32'(DES3_DONE)), 
                                                    .data(data), 
                                                    .resp(resp));
            end 
            while (data[0] == 1'b0);

            // Read and Compare the CipherText
            for (int j = 0; j < DES3_BLOCK_WORDS; ++j) begin
                axi_lite_drv.axi_lite_slave_read(   .addr(32'(DES3_CT_BASE + (j * BYTES_PER_WORD))), 
                                                    .data(data), 
                                                    .resp(resp));

                if (data !==des3_test_vectors[i][4][(1 -j)*32 +:32]) begin
                    $error("Unexpected C iphertext - word #[%0d], expected %h, got %h", j, des3_test_vectors[i][4][(1 -j)*32 +:32], data);
                    assign pass = 1'b0;
                end // if (data != des3_test_vectors[i][4][(1 -j)*32 +:32])

            end // for (int i = 0; i < AES_BLOCK_WORDS; ++i)

        end // for (i = 0; i < 10; i++)

        // Execute the Ciphertext Tests
        for (int i = 0; i < 10; i++) begin
 
            `ifdef DEBUG1
                $display("DEBUG1: Running Decryption test %0d", i);
            `endif

            // Load the keys in reverse order (there are a total of six 32-bit key words)
            for (int j = 0; j < 6; j++) begin          
                axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_KEY_BASE + (((DES3_KEY_WORDS - 1) - j) * BYTES_PER_WORD))),
                                                   .data(des3_test_vectors[i][j / 2][(1 - (j % 2)) * 32 +: 32]),
                                                   .strb('1),
                                                   .resp(resp));            
            end // end for

            // Load the ciphertext
            for (int j = 0; j < DES3_BLOCK_WORDS; j++) begin
                axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_IN_BASE + (((DES3_BLOCK_WORDS - 1) - j) * BYTES_PER_WORD))),
                                                   .data(des3_test_vectors[i][4][(1-j) * 32 +: 32]),
                                                   .strb('1),
                                                   .resp(resp));
            end // for (int j = 6; j < 8; j++)

            // Set the decrypt bit, then start
            axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_DECRYPT)),
                                               .data(32'h0000_0001),
                                               .strb('1),
                                               .resp(resp));
            axi_lite_drv.axi_lite_slave_write( .addr(32'(DES3_START)),
                                               .data(32'h0000_0001),
                                               .strb('1),
                                               .resp(resp));

            // Poll the CT Valid bit to determine when encryption is complete
            do begin
                axi_lite_drv.axi_lite_slave_read(   .addr(32'(DES3_DONE)), 
                                                    .data(data), 
                                                    .resp(resp));
            end 
            while (data[0] == 1'b0);

            // Read and Compare the CipherText
            for (int j = 0; j < DES3_BLOCK_WORDS; ++j) begin
                axi_lite_drv.axi_lite_slave_read(   .addr(32'(DES3_CT_BASE + (j * BYTES_PER_WORD))), 
                                                    .data(data), 
                                                    .resp(resp));

                if (data !== des3_test_vectors[i][3][(1 -j)*32 +:32]) begin
                    $error("Unexpected Plaintext - word #[%0d], expected %h, got %h", j, des3_test_vectors[i][3][(1 -j)*32 +:32], data);
                    assign pass = 1'b0;
                end else begin
                    `ifdef DEBUG1
                        $display("DEBUG1: tb_axi4lite_des3: Decryption Test - Good PT word = %8h", data);
                    `endif
                    ;
                end // end else

            end // for (int i = 0; i < AES_BLOCK_WORDS; ++i)

        end // for (i = 0; i < 10; i++)

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
