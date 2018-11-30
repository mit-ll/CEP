//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : tb_axi_lite_gps.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Unit test bench for the AXI4-lite based GPS core
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

module tb_axi_lite_gps;

    // Expected CA codes
    parameter [31:0]  exp_ca_codes  [0:0] = { 32'h00001F43 };
    parameter [127:0] exp_p_codes   [0:0] = { 128'h9DDFDD9CE127D8D95394BF2838E7EF54 };
    parameter [127:0] exp_l_codes   [0:0] = { 128'hFD86C6D34155A750E284B05AC643BC27 };

    logic clk           = 0;
    logic rst           = 1;
    logic done          = 0;
    logic pass          = 1;
    logic gps_clk_fast;
    logic gps_clk_slow;

    AXI_LITE #(
        .AXI_ADDR_WIDTH(AW),
        .AXI_DATA_WIDTH(DW)
    ) axi_lite(clk);

    // Instantiate the DUT
    gps_top_axi4lite gps_top_axi4lite_inst (
        .clk_i          ( clk          ),
        .rst_ni         ( rst          ),
        .gps_clk_fast   ( gps_clk_fast ),
        .gps_clk_slow   ( gps_clk_slow ),
        .slave          ( axi_lite     )
    );

    // The GPS core leverages some additional clocks.  The clkgen component has been removed from the core itself and will
    // be generated at the top level of the CEP.  For unit testing of the GPS core, we will borrow the GPS clkgen component
    // 10.23 MHz for P-code and 1.023 MHz for C/A code
    gps_clkgen gps_clkgen_inst (
        .sys_clk_50     ( clk          ),
        .sync_rst_in    ( rst          ),
        .gps_clk_fast   ( gps_clk_fast ),
        .gps_clk_slow   ( gps_clk_slow ),
        .gps_rst        (              )
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
        automatic logic [DW-1:0]    expected_data;

         // Reset the system
        axi_lite_drv.reset_master();
        reset_dut();
        @(posedge clk);

        // Wait a few cycles
        repeat (4) @(posedge clk);

        // Tell the GPS core to generate the next code
        axi_lite_drv.axi_lite_slave_write(  .addr(32'(GPS_GEN_NEXT)),
                                            .data(32'h0000_0001),
                                            .strb('1),
                                            .resp(resp));
        axi_lite_drv.axi_lite_slave_write(  .addr(32'(GPS_GEN_NEXT)),
                                            .data(32'h0000_0000),
                                            .strb('1),
                                            .resp(resp));

        // Wait for valid output
        do begin
            axi_lite_drv.axi_lite_slave_read(   .addr(32'(GPS_GEN_DONE)), 
                                                .data(data), 
                                                .resp(resp));
        end 
        while (data[0] == 1'b0);

        // Verify the CA code ouput
        axi_lite_drv.axi_lite_slave_read(   .addr(32'(GPS_CA_BASE)), 
                                            .data(data), 
                                            .resp(resp));

        if (data !== exp_ca_codes[0]) begin
            $error("tb_axi_lite_gps: CA code: actual/expected (%8h/%8h)", data, exp_ca_codes[0]);
            pass = 0;
        end else begin
            $display("tb_axi_lite_gps: CA code test passed");
        end // if (data !== expected_data)

        // Verify the P code output
        for (int i = 0; i < 4; i++) begin
            axi_lite_drv.axi_lite_slave_read(   .addr(32'(GPS_P_BASE + ((3 -i) * 4))), 
                                                .data(data), 
                                                .resp(resp));

            if (data !== exp_p_codes[0][i*32+:32]) begin
                $error("tb_axi_lite_gps: P code: word %d, actual/expected (%8h/%8h)", i, data, exp_p_codes[0][i*32+:32]);
                pass = 0;
            end else begin
                $display("tb_axi_lite_gps: P code test passed");
            end // if (data !== expected_data)

        end // for (int i = 0; i < 4; i++)

        // Verify the L code output
        for (int i = 0; i < 4; i++) begin
            axi_lite_drv.axi_lite_slave_read(   .addr(32'(GPS_L_BASE + ((3 -i) * 4))), 
                                                .data(data), 
                                                .resp(resp));

            if (data !== exp_l_codes[0][i*32+:32]) begin
                $error("tb_axi_lite_gps: L code: word %d, actual/expected (%8h/%8h)", i, data, exp_l_codes[0][i*32+:32]);
                pass = 0;
            end else begin 
                $display("tb_axi_lite_gps: L code test passed");
            end // if (data !== expected_data)

        end // for (int i = 0; i < 4; i++)

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
