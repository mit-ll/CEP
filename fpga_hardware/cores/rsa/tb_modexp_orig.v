//======================================================================
//
// tb_modexp.v
// -----------
// Testbench modular exponentiation core.
//
//
// Author: Joachim Strombergson, Peter Magnusson
// Copyright (c) 2015, NORDUnet A/S All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the name of the NORDUnet nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//======================================================================

//------------------------------------------------------------------
// Simulator directives.
//------------------------------------------------------------------
`timescale 1ns/100ps


//------------------------------------------------------------------
// Test module.
//------------------------------------------------------------------
module tb_modexp();

  //----------------------------------------------------------------
  // Internal constant and parameter definitions.
  //----------------------------------------------------------------
  // Debug output control.
  parameter DEBUG               = 0;
  parameter DEBUG_EI            = 0;
  parameter DEBUG_RESULT        = 0;
  parameter DISPLAY_TEST_CYCLES = 1;


  // Clock defines.
  localparam CLK_HALF_PERIOD = 1;
  localparam CLK_PERIOD      = 2 * CLK_HALF_PERIOD;


  // The DUT address map.
  localparam GENERAL_PREFIX        = 4'h0;
  localparam ADDR_NAME0            = 8'h00;
  localparam ADDR_NAME1            = 8'h01;
  localparam ADDR_VERSION          = 8'h02;

  localparam ADDR_CTRL             = 8'h08;
  localparam CTRL_START_BIT        = 0;

  localparam ADDR_STATUS           = 8'h09;
  localparam STATUS_READY_BIT      = 0;

  localparam ADDR_MODULUS_LENGTH   = 8'h20;
  localparam ADDR_EXPONENT_LENGTH  = 8'h21;
  localparam ADDR_LENGTH           = 8'h22;

  localparam ADDR_MODULUS_PTR_RST  = 8'h30;
  localparam ADDR_MODULUS_DATA     = 8'h31;

  localparam ADDR_EXPONENT_PTR_RST = 8'h40;
  localparam ADDR_EXPONENT_DATA    = 8'h41;

  localparam ADDR_MESSAGE_PTR_RST  = 8'h50;
  localparam ADDR_MESSAGE_DATA     = 8'h51;

  localparam ADDR_RESULT_PTR_RST   = 8'h60;
  localparam ADDR_RESULT_DATA      = 8'h61;


  //----------------------------------------------------------------
  // Register and Wire declarations.
  //----------------------------------------------------------------
  reg [31 : 0]  test_cycle_ctr;
  reg           test_cycle_ctr_rst;
  reg           test_cycle_ctr_inc;

  reg [31 : 0]  cycle_ctr;
  reg [31 : 0]  error_ctr;
  reg [31 : 0]  tc_ctr;

  reg [31 : 0]  read_data;
  reg [127 : 0] result_data;

  reg           tb_clk;
  reg           tb_reset_n;
  reg           tb_cs;
  reg           tb_we;
  reg [11  : 0] tb_address;
  reg [31 : 0]  tb_write_data;
  wire [31 : 0] tb_read_data;
  wire          tb_error;


  //----------------------------------------------------------------
  // Device Under Test.
  //----------------------------------------------------------------
  modexp dut(
             .clk(tb_clk),
             .reset_n(tb_reset_n),
             .cs(tb_cs),
             .we(tb_we),
             .address(tb_address),
             .write_data(tb_write_data),
             .read_data(tb_read_data)
            );


  //----------------------------------------------------------------
  // clk_gen
  //
  // Always running clock generator process.
  //----------------------------------------------------------------
  always
    begin : clk_gen
      #CLK_HALF_PERIOD;
      tb_clk = !tb_clk;
    end // clk_gen


  //----------------------------------------------------------------
  // sys_monitor()
  //
  // An always running process that creates a cycle counter and
  // conditionally displays information about the DUT.
  //----------------------------------------------------------------
  always
    begin : sys_monitor
      cycle_ctr = cycle_ctr + 1;

      #(CLK_PERIOD);

      if (DEBUG)
        begin
          dump_dut_state();
        end
    end


  //----------------------------------------------------------------
  // test_cycle_counter
  //
  // Used to measure the number of cycles it takes to perform
  // a given test case.
  //----------------------------------------------------------------
  always @ (posedge tb_clk)
    begin
      if (test_cycle_ctr_rst)
        test_cycle_ctr = 64'h0000000000000000;

      if (test_cycle_ctr_inc)
        test_cycle_ctr = test_cycle_ctr + 1;
    end


  //----------------------------------------------------------------
  // start_test_cycle_ctr
  //
  // Reset and start the test cycle counter.
  //----------------------------------------------------------------
  task start_test_cycle_ctr();
    begin
      test_cycle_ctr_rst = 1;
      #(CLK_PERIOD);
      test_cycle_ctr_rst = 0;

      test_cycle_ctr_inc = 1;
    end
  endtask // start_test_cycle_ctr()


  //----------------------------------------------------------------
  // stop_test_cycle_ctr()
  //
  // Stop the test cycle counter and optionally display the
  // result.
  //----------------------------------------------------------------
  task stop_test_cycle_ctr();
    begin
      test_cycle_ctr_inc = 0;
      #(CLK_PERIOD);

      if (DISPLAY_TEST_CYCLES)
        $display("*** Number of cycles performed during test: 0x%016x", test_cycle_ctr);
    end
  endtask // stop_test_cycle_ctr()


  //----------------------------------------------------------------
  // ei_monitor()
  //
  // Displays ei_new, the most important variable for determining
  // what modexp will do (i.e. should Z=MONTPROD( Z, P, M) be
  // performed
  //----------------------------------------------------------------
  always @*
    begin : ei_monitor
      if (DEBUG_EI)
        if (dut.modexp_ctrl_reg == dut.CTRL_ITERATE_Z_P)
          $display("loop counter %d: ei = %d", dut.loop_counter_reg, dut.ei_reg);
    end

  //----------------------------------------------------------------
  // z_monitor()
  //
  // Displays the contents of the result_mem.
  //----------------------------------------------------------------
  always @*
    begin : result_monitor
      if (DEBUG_RESULT)
        $display("result_mem[0][1] = %x %x",dut.result_mem.mem[0],dut.result_mem.mem[1]);
    end


  //----------------------------------------------------------------
  // dump_dut_state()
  //
  // Dump the state of the dump when needed.
  //----------------------------------------------------------------
  task dump_dut_state();
    begin
      $display("cycle: 0x%016x", cycle_ctr);
      $display("State of DUT");
      $display("------------");
      $display("Inputs and outputs:");
      $display("cs   = 0x%01x, we = 0x%01x", tb_cs, tb_we);
      $display("addr = 0x%08x, read_data = 0x%08x, write_data = 0x%08x",
               tb_address, tb_read_data, tb_write_data);
      $display("");

      $display("State:");
      $display("ready_reg = 0x%01x, start_reg = 0x%01x, start_new = 0x%01x",
               dut.core_inst.ready_reg, dut.start_reg, dut.start_new);
      $display("residue_valid = 0x%01x", dut.core_inst.residue_valid_reg);
      $display("loop_counter_reg = 0x%08x", dut.core_inst.loop_counter_reg);
      $display("exponent_length_reg = 0x%02x exponent_length_m1 = 0x%02x modulus_length_reg = 0x%02x modulus_length_m1 = 0x%02x",
               dut.exponent_length_reg,  dut.core_inst.exponent_length_m1, dut.modulus_length_reg, dut.core_inst.modulus_length_m1);
      $display("ctrl_reg = 0x%04x", dut.core_inst.modexp_ctrl_reg);
      $display("");
    end
  endtask // dump_dut_state


  //----------------------------------------------------------------
  // reset_dut()
  //
  // Toggle reset to put the DUT into a well known state.
  //----------------------------------------------------------------
  task reset_dut();
    begin
      $display("*** Toggle reset.");
      tb_reset_n = 0;

      #(2 * CLK_PERIOD);
      tb_reset_n = 1;
      $display("");
    end
  endtask // reset_dut


  //----------------------------------------------------------------
  // display_test_results()
  //
  // Display the accumulated test results.
  //----------------------------------------------------------------
  task display_test_results();
    begin
      if (error_ctr == 0)
        begin
          $display("*** All %02d test cases completed successfully", tc_ctr);
        end
      else
        begin
          $display("*** %02d tests completed - %02d test cases did not complete successfully.",
                   tc_ctr, error_ctr);
        end
    end
  endtask // display_test_results


  //----------------------------------------------------------------
  // init_sim()
  //
  // Initialize all counters and testbed functionality as well
  // as setting the DUT inputs to defined values.
  //----------------------------------------------------------------
  task init_sim();
    begin
      cycle_ctr          = 0;
      error_ctr          = 0;
      tc_ctr             = 0;

      tb_clk             = 0;
      tb_reset_n         = 1;

      tb_cs              = 0;
      tb_we              = 0;
      tb_address         = 8'h00;
      tb_write_data      = 32'h00000000;
    end
  endtask // init_sim


  //----------------------------------------------------------------
  // read_word()
  //
  // Read a data word from the given address in the DUT.
  // the word read will be available in the global variable
  // read_data.
  //----------------------------------------------------------------
  task read_word(input [11 : 0] address);
    begin
      tb_address = address;
      tb_cs = 1;
      tb_we = 0;
      #(CLK_PERIOD);
      read_data = tb_read_data;
      tb_cs = 0;

      if (DEBUG)
        begin
          $display("*** (read_word) Reading 0x%08x from 0x%02x.", read_data, address);
          $display("");
        end
    end
  endtask // read_word


  //----------------------------------------------------------------
  // write_word()
  //
  // Write the given word to the DUT using the DUT interface.
  //----------------------------------------------------------------
  task write_word(input [11 : 0] address,
                  input [31 : 0] word);
    begin
      if (DEBUG)
        begin
          $display("*** (write_word) Writing 0x%08x to 0x%02x.", word, address);
          $display("");
        end

      tb_address = address;
      tb_write_data = word;
      tb_cs = 1;
      tb_we = 1;
      #(CLK_PERIOD);
      tb_cs = 0;
      tb_we = 0;
    end
  endtask // write_word


  //----------------------------------------------------------------
  // wait_ready()
  //
  // Wait until the ready flag in the core is set.
  //----------------------------------------------------------------
  task wait_ready();
    begin
      while (tb_read_data != 32'h00000001)
          read_word({GENERAL_PREFIX, ADDR_STATUS});

      if (DEBUG)
        $display("*** (wait_ready) Ready flag has been set.");
    end
  endtask // wait_ready


  //----------------------------------------------------------------
  // dump_message_mem()
  //
  // Dump the contents of the message memory.
  //----------------------------------------------------------------
  task dump_message_mem();
    reg [8 : 0] i;
    begin
      $display("Contents of the message memory:");
      for (i = 0 ; i < 256 ; i = i + 8)
        begin
          $display("message_mem[0x%02x .. 0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x",
                   i[7 : 0], (i[7 : 0] + 8'h07),
                   dut.core_inst.message_mem.mem[(i[7 : 0] + 0)], dut.core_inst.message_mem.mem[(i[7 : 0] + 1)],
                   dut.core_inst.message_mem.mem[(i[7 : 0] + 2)], dut.core_inst.message_mem.mem[(i[7 : 0] + 3)],
                   dut.core_inst.message_mem.mem[(i[7 : 0] + 4)], dut.core_inst.message_mem.mem[(i[7 : 0] + 5)],
                   dut.core_inst.message_mem.mem[(i[7 : 0] + 6)], dut.core_inst.message_mem.mem[(i[7 : 0] + 7)],
                   );
        end
      $display("");
    end
  endtask // dump_message_mem


  //----------------------------------------------------------------
  // dump_exponent_mem()
  //
  // Dump the contents of the exponent memory.
  //----------------------------------------------------------------
  task dump_exponent_mem();
    reg [8 : 0] i;
    begin
      $display("Contents of the exponent memory:");
      for (i = 0 ; i < 256 ; i = i + 8)
        begin
          $display("exponent_mem[0x%02x .. 0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x",
                   i[7 : 0], (i[7 : 0] + 8'h07),
                   dut.core_inst.exponent_mem.mem[(i[7 : 0] + 0)], dut.core_inst.exponent_mem.mem[(i[7 : 0] + 1)],
                   dut.core_inst.exponent_mem.mem[(i[7 : 0] + 2)], dut.core_inst.exponent_mem.mem[(i[7 : 0] + 3)],
                   dut.core_inst.exponent_mem.mem[(i[7 : 0] + 4)], dut.core_inst.exponent_mem.mem[(i[7 : 0] + 5)],
                   dut.core_inst.exponent_mem.mem[(i[7 : 0] + 6)], dut.core_inst.exponent_mem.mem[(i[7 : 0] + 7)],
                   );
        end
      $display("");
    end
  endtask // dump_exponent_mem


  //----------------------------------------------------------------
  // dump_modulus_mem()
  //
  // Dump the contents of the modulus memory.
  //----------------------------------------------------------------
  task dump_modulus_mem();
    reg [8 : 0] i;
    begin
      $display("Contents of the modulus memory:");
      for (i = 0 ; i < 256 ; i = i + 8)
        begin
          $display("modulus_mem[0x%02x .. 0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x",
                   i[7 : 0], (i[7 : 0] + 8'h07),
                   dut.core_inst.modulus_mem.mem[(i[7 : 0] + 0)], dut.core_inst.modulus_mem.mem[(i[7 : 0] + 1)],
                   dut.core_inst.modulus_mem.mem[(i[7 : 0] + 2)], dut.core_inst.modulus_mem.mem[(i[7 : 0] + 3)],
                   dut.core_inst.modulus_mem.mem[(i[7 : 0] + 4)], dut.core_inst.modulus_mem.mem[(i[7 : 0] + 5)],
                   dut.core_inst.modulus_mem.mem[(i[7 : 0] + 6)], dut.core_inst.modulus_mem.mem[(i[7 : 0] + 7)],
                   );
        end
      $display("");
    end
  endtask // dump_modulus_mem


  //----------------------------------------------------------------
  // dump_residue_mem()
  //
  // Dump the contents of the residue memory.
  //----------------------------------------------------------------
  task dump_residue_mem();
    reg [8 : 0] i;
    begin
      $display("Contents of the residue memory:");
      for (i = 0 ; i < 256 ; i = i + 8)
        begin
          $display("residue_mem[0x%02x .. 0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x",
                   i[7 : 0], (i[7 : 0] + 8'h07),
                   dut.core_inst.residue_mem.mem[(i[7 : 0] + 0)], dut.core_inst.residue_mem.mem[(i[7 : 0] + 1)],
                   dut.core_inst.residue_mem.mem[(i[7 : 0] + 2)], dut.core_inst.residue_mem.mem[(i[7 : 0] + 3)],
                   dut.core_inst.residue_mem.mem[(i[7 : 0] + 4)], dut.core_inst.residue_mem.mem[(i[7 : 0] + 5)],
                   dut.core_inst.residue_mem.mem[(i[7 : 0] + 6)], dut.core_inst.residue_mem.mem[(i[7 : 0] + 7)],
                   );
        end
      $display("");
    end
  endtask // dump_residue_mem


  //----------------------------------------------------------------
  // dump_result_mem()
  //
  // Dump the contents of the result memory.
  //----------------------------------------------------------------
  task dump_result_mem();
    reg [8 : 0] i;
    begin
      $display("Contents of the result memory:");
      for (i = 0 ; i < 256 ; i = i + 8)
        begin
          $display("result_mem[0x%02x .. 0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x",
                   i[7 : 0], (i[7 : 0] + 8'h07),
                   dut.core_inst.result_mem.mem[(i[7 : 0] + 0)], dut.core_inst.result_mem.mem[(i[7 : 0] + 1)],
                   dut.core_inst.result_mem.mem[(i[7 : 0] + 2)], dut.core_inst.result_mem.mem[(i[7 : 0] + 3)],
                   dut.core_inst.result_mem.mem[(i[7 : 0] + 4)], dut.core_inst.result_mem.mem[(i[7 : 0] + 5)],
                   dut.core_inst.result_mem.mem[(i[7 : 0] + 6)], dut.core_inst.result_mem.mem[(i[7 : 0] + 7)],
                   );
        end
      $display("");
    end
  endtask // dump_result_mem


  //----------------------------------------------------------------
  // dump_memories()
  //
  // Dump the contents of the memories in the dut.
  //----------------------------------------------------------------
  task dump_memories();
    begin
      dump_message_mem();
      dump_exponent_mem();
      dump_modulus_mem();
      dump_residue_mem();
      dump_result_mem();
    end
  endtask // dump_memories


  //----------------------------------------------------------------
  // tc1
  //
  // A first, very simple testcase where we want to do:
  // c = m ** e % N with the following (decimal) test values:
  //  m = 3
  //  e = 7
  //  n = 11
  //  c = 3 ** 7 % 11 = 9
  //----------------------------------------------------------------
  task tc1();
    reg [31 : 0] read_data;

    begin
      tc_ctr = tc_ctr + 1;
      $display("TC1: Trying to calculate 3**7 mod 11 = 9");

      // Write 3 to message memory.
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000003);

      // Write 7 to exponent memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000007);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);

      // Write 11 to modulus memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h0000000b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000001);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      // Read out result word and check result.
      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA});
      read_data = tb_read_data;

      if (read_data == 32'h00000009)
        begin
          $display("*** TC1 successful.");
          $display("");
        end
      else
        begin
          $display("*** ERROR: TC1 NOT successful.");
          $display("Expected: 0x09, got 0x%08x", read_data);
          error_ctr = error_ctr + 1;
          dump_memories();
        end
    end
  endtask // tc1


  //----------------------------------------------------------------
  // tc2
  //
  // c = m ** e % N with the following (decimal) test values:
  //  m = 251
  //  e = 251
  //  n = 257
  //  c = 251 ** 251 % 257 = 183
  //----------------------------------------------------------------
  task tc2();
    reg [31 : 0] read_data;

    begin
      tc_ctr = tc_ctr + 1;
      $display("TC2: Trying to calculate 251**251 mod 257 = 183");

      // Write 13 to message memory.
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h000000fb);

      // Write 11 to exponent memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h000000fb);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);

      // Write 7 to modulus memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000101);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000001);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      // Read out result word and check result.
      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA});
      read_data = tb_read_data;

      if (read_data == 32'h000000b7)
        begin
          $display("*** TC2 successful.");
          $display("");
        end
      else
        begin
          $display("*** ERROR: TC2 NOT successful.");
          $display("Expected: 0x000000b7, got 0x%08x", read_data);
          error_ctr = error_ctr + 1;
        end
    end
  endtask // tc2


  //----------------------------------------------------------------
  // tc3
  //
  // c = m ** e % N with the following (decimal) test values:
  //  m = 0x81
  //  e = 0x41
  //  n = 0x87
  //  c = 0x81 ** 0x41 % 0x87 = 0x36
  //----------------------------------------------------------------
  task tc3();
    reg [31 : 0] read_data;

    begin
      tc_ctr = tc_ctr + 1;
      $display("TC3: Trying to calculate 0x81 ** 0x41 mod 0x87 = 0x36");

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000081);

      // Write 11 to exponent memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000041);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);

      // Write 7 to modulus memory and set length to one word.
      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000087);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000001);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      // Read out result word and check result.
      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA});
      read_data = tb_read_data;

      if (read_data == 32'h00000036)
        begin
          $display("*** TC3 successful.");
          $display("");
        end
      else
        begin
          $display("*** ERROR: TC3 NOT successful.");
          $display("Expected: 0x06, got 0x%08x", read_data);
          error_ctr = error_ctr + 1;
        end
    end
  endtask // tc3


  //----------------------------------------------------------------
  // assertEquals
  //----------------------------------------------------------------
  function assertEquals(
      input [31:0] expected,
      input [31:0] actual
    );
    begin
      if (expected === actual)
        begin
          assertEquals = 1; // success
        end
      else
        begin
          $display("Expected: 0x%08x, got 0x%08x", expected, actual);
          assertEquals = 0; // failure
        end
    end
  endfunction // assertEquals

  integer success;


  //----------------------------------------------------------------
  // autogenerated_BASIC_33bit()
  //
  // Task that tests modexp with 33 bit oprerands.
  //----------------------------------------------------------------
  task autogenerated_BASIC_33bit();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("autogenerated_BASIC_33bit: 00000001946473e1 ** h000000010e85e74f mod 0000000170754797 ");

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h946473e1);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h0e85e74f);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h70754797);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000002);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000002);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h7761ed4f, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: autogenerated_BASIC_33bit was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** autogenerated_BASIC_33bit success.");
    end
  endtask // autogenerated_BASIC_33bit


  //----------------------------------------------------------------
  // autogenerated_BASIC_128bit()
  //
  // Task that tests modexp with 128 bit operands.
  //----------------------------------------------------------------
  task autogenerated_BASIC_128bit();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("autogenerated_BASIC_128bit");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h3285c343);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h2acbcb0f);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h4d023228);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h2ecc73db);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h29462882);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h12caa2d5);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb80e1c66);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h1006807f);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h267d2f2e);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h51c216a7);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hda752ead);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h48d22d89);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000004);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000005);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h0ddc404d, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h91600596, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h7425a8d8, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha066ca56, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: autogenerated_BASIC_128bit was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** autogenerated_BASIC_128bit success.");
    end
  endtask // autogenerated_BASIC_128bit


  //----------------------------------------------------------------
  // e64bit_64bit_modulus()
  //----------------------------------------------------------------
  task e64bit_64bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with 64 bit exponent and 64 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h12345678);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h97543211);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hfeababab);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hdeadbeef);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffee);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hbeefbeef);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000002);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000003);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'he52c5b9f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h85de87eb, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: 64 bit exponent and 64 bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** 64 bit exponent and 64 bit modulus success.");
    end
  endtask // e64bit_64bit_modulus


  //----------------------------------------------------------------
  // e65537_64bit_modulus()
  //----------------------------------------------------------------
  task e65537_64bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 64 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hf077656f);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h3bf9e69b);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb6684dc3);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h79a5824b);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000003);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h132d8e17, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hdd4d85a4, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_64bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_64bit_modulus success.");
    end
  endtask // e65537_64bit_modulus


  //----------------------------------------------------------------
  // e65537_64bit_modulus_elength()
  //----------------------------------------------------------------
  task e65537_64bit_modulus_elength();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 64 bit modulus, explicit exponent length.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hf077656f);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h3bf9e69b);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb6684dc3);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h79a5824b);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000003);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h132d8e17, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hdd4d85a4, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_64bit_modulus with explicit elength was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_64bit_modulus success.");
    end
  endtask // e65537_64bit_modulus_elength


  //----------------------------------------------------------------
  // e65537_128bit_modulus()
  //----------------------------------------------------------------
  task e65537_128bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 128 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hf5e8eee0);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc06b048a);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h964b2105);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h2c36ad6b);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h956e61b3);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h27997bc4);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h94e7e5c9);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb53585cf);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000005);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h9c6d322c, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h25ab8bd3, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4aa80100, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf0f3a02c, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_128bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_128bit_modulus success.");
    end
  endtask // e65537_128bit_modulus


  //----------------------------------------------------------------
  // e65537_256bit_modulus()
  //
  // Task that tests modexp with small exponent and 256 bit modulus.
  //----------------------------------------------------------------
  task e65537_256bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 256 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hf169d36e);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hbe2ce61d);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc2e87809);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h4fed15c3);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h7c70eac5);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'ha123e643);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h299b36d2);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h788e583b);

      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hf169d36e);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hbe2ce61d);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hc2e87809);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h4fed15c3);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h7c70eac5);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'ha123e643);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h299b36d2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h788e583a);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000009);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf169d36e, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hbe2ce61d, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hc2e87809, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4fed15c3, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h7c70eac5, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha123e643, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h299b36d2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h788e583a, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_256bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_256bit_modulus success.");
    end
  endtask // e65537_256bit_modulus


  //----------------------------------------------------------------
  // e65537_1024bit_modulus()
  //
  // Task that tests modexp with small exponent and
  //  2048 bit modulus.
  //----------------------------------------------------------------
  task e65537_1024bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 1024 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000021);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h45d55343, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha0971add, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_1024bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_1024bit_modulus success.");
    end
  endtask // e65537_1024bit_modulus


  //----------------------------------------------------------------
  // e65537_1536bit_modulus()
  //
  // Task that tests modexp with small exponent and
  // 1536 bit modulus.
  //----------------------------------------------------------------
  task e65537_1536bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 1536 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000031);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4ade4f46, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h02cb4a2f, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_1536bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_1536bit_modulus success.");
    end
  endtask // e65537_1536bit_modulus



  //----------------------------------------------------------------
  // e65537_1664bit_modulus()
  //
  // Task that tests modexp with small exponent and
  // 1664 bit modulus.
  //----------------------------------------------------------------
  task e65537_1664bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 1664 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000035);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h88671c15, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2aeeb8b2, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_1664bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_1664it_modulus success.");
    end
  endtask // e65537_1664bit_modulus


  //----------------------------------------------------------------
  // e65537_2048bit_modulus()
  //
  // Task that tests modexp with small exponent and
  //  2048 bit modulus.
  //----------------------------------------------------------------
  task e65537_2048bit_modulus();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Test with e = 65537 and 2048 bit modulus.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);

      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hffeeffef);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffaabbcc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hddeeffff);

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000001);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000041);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);

      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf1752196, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4c36e92f, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: e65537_2048bit_modulus was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** e65537_2048bit_modulus success.");
    end
  endtask // e65537_2048bit_modulus


  //----------------------------------------------------------------
  // rob_dec_1024()
  //
  // Task that tests modexp with 1024 bit decipher/sign with
  // real RSA key operands.
  //----------------------------------------------------------------
  task rob_dec_1024();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Desipher/Sign Test with Rob 1024 bit operands.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h3ff26c9e);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h32685b93);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h66570228);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hf0603c4e);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h04a717c1);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h8038b116);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'heb48325e);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hcada992a);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h920bb241);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h5aee4afe);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'he2a37e87);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hb35b9519);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hb335775d);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h989553e9);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h1326f46e);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h2cdf6b7b);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h84aabfa9);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hef24c600);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hb56872ad);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h5edb9041);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'he8ecd7f8);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h535133fb);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hdefc92c7);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h42384226);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h7d40e5f5);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hc91bd745);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h9578e460);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hfc858374);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h3172bed3);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h73b6957c);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'hc0d6a68e);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h33156a61);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h0001ffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hffffffff);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00303130);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h0d060960);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h86480165);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h03040201);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h05000420);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h8e36fc9a);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'ha31724c3);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h2416263c);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h0366a175);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hfabbb92b);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h741ca649);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h6107074d);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h0343b597);


      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hd075ec0a);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h95048ef8);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hcaa69073);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h8d9d58e9);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h1764b437);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h50b58cad);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h8a6e3199);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h135f80ee);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h84eb2bde);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h58d38ee3);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h5825e91e);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hafdeb1ba);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'ha15a160b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h0057c47c);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc7765e31);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h868a3e15);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h5ee57cef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hb008c4dd);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h6a0a89ee);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h98a4ee9c);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h971a07de);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h61e5b0d3);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hcf70e1cd);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc6a0de5b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h451f2fb9);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hdb995196);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h9f2f884b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h4b09749a);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'he6c4ddbe);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h7ee61f79);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h265c6adf);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hb16b3015);


      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000021);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000021);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h06339a64, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h367db02a, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf41158cc, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h95e76049, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4519c165, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h111184be, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'he41d8ee2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2ae5f5d1, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h1da7f962, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hac93ac88, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h915eee13, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha3350c22, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hf0dfa62e, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hfdfc2b62, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h29f26e27, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hbebdc84e, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h4746df79, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h7b387ad2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h13423c9f, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h98e8a146, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hff486b6c, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h1a85414e, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h73117121, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hb700e547, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hab4e07b2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h21b988b8, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h24dd77c2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h046b0a20, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hcddb986a, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hac75c2f2, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hb044ed59, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hea565879, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: rob_dec_1024 was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** rob_dec_1024 success.");
    end
  endtask // rob_dec_1024


  //----------------------------------------------------------------
  // rob_enc_1024()
  //
  // Task that tests modexp with 1024 bit encipher/verify with
  // real RSA key operands.
  //----------------------------------------------------------------
  task rob_enc_1024();
    reg [31 : 0] read_data;
    begin
      success = 32'h1;
      tc_ctr = tc_ctr + 1;
      $display("Encipher/Verify Test with Rob 1024 bit operands.");

      write_word({GENERAL_PREFIX, ADDR_EXPONENT_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_EXPONENT_DATA}, 32'h00010001);


      write_word({GENERAL_PREFIX, ADDR_MESSAGE_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h06339a64);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h367db02a);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hf41158cc);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h95e76049);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h4519c165);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h111184be);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'he41d8ee2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h2ae5f5d1);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h1da7f962);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hac93ac88);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h915eee13);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'ha3350c22);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hf0dfa62e);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hfdfc2b62);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h29f26e27);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hbebdc84e);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h4746df79);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h7b387ad2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h13423c9f);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h98e8a146);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hff486b6c);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h1a85414e);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h73117121);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb700e547);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hab4e07b2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h21b988b8);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h24dd77c2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'h046b0a20);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hcddb986a);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hac75c2f2);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hb044ed59);
      write_word({GENERAL_PREFIX, ADDR_MESSAGE_DATA}, 32'hea565879);


      write_word({GENERAL_PREFIX, ADDR_MODULUS_PTR_RST}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h00000000);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hd075ec0a);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h95048ef8);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hcaa69073);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h8d9d58e9);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h1764b437);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h50b58cad);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h8a6e3199);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h135f80ee);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h84eb2bde);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h58d38ee3);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h5825e91e);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hafdeb1ba);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'ha15a160b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h0057c47c);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc7765e31);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h868a3e15);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h5ee57cef);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hb008c4dd);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h6a0a89ee);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h98a4ee9c);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h971a07de);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h61e5b0d3);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hcf70e1cd);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hc6a0de5b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h451f2fb9);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hdb995196);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h9f2f884b);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h4b09749a);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'he6c4ddbe);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h7ee61f79);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'h265c6adf);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_DATA}, 32'hb16b3015);


      write_word({GENERAL_PREFIX, ADDR_EXPONENT_LENGTH}, 32'h00000021);
      write_word({GENERAL_PREFIX, ADDR_MODULUS_LENGTH}, 32'h00000021);

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({GENERAL_PREFIX, ADDR_CTRL}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({GENERAL_PREFIX, ADDR_RESULT_PTR_RST}, 32'h00000000);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00000000, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h0001ffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hffffffff, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h00303130, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h0d060960, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h86480165, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h03040201, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h05000420, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h8e36fc9a, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'ha31724c3, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h2416263c, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h0366a175, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'hfabbb92b, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h741ca649, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h6107074d, read_data);
      read_word({GENERAL_PREFIX, ADDR_RESULT_DATA}); read_data = tb_read_data; success = success & assertEquals(32'h0343b597, read_data);

      if (success !== 1)
        begin
          $display("*** ERROR: rob_enc_1024 was NOT successful.");
          error_ctr = error_ctr + 1;
        end
      else
        $display("*** rob_enc_1024 success.");
    end
  endtask // rob_enc_1024


  //----------------------------------------------------------------
  // main
  //
  // The main test functionality.
  //----------------------------------------------------------------
  initial
    begin : main

      $display("   -= Testbench for modexp started =-");
      $display("    =================================");
      $display("");

      init_sim();
      dump_dut_state();
      reset_dut();
      dump_dut_state();

//      tc1();
//      tc2();
//      tc3();
//      autogenerated_BASIC_33bit();
//      autogenerated_BASIC_128bit();
//      e64bit_64bit_modulus();
//      e65537_64bit_modulus();
//      e65537_64bit_modulus_elength();
//      e65537_128bit_modulus();
//      e65537_256bit_modulus();
//      e65537_1024bit_modulus();
//      e65537_1536bit_modulus();
//      e65537_1664bit_modulus();
      e65537_2048bit_modulus();
//      rob_dec_1024();
//      rob_enc_1024();

      display_test_results();

      $display("");
      $display("*** modexp simulation done. ***");
      $finish;
    end // main
endmodule // tb_modexp

//======================================================================
// EOF tb_modexp.v
//======================================================================

