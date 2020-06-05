//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      filter_tb.v
// Program:        Common Evaluation Platform (CEP)
// Description:    This is a unit level testbench for the generated
//                 IIR filters.  It operates in two modes (controlled
//                 via a DEFINE passed to the simulator.
// Notes:          Usable defines:
//                 CAPTURE - If defined, the test bench will generate
//                   NUM_SAMPLES of random inputs and capture the outputs
//                   to CAPTURE_FILE for subsequent verification.
//                 VERIFY - If defined (CAPTURE has a higher priority),
//                   the testbench will use the CAPTURE_FILE to verify the
//                   device under test still behaves as expected.
//
//************************************************************************
`timescale 1ns/1ns

// Provide a default for NUM_SAMPLES in the event it was not
// previously defined
`ifndef NUM_SAMPLES
    `define NUM_SAMPLES     100
`endif
        
// Define the default capture file (if not defined)
`ifndef CAPTURE_FILE
    `define CAPTURE_FILE    filter_tb_capture.log
`endif

// Define the DUT if not define
`ifndef DUT_NAME
    `define DUT_NAME        filter
`endif

// Misc. Defines
`define     DATA_WIDTH      64
`define     CLK_PERIOD      10

module filter_tb;

    // DUT Signals and other things
    reg                     clk     = 0;
    reg                     reset   = 1;
    reg [`DATA_WIDTH-1:0]   inData  = 0;
    wire [`DATA_WIDTH-1:0]  outData;
    integer                 errCnt  = 0;

    // Clocking block
    initial begin
        forever #(`CLK_PERIOD/2) clk = !clk;
    end

    // Instantiate the DUT
    `DUT_NAME u1(
        .inData     (inData), 
        .clk        (clk), 
        .reset      (!reset), 
        .outData    (outData)
    );


    // The testbench must have either CAPTURE or VERIFY defined (but NOT both)
    `ifndef CAPTURE
        `ifndef VERIFY
            initial begin
                $display("ERROR: CAPTURE or VERIFY must be defined");
                $fatal;
            end
        `endif
    `endif

    `ifdef CAPTURE
        `ifdef VERIFY
            initial begin
                $display("ERROR: CAPTURE and VERIFY cannot both be defined");
                $fatal;
            end
        `endif
    `endif

    // CAPTURE mode
    // When operating in CAPTURE mode, the testbench will generate a random set of
    // input vectors (64-bits each) and capture the output (following a release
    // from reset).  The I/O pair will be saved to CAPTURE_FILE
    `ifdef CAPTURE
        initial begin
            inData  = 0;
            reset   = 1;
            repeat (5) @(posedge clk);
            reset   = 0;
            repeat (5) @(posedge clk);
            captureData();       
            $finish;
        end
    `endif

    // VERIFY mode  
    // When operating in VERIFY mode, the testbench will read input and output pairs
    // and check that they behave as expected
    `ifdef VERIFY
        initial begin
            inData  = 0;
            reset   = 1;
            repeat (5) @(posedge clk);
            reset   = 0;
            repeat (5) @(posedge clk);
            // Display some status
            playbackData();       
            $finish;
        end
    `endif

    //
    // Task     : captureData
    // Funcion  : 
    //
    task captureData;
        // File Pointer 
        integer fp;
        // Loop variable
        integer i;
        
        begin

            // open file to store input/output
            $display("Capturing %0d samples into file %s", `NUM_SAMPLES, `CAPTURE_FILE);
            fp = $fopen(`CAPTURE_FILE, "w");

            // Generate NUM_SAMPLES random samples
            for (i = 0; i < `NUM_SAMPLES; i = i + 1) begin

                // Jump to positive edge of the clock
                @(posedge clk);

                // Generate a random 64-bit vector
                // A single $urandom call will only give up to 32-bits
                inData[63:32] = $urandom;
                inData[31:0]  = $urandom;

                // Write the random number to the capture file
                $fwrite(fp, "%016x_", inData);

                // Plan to capture the data at the negative clock edge
                @(negedge clk);

                // Write the output data
                $fwrite(fp, "%016x\n", outData);

            end

            // Close the capture file
            $fclose(fp);

        end
    endtask // end captureData

    //
    // Task     : playbackData
    // Funcion  : 
    //
    task playbackData;
        // Loop variable
        integer i;
        // Temporary buffer
        reg [`DATA_WIDTH*2 - 1 : 0] buffer[`NUM_SAMPLES - 1 : 0];
        // Hold the expected data for comparison
        reg [`DATA_WIDTH - 1:0] expData;

        begin

            // Display some status
            $display("Verifying %0d samples from file %s", `NUM_SAMPLES, `CAPTURE_FILE);

            // Read the samples data from the capture file
            $readmemh(`CAPTURE_FILE, buffer);

            // Read NUM_SAMPLES from file, stimulate the input, and compare to the expected output
            for (i = 0; i < `NUM_SAMPLES; i = i + 1) begin

                // Jump to positive edge of the clock
                @(posedge clk);

                // Extract data from the currently selected buffer
                {inData, expData}   = buffer[i];

                // Plan to capture the data at the negative clock edge
                @(negedge clk);

                // Compare expected vs actual output data
                if (outData != expData) begin
                    $display("ERROR: miscompared at sample %0d outData (exp/act) = %016x / %016x", i, expData, outData);
                    errCnt = errCnt + 1;
                end

            end     // end for

            //
            // print summary
            //
            if (errCnt) begin
                $display("==== TEST FAILED with %0d errors ====", errCnt);
            end else begin
                $display("==== TEST PASSED  ====");
            end
      
      end
    endtask // end captureData

endmodule   // end filter_tb