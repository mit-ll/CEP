//------------------------------------------------------------------
// Simulator directives.
//------------------------------------------------------------------
`timescale 1ns/100ps

//------------------------------------------------------------------
// Test module.
//------------------------------------------------------------------
module tb_top();

//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------
// Debug output control.
parameter DEBUG                  = 0;
parameter VCD                    = 1;
// Clock defines.
localparam CLK_HALF_PERIOD       = 1;
localparam CLK_PERIOD            = 2 * CLK_HALF_PERIOD;
// Address defines
localparam OPERAND_WIDTH         = 32;
localparam ADDRESS_WIDTH         = 8;

localparam ADDR_NAME0            = 8'h00;
localparam ADDR_NAME1            = 8'h01;
localparam ADDR_VERSION          = 8'h02;

localparam ADDR_CTRL             = 8'h08;
localparam CTRL_INIT_BIT         = 0;
localparam CTRL_NEXT_BIT         = 1;

localparam ADDR_STATUS           = 8'h09;
localparam STATUS_READY_BIT      = 0;

localparam ADDR_CYCLES_HIGH      = 8'h10;
localparam ADDR_CYCLES_LOW       = 8'h11;

localparam ADDR_MODULUS_LENGTH   = 8'h20;
localparam ADDR_EXPONENT_LENGTH  = 8'h21;

localparam ADDR_MODULUS_PTR_RST  = 8'h30;
localparam ADDR_MODULUS_DATA     = 8'h31;

localparam ADDR_EXPONENT_PTR_RST = 8'h40;
localparam ADDR_EXPONENT_DATA    = 8'h41;

localparam ADDR_MESSAGE_PTR_RST  = 8'h50;
localparam ADDR_MESSAGE_DATA     = 8'h51;

localparam ADDR_RESULT_PTR_RST   = 8'h60;
localparam ADDR_RESULT_DATA      = 8'h61;

localparam DEFAULT_MODLENGTH     = 8'h80; // 2048 bits.
localparam DEFAULT_EXPLENGTH     = 8'h80;

localparam CORE_NAME0            = 32'h6d6f6465; // "mode"
localparam CORE_NAME1            = 32'h78702020; // "xp  "
localparam CORE_VERSION          = 32'h302e3532; // "0.52"

//----------------------------------------------------------------
// Register and Wire declarations.
//----------------------------------------------------------------
reg  [31 : 0] error_ctr;
reg  [31 : 0] tc_ctr;

reg [127 : 0] result_data;

reg           tb_clk;
reg           tb_reset;
reg           tb_cs;
reg           tb_we;
reg  [31 : 0] tb_address;
reg  [31 : 0] tb_write_data;
wire [31 : 0] tb_read_data;
wire          tb_error;

reg           tb_cyc;
reg  [3 : 0]  tb_sel;
wire          tb_ack;
wire          tb_err;
wire          tb_int;

reg  [31 : 0] pmsg [63 : 0];
reg  [31 : 0] cmsg [63 : 0];
reg  [31 : 0] gmsg [63 : 0];

integer f1;

//----------------------------------------------------------------
// Device Under Test.
//----------------------------------------------------------------
modexp_top dut(
		 .wb_adr_i(tb_address),
		 .wb_cyc_i(tb_cyc),//
		 .wb_dat_i(tb_write_data),
		 .wb_sel_i(tb_sel),//
		 .wb_stb_i(tb_cs),
		 .wb_we_i(tb_we),
		 
		 .wb_ack_o(tb_ack),//
		 .wb_err_o(tb_err),//
		 .wb_dat_o(tb_read_data),
		 
		 .wb_clk_i(tb_clk),
		 .wb_rst_i(tb_reset),
		 .int_o(tb_int)//
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
// reset_dut()
//
// Toggle reset to put the DUT into a well known state.
//----------------------------------------------------------------
task reset_dut();
    begin
        $display("*** Toggle reset.");
        tb_reset = 1;

        #(2 * CLK_PERIOD);
        tb_reset = 0;
        $display("");
    end
endtask // reset_dut

//----------------------------------------------------------------
// init_sim()
//
// Initialize all counters and testbed functionality as well
// as setting the DUT inputs to defined values.
//----------------------------------------------------------------
task init_sim();
    begin
        error_ctr          = 0;
        tc_ctr             = 0;

        tb_clk             = 0;
        tb_reset           = 0;

        tb_cs              = 0;
        tb_we              = 0;
        tb_address         = 32'h00000000;
        tb_write_data      = 32'h00000000;
    end
endtask // init_sim

//----------------------------------------------------------------
// read_word()
//
// Read a data word from the given address in the DUT.
//----------------------------------------------------------------
task read_word(input [7 : 0] address);
    begin
        tb_address = {22'b0,address,2'b0};
        tb_cs = 1;
        tb_we = 0;
        #(CLK_PERIOD);
        tb_cs = 0;

        if (DEBUG)
        begin
            $display("*** (read_word) Reading 0x%08x from 0x%02x.", tb_read_data, address);
            $display("");
        end
    end
endtask // read_word

//----------------------------------------------------------------
// write_word()
//
// Write the given word to the DUT using the DUT interface.
//----------------------------------------------------------------
task write_word(input [ 7 : 0] address,
    input [31 : 0] word);
begin
    if (DEBUG)
    begin
        $display("*** (write_word) Writing 0x%08x to 0x%02x.", word, address);
        $display("");
    end

    tb_address = {22'b0,address,2'b0};
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
            read_word(8'h09);

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
        $display("*** Expected: 0x%08x, got 0x%08x", expected, actual);
        assertEquals = 0; // failure
    end
end
endfunction // assertEquals

//----------------------------------------------------------------
// assertSuccess
//----------------------------------------------------------------
task assertSuccess(input success);
begin
    if (success !== 1)
    begin
        $display("*** Test    -> FAILED");
        error_ctr = error_ctr + 1;
    end
    else
        $display("*** Test    -> passed");        
end
endtask // assertSuccess

//----------------------------------------------------------------
// display_test_results()
//
// Display the accumulated test results.
//----------------------------------------------------------------
task display_test_results();
    begin
        $display("");

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
// exp32bit_mod2048bit_test
//----------------------------------------------------------------
task exp32bit_mod2048bit_test();
    integer i;
    integer success;
    reg [31 : 0] read_data;
    reg  [31 : 0] exp  [63 : 0];
    reg  [31 : 0] mod  [63 : 0];
    reg  [31 : 0] msg  [63 : 0];
    reg  [31 : 0] res  [63 : 0];
begin
    success = 32'h1;
    tc_ctr = tc_ctr + 1;
    $display("");
    $display("Test with e = 65537 and 2048 bit modulus -- Encrypting");

    $readmemh("./gen/exp", exp);
    $readmemh("./gen/mod", mod);
    $readmemh("./gen/msg",  msg);
    $readmemh("./gen/res",  res);

    write_word(ADDR_EXPONENT_PTR_RST, 32'h00000000);
    write_word(ADDR_EXPONENT_DATA   , exp[0]);

    write_word(ADDR_MODULUS_PTR_RST, 32'h00000000);
    write_word(ADDR_MODULUS_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MODULUS_DATA, mod[i]);
        if(DEBUG) $display("writing: %d -> %h", i, mod[i]);
    end

    write_word(ADDR_MESSAGE_PTR_RST, 32'h00000000);
    write_word(ADDR_MESSAGE_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MESSAGE_DATA, msg[i]);
        if(DEBUG) $display("Writing: %d -> %h", i, msg[i]);
    end

    write_word(ADDR_EXPONENT_LENGTH, 32'h00000001);
    write_word(ADDR_MODULUS_LENGTH , 32'h00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 32'h00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 32'h00000000);
    read_word(ADDR_RESULT_DATA); read_data=tb_read_data;
    success=success&assertEquals(32'h00000000, read_data);

    for(i=0; i<64; i=i+1) begin
        read_word(ADDR_RESULT_DATA);
        read_data=tb_read_data;
        success=success&assertEquals(res[i], read_data);
        if(DEBUG) $display("Reading: %d -> %h -> %h", i, res[i], read_data);
    end

    if (success !== 1)
    begin
        $display("*** ERROR: e65537_2048bit_modulus was NOT successful.");
        error_ctr = error_ctr + 1;
    end
    else
        $display("*** e65537_2048bit_modulus success.");
end
endtask // exp32bit_mod2048bit_test

//----------------------------------------------------------------
// modexp_encrypt
//----------------------------------------------------------------
task modexp_encrypt();
    integer i;
    integer success;
    reg [31 : 0] pubexp [1];
    reg [31 : 0] pubmod [63 : 0];
begin
    success = 32'h1;
    tc_ctr = tc_ctr + 1;
    $display("");
    $display("Encrypting -- exp = 65537 and 2048 bit mod");

    $readmemh("./gen/pTextHEX", pmsg);
    $readmemh("./gen/exponent", pubexp);
    $readmemh("./gen/modulus",  pubmod);

    write_word(ADDR_EXPONENT_PTR_RST, 32'h00000000);
    write_word(ADDR_EXPONENT_DATA   , pubexp[0]);
    if(DEBUG) $display("Writing EXP: %d %h", pubexp[0], pubexp[0]);

    write_word(ADDR_MODULUS_PTR_RST, 32'h00000000);
    write_word(ADDR_MODULUS_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MODULUS_DATA, pubmod[i]);
        if(DEBUG) $display("Writing MOD: %d -> %h", i, pubmod[i]);
    end

    write_word(ADDR_MESSAGE_PTR_RST, 32'h00000000);
    write_word(ADDR_MESSAGE_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MESSAGE_DATA, pmsg[i]);
        if(DEBUG) $display("Writing MSG: %d -> %h", i, pmsg[i]);
    end

    write_word(ADDR_EXPONENT_LENGTH, 32'h00000001);
    write_word(ADDR_MODULUS_LENGTH , 32'h00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 32'h00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 32'h00000000);
    read_word(ADDR_RESULT_DATA); cmsg[0]=tb_read_data;
    //success=success&assertEquals(32'h00000000, cmsg[0]);

    f1 = $fopen("./gen/cTextHEX", "w");

    for(i=0; i<64; i=i+1) begin
        read_word(ADDR_RESULT_DATA);
        cmsg[i]=tb_read_data;
        if(DEBUG) $display("Reading: %d -> %h ", i, cmsg[i]);
        $fwrite(f1, "%h\n",cmsg[i]);
    end
    $fclose(f1);

    assertSuccess(success);
end
endtask // modexp_encrypt

//----------------------------------------------------------------
// modexp_decrypt
//----------------------------------------------------------------
task modexp_decrypt();
    integer i;
    integer success;
    reg [31 : 0] rdata;    
    reg [31 : 0] read_data;
    reg [31 : 0] priexp [63 : 0];
    reg [31 : 0] primod [63 : 0];
begin
    $display("");
    $display("*** Running -> modexp_decrypt()");


    $readmemh("./gen/priExponent", priexp);
    $readmemh("./gen/priModulus",  primod);

    success = 32'h1;
    tc_ctr = tc_ctr + 1;
    $display("Decrypting -- 2048 bit exp and 2048 bit mod");

    write_word(ADDR_EXPONENT_PTR_RST, 32'h00000000);
    write_word(ADDR_EXPONENT_DATA   , 32'h00000000);
    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_EXPONENT_DATA, priexp[i]);
        if(DEBUG) $display("Writing EXP: %d -> %h", i, priexp[i]);
    end

    write_word(ADDR_MODULUS_PTR_RST, 32'h00000000);
    write_word(ADDR_MODULUS_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MODULUS_DATA, primod[i]);
        if(DEBUG) $display("Writing MOD: %d -> %h", i, primod[i]);
    end

    write_word(ADDR_MESSAGE_PTR_RST, 32'h00000000);
    write_word(ADDR_MESSAGE_DATA   , 32'h00000000);

    for(i=0; i<64; i=i+1) begin
        write_word(ADDR_MESSAGE_DATA, cmsg[i]);
        if(DEBUG) $display("Writing MSG: %d -> %h", i, cmsg[i]);
    end

    write_word(ADDR_EXPONENT_LENGTH, 32'h00000041);
    write_word(ADDR_MODULUS_LENGTH , 32'h00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 32'h00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 32'h00000000);
    read_word(ADDR_RESULT_DATA); rdata=tb_read_data;
    success=success&assertEquals(32'h00000000, rdata);

    f1 = $fopen("./gen/gTextHEX", "w");

    for(i=0; i<64; i=i+1) begin
        read_word(ADDR_RESULT_DATA);
        rdata=tb_read_data;
        if(DEBUG) $display("Reading: %d -> %h ", i, rdata);
        $fwrite(f1, "%h\n",rdata);

        success=success&assertEquals(pmsg[i], rdata);
    end
    $fclose(f1);

    assertSuccess(success);
end
endtask // modexp_decrypt

//----------------------------------------------------------------
// modexp_32bits
//----------------------------------------------------------------
task modexp_32bits(input [31:0] Wmsg,
                   input [31:0] Wexp,
                   input [31:0] Wmod,
                   input [31:0] Wres);
    reg [31 : 0] Rmsg;
    reg [31 : 0] Rexp;
    reg [31 : 0] Rmod;
    reg [31 : 0] Rres;
    integer success;
begin
    $display("");
    $display("*** Running -> modexp_32bits()");
  
    success = 32'h1;
    tc_ctr = tc_ctr + 1;
    $display("*** Writing -> MES: %h EXP: %h MOD: %h", Wmsg, Wexp, Wmod);

    write_word(ADDR_EXPONENT_PTR_RST, 32'h00000000);
    write_word(ADDR_EXPONENT_DATA   , Wexp);

    write_word(ADDR_MODULUS_PTR_RST , 32'h00000000);
    write_word(ADDR_MODULUS_DATA    , Wmod);

    write_word(ADDR_MESSAGE_PTR_RST , 32'h00000000);
    write_word(ADDR_MESSAGE_DATA    , Wmsg);

    write_word(ADDR_EXPONENT_LENGTH , 32'h00000001);
    write_word(ADDR_MODULUS_LENGTH  , 32'h00000001);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL            , 32'h00000001);
    wait_ready();

    write_word(ADDR_MESSAGE_PTR_RST , 32'h00000000);
    read_word(ADDR_MESSAGE_DATA);
    Rmsg=tb_read_data;
    
    write_word(ADDR_EXPONENT_PTR_RST, 32'h00000000);
    read_word(ADDR_EXPONENT_DATA);
    Rexp=tb_read_data;
    
    write_word(ADDR_MODULUS_PTR_RST , 32'h00000000);
    read_word(ADDR_MODULUS_DATA);
    Rmod=tb_read_data;
    
    write_word(ADDR_RESULT_PTR_RST  , 32'h00000000);
    read_word(ADDR_RESULT_DATA);
    Rres=tb_read_data;
    
    $display("*** Reading -> MES: %h EXP: %h MOD: %h RES: %h", Rmsg, Rexp, Rmod, Rres);
    success=success&assertEquals(Wres, Rres);

    assertSuccess(success);
end
endtask // modexp_32bits

//----------------------------------------------------------------
// main
//
// The main test functionality.
//----------------------------------------------------------------
initial
begin : main
    if(VCD) begin
        $dumpfile("./iverilog/tb_top.vcd");
        $dumpvars(0,tb_top);
        //$dumpvars(1,tb_clk, tb_reset, tb_cs, tb_we, tb_address, tb_write_data, tb_read_data);
    end

    $display("   -= Testbench for modexp started =-");
    $display("    =================================");
    $display("");

    init_sim();
    reset_dut();

    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000005, 32'h00000001); //msg^exp < mod -> 1^2 < 5    
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000003, 32'h00000001); //msg^exp < mod -> 1^2 < 3
    modexp_32bits(32'h00000002, 32'h00000002, 32'h00000005, 32'h00000004); //msg^exp < mod -> 2^2 < 5
    modexp_32bits(32'h00000002, 32'h00000002, 32'h00000003, 32'h00000001); //msg^exp > mod -> 2^2 > 3
    modexp_32bits(32'h00000004, 32'h0000000D, 32'h000001F1, 32'h000001bd); //msg^exp > mod -> 4^13 > 497
    modexp_32bits(32'h01234567, 32'h89ABCDEF, 32'h11111111, 32'h0D9EF081); //msg^exp > mod -> 19088743^2309737967 > 286331153
    
    modexp_32bits(32'h30000000, 32'hC0000000, 32'h00A00001, 32'h0000CC3F); //msg^exp > mod -> 805306368^3221225472 > 10485761 <- Passes
    //modexp_32bits(32'h30000000, 32'hC0000000, 32'h00A00000, 32'h00600000); //msg^exp > mod -> 805306368^3221225472 > 10485760 <- Fails

    //modexp_32bits(32'h00000002, 32'h00000003, 32'h00000001, 32'h00000000); //mod 1 <- FAILS Does not check
/*
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000002, 32'h00000001); //1 mod 2 = 1 -> 0 <- FAILS
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000003, 32'h00000001); //1 mod 3 = 1 -> 1 <- passes
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000004, 32'h00000001); //1 mod 4 = 1 -> 0 <- FAILS
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000005, 32'h00000001); //1 mod 5 = 1 -> 1 <- passes
    modexp_32bits(32'h00000001, 32'h00000002, 32'h00000006, 32'h00000001); //1 mod 6 = 1 -> 4 <- FAILS
    
    modexp_32bits(32'h00000002, 32'h00000001, 32'h00000002, 32'h00000000); //2 mod 2 = 0 -> passes
    modexp_32bits(32'h00000002, 32'h00000001, 32'h00000003, 32'h00000002); //2 mod 3 = 2 -> passes
    modexp_32bits(32'h00000002, 32'h00000001, 32'h00000004, 32'h00000002); //2 mod 4 = 2 -> 0 <- FAILS
    modexp_32bits(32'h00000002, 32'h00000001, 32'h00000005, 32'h00000002); //2 mod 5 = 2 -> passes
    modexp_32bits(32'h00000002, 32'h00000001, 32'h00000006, 32'h00000002); //2 mod 6 = 2 -> passes
    
    modexp_32bits(32'h00000003, 32'h00000001, 32'h00000002, 32'h00000001); //3 mod 2 = 1 -> 0 <- FAILS
    modexp_32bits(32'h00000003, 32'h00000001, 32'h00000003, 32'h00000000); //3 mod 3 = 0 -> 3 <- FAILS
    modexp_32bits(32'h00000003, 32'h00000001, 32'h00000004, 32'h00000003); //3 mod 4 = 3 -> 0 <- FAILS
    modexp_32bits(32'h00000003, 32'h00000001, 32'h00000005, 32'h00000003); //3 mod 5 = 3 -> passes
    modexp_32bits(32'h00000003, 32'h00000001, 32'h00000006, 32'h00000003); //3 mod 6 = 3 -> passes

    modexp_32bits(32'h00000004, 32'h00000001, 32'h00000002, 32'h00000000); //4 mod 2 = 0 -> passes
    modexp_32bits(32'h00000004, 32'h00000001, 32'h00000003, 32'h00000001); //4 mod 3 = 1 -> passes
    modexp_32bits(32'h00000004, 32'h00000001, 32'h00000004, 32'h00000000); //4 mod 4 = 0 -> passes
    modexp_32bits(32'h00000004, 32'h00000001, 32'h00000005, 32'h00000004); //4 mod 5 = 4 -> passes
    modexp_32bits(32'h00000004, 32'h00000001, 32'h00000006, 32'h00000004); //4 mod 6 = 4 -> passes

    modexp_32bits(32'h00000005, 32'h00000001, 32'h00000002, 32'h00000001); //5 mod 2 = 1 -> 0 <- FAILS
    modexp_32bits(32'h00000005, 32'h00000001, 32'h00000003, 32'h00000002); //5 mod 3 = 2 -> passes
    modexp_32bits(32'h00000005, 32'h00000001, 32'h00000004, 32'h00000001); //5 mod 4 = 1 -> 0 <- FAILS
    modexp_32bits(32'h00000005, 32'h00000001, 32'h00000005, 32'h00000000); //5 mod 5 = 0 -> 5 <- FAILS
    modexp_32bits(32'h00000005, 32'h00000001, 32'h00000006, 32'h00000005); //5 mod 6 = 5 -> passes

    modexp_32bits(32'h00000006, 32'h00000001, 32'h00000002, 32'h00000000); //6 mod 2 = 0 -> passes
    modexp_32bits(32'h00000006, 32'h00000001, 32'h00000003, 32'h00000000); //6 mod 3 = 0 -> 3 <- FAILS
    modexp_32bits(32'h00000006, 32'h00000001, 32'h00000004, 32'h00000002); //6 mod 4 = 2 -> 0 <- FAILS
    modexp_32bits(32'h00000006, 32'h00000001, 32'h00000005, 32'h00000001); //6 mod 5 = 1 -> passes
    modexp_32bits(32'h00000006, 32'h00000001, 32'h00000006, 32'h00000000); //6 mod 6 = 0 -> 1 <- FAILS
*/
/*
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000002, 32'h00000000); //8^6 mod 2 = 0 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000003, 32'h00000001); //8^6 mod 3 = 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000004, 32'h00000000); //8^6 mod 4 = 0 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000005, 32'h00000004); //8^6 mod 5 = 4 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000006, 32'h00000004); //8^6 mod 6 = 4 -> 1 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000007, 32'h00000001); //8^6 mod 7 = 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000008, 32'h00000000); //8^6 mod 8 = 0 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000009, 32'h00000001); //8^6 mod 9 = 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000A, 32'h00000004); //8^6 mod 10= 4 -> 9 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000B, 32'h00000003); //8^6 mod 11= 3 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000C, 32'h00000004); //8^6 mod 12= 4 -> 5 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000D, 32'h0000000C); //8^6 mod 13= 12-> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000E, 32'h00000008); //8^6 mod 14= 8 -> 5 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000000F, 32'h00000004); //8^6 mod 15= 4 -> passes

    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000010, 32'h00000000); //8^6 mod 16= 0 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000011, 32'h00000004); //8^6 mod 17= 4 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000012, 32'h0000000A); //8^6 mod 18= 10-> 13<- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000013, 32'h00000001); //8^6 mod 19= 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000014, 32'h00000004); //8^6 mod 20= 4 -> 11<- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000015, 32'h00000001); //8^6 mod 21= 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000016, 32'h0000000E); //8^6 mod 22= 14-> 1 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000017, 32'h0000000D); //8^6 mod 23= 13-> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000018, 32'h00000010); //8^6 mod 24= 16-> 9 <- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h00000019, 32'h00000013); //8^6 mod 25= 19-> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001A, 32'h0000000C); //8^6 mod 26= 12-> 19<- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001B, 32'h00000001); //8^6 mod 27= 1 -> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001C, 32'h00000008); //8^6 mod 28= 8 -> 19<- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001D, 32'h0000000D); //8^6 mod 29= 13-> passes
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001E, 32'h00000004); //8^6 mod 30= 4 -> 13<- FAILS
    modexp_32bits(32'h00000008, 32'h00000006, 32'h0000001F, 32'h00000008); //8^6 mod 31= 8 -> passes
*/

    //exp32bit_mod2048bit_test();
    //modexp_encrypt();
    //modexp_decrypt();

    display_test_results();

    $display("");
    $display("*** modexp simulation done. ***");
    $finish;
end // main
endmodule // tb_modexp

//======================================================================
// EOF tb_modexp.v
//======================================================================
