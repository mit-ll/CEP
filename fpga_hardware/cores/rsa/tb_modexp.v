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
  reg  [11 : 0] tb_address;
  reg  [31 : 0] tb_write_data;
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
          read_word({4'h0, 8'h09});

      if (DEBUG)
        $display("*** (wait_ready) Ready flag has been set.");
    end
  endtask // wait_ready

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

      write_word({4'h0, 8'h40}, 32'h00000000); //ADDR_EXPONENT_PTR_RST
      write_word({4'h0, 8'h41}, 32'h000000ff); //ADDR_EXPONENT_DATA

      write_word({4'h0, 8'h30}, 32'h00000000); //ADDR_MODULUS_PTR_RST
      write_word({4'h0, 8'h31}, 32'h00000000); //ADDR_MODULUS_DATA
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //1
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //2
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //3
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //4
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //5
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //6
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //7
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //8
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //9
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //10
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //11
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //12
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //13
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //14
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //15
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //16
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //17
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //18
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //19
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //20
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //21
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //22
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //23
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //24
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //25
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //26
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //27
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //28
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //29
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //30
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //31
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //32
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //33
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //34
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //35
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //36
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //37
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //38
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //39
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //40
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //41
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //42
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //43
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //44
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //45
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //46
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //47
      write_word({4'h0, 8'h31}, 32'h00000000); //32'hffeeffef); //48
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //49
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //50
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //51
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //52
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //53
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //54
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //55
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //56
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //57
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //58
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //59
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //60
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //61
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //62
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //63
      write_word({4'h0, 8'h31}, 32'hffffffff); //32'hffeeffef); //64


      write_word({4'h0, 8'h50}, 32'h00000000); //ADDR_MESSAGE_PTR_RST
      write_word({4'h0, 8'h51}, 32'h00000000); //ADDR_MESSAGE_DATA
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //1
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //2
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //3
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //4
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //5
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //6
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //7
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //8
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //9
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //10
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //11
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //12
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //13
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //14
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //15
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //16
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //17
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //18
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //19
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //20
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //21
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //22
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //23
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //24
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //25
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //26
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //27
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //28
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //29
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //30
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //31
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //32
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //33
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //34
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //35
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //36
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //37
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //38
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //39
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //40
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //41
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //42
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //43
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //44
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //45
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //46
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //47
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //48
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //49
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //50
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //51
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //52
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //53
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //54
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hffaabbcc); //55
      write_word({4'h0, 8'h51}, 32'h00000000); //32'hddeeffff); //56
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hffaabbcc); //57
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hddeeffff); //58
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hffaabbcc); //59
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hddeeffff); //60
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hffaabbcc); //61
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hddeeffff); //62
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hffaabbcc); //63
      write_word({4'h0, 8'h51}, 32'hffffffff); //32'hddeeffff); //64

      write_word({4'h0, 8'h21}, 32'h00000001); //ADDR_EXPONENT_LENGTH
      write_word({4'h0, 8'h20}, 32'h00000041); //ADDR_MODULUS_LENGTH

      start_test_cycle_ctr();

      // Start processing and wait for ready.
      write_word({4'h0, 8'h08}, 32'h00000001);
      wait_ready();

      stop_test_cycle_ctr();

      write_word({4'h0, 8'h60}, 32'h00000000);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h00000000, read_data);$display("Result: 00 -> 0x%08x", read_data);

      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 01 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 02 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 03 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 04 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 05 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 06 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 07 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 08 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 09 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 10 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 11 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 12 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 13 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 14 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 15 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 16 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 17 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 18 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 19 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 20 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 21 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 22 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 23 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 24 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 25 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 26 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 27 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 28 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 29 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 30 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 31 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 32 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 33 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 34 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 35 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 36 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 37 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 38 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 39 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 40 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 41 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 42 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 43 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 44 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 45 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 46 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 47 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 48 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 49 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 50 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 51 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 52 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 53 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 54 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 55 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 56 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 57 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 58 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 59 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 60 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 61 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 62 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'hf1752196, read_data);$display("Result: 63 -> 0x%08x", read_data);
      read_word({4'h0, 8'h61}); read_data=tb_read_data; success=success&assertEquals(32'h4c36e92f, read_data);$display("Result: 64 -> 0x%08x", read_data);

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
  // main
  //
  // The main test functionality.
  //----------------------------------------------------------------
  initial
    begin : main
//$dumpfile("tb_modexp.vcd");
//$dumpvars(0,tb_modexp);
      $display("   -= Testbench for modexp started =-");
      $display("    =================================");
      $display("");

      init_sim();
      dump_dut_state();
      reset_dut();
      dump_dut_state();

      e65537_2048bit_modulus();

      display_test_results();

      $display("");
      $display("*** modexp simulation done. ***");
      $finish;
    end // main
endmodule // tb_modexp

//======================================================================
// EOF tb_modexp.v
//======================================================================
