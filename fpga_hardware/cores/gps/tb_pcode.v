//------------------------------------------------------------------
// Simulator directives.
//------------------------------------------------------------------
`timescale 1ns/100ps

//------------------------------------------------------------------
// Test module.
//------------------------------------------------------------------
module tb_pcode();

//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------
// Debug output control.
parameter DEBUG                  = 0;
parameter VCD                    = 1;
// Clock defines.
localparam CLK_HALF_PERIOD       = 1;
localparam CLK_PERIOD            = 2 * CLK_HALF_PERIOD;

localparam SAT_WIDTH = 6;
localparam SREG_WIDTH = 37;
localparam XREG_WIDTH = 12;
//localparam PREG_WIDTH = 32;

//----------------------------------------------------------------
// Register and Wire declarations.
//----------------------------------------------------------------
reg  [31 : 0] error_ctr;
reg  [31 : 0] tc_ctr;

reg                  tb_clk;
reg                  tb_rst;
reg                  tb_en;
reg [ SAT_WIDTH-1:0] tb_sat;
reg [XREG_WIDTH-1:0] tb_x1a;
reg [XREG_WIDTH-1:0] tb_x1b;
reg [XREG_WIDTH-1:0] tb_x2a;
reg [XREG_WIDTH-1:0] tb_x2b;
`ifdef PREG_WIDTH
wire [PREG_WIDTH-1:0]tb_preg;
`else
wire tb_preg;
`endif

//----------------------------------------------------------------
// Device Under Test.
//----------------------------------------------------------------
pcode dut(
    .clk(tb_clk),
    .rst(tb_rst),
    .en(tb_en),
    .sat(tb_sat),
    .preg(tb_preg)
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
// rst_dut()
//
// Toggle rst to put the DUT into a well known state.
//----------------------------------------------------------------
task rst_dut();
    begin
        $display("*** Toggle rst.");
        tb_rst = 1;

        #(2 * CLK_PERIOD);
        tb_rst = 0;
        $display("");
    end
endtask // rst_dut

//----------------------------------------------------------------
// init_sim()
//
// Initialize all counters and testbed functionality as well
// as setting the DUT inputs to defined values.
//----------------------------------------------------------------
task init_sim();
    begin
        tb_clk  = 0;
        tb_rst  = 0;
        tb_en   = 0;
        tb_sat  = 6'b000000;
    end
endtask // init_sim

//----------------------------------------------------------------
// timing_verification
//----------------------------------------------------------------
task timing_verification();
    begin
        tb_rst=1;
        #(2*CLK_PERIOD);
        tb_rst=0;

        tb_en = 1;
        #(1000*CLK_PERIOD);
        tb_en = 0;
        #(1000*CLK_PERIOD);
        tb_en = 1;
        #(1000*CLK_PERIOD);

        #(100000*CLK_PERIOD);
/*
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
        #(100000*CLK_PERIOD);
*/
end
endtask // timing_verification

//----------------------------------------------------------------
// main
//----------------------------------------------------------------
initial
begin : main
    if(VCD) begin
        $dumpfile("./iverilog/tb_pcode.vcd");
        $dumpvars(0,tb_pcode);
        //$dumpvars(1,tb_clk, tb_rst, tb_cs, tb_we, tb_address, tb_write_data, tb_read_data);
    end

    $display("   -= Testbench for pcode started =-");
    $display("    =================================");
    $display("");

    init_sim();
    rst_dut();

    timing_verification();
    
    $display("");
    $display("*** pcode simulation done. ***");
    $finish;
end // main
endmodule // tb_modexp

//======================================================================
// EOF tb_modexp.v
//======================================================================
