//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : tb_axi4lite_rsa.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based RSA core
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

module tb_axi4lite_rsa;

    logic                   clk = 0;
    logic                   rst = 1;
    logic                   done = 0;
    logic                   pass = 1;


    parameter [31:0] modexp_32bits_test_vectors[0:6][0:3] = '{
        '{32'h00000001, 32'h00000002, 32'h00000005, 32'h00000001},  //msg^exp < mod -> 1^2 < 5
        '{32'h00000001, 32'h00000002, 32'h00000003, 32'h00000001},  //msg^exp < mod -> 1^2 < 3
        '{32'h00000002, 32'h00000002, 32'h00000005, 32'h00000004},  //msg^exp < mod -> 2^2 < 5
        '{32'h00000002, 32'h00000002, 32'h00000003, 32'h00000001},  //msg^exp > mod -> 2^2 > 3
        '{32'h00000004, 32'h0000000D, 32'h000001F1, 32'h000001bd},  //msg^exp > mod -> 4^13 > 497
        '{32'h01234567, 32'h89ABCDEF, 32'h11111111, 32'h0D9EF081},  //msg^exp > mod -> 19088743^2309737967 > 286331153
        '{32'h30000000, 32'hC0000000, 32'h00A00001, 32'h0000CC3F}}; //msg^exp > mod ->

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
    rsa_top_axi4lite rsa_top_axi4lite_inst (
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
        automatic logic [AW-1:0]                    addr;
        automatic logic [DW-1:0]                    data;
        automatic logic [DW/8-1:0]                  strb;
        automatic axi_pkg::resp_t                   resp;
        automatic logic [DW-1:0]                    Wres;

        // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // 32-bit modulo exponentiation tests
        for (int i = 0; i < $size(modexp_32bits_test_vectors, 1); i ++) begin
            axi_lite_drv.modexp_32bits( modexp_32bits_test_vectors[i][0],
                                        modexp_32bits_test_vectors[i][1],
                                        modexp_32bits_test_vectors[i][2],
                                        Wres);

            if (Wres !== modexp_32bits_test_vectors[i][3]) begin
                $error("tb_axi_lite_rsa: Bad 32-bit modulo exponentiation Exp / Act (%08h / %08h)",
                    modexp_32bits_test_vectors[i][3], Wres);

                pass = 0;
            end else begin
                `ifdef DEBUG1
                    $display("DEBUG1: tb_axi_lite_rsa: Good 32-bit modulo exponentiation (RES: %08h)", modexp_32bits_test_vectors[i][3]);
                `endif
                ;
            end // if (Wres != modexp_32bits_test_vectors[3][i])

        end // for (i = 0; i < modexp_32bits_test_vectors.(2); i ++)

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
