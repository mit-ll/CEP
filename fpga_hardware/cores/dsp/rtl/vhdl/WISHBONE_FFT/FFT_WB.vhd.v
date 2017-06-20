// File FFT_WB.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
// vhd2vl settings:
//  * Verilog Module Declaration Style: 1995

// vhd2vl is Free (libre) Software:
//   Copyright (C) 2001 Vincenzo Liguori - Ocean Logic Pty Ltd
//     http://www.ocean-logic.com
//   Modifications Copyright (C) 2006 Mark Gonzales - PMC Sierra Inc
//   Modifications (C) 2010 Shankar Giri
//   Modifications Copyright (C) 2002, 2005, 2008-2010 Larry Doolittle - LBNL
//     http://doolittle.icarus.com/~larry/vhd2vl/
//
//   vhd2vl comes with ABSOLUTELY NO WARRANTY.  Always check the resulting
//   Verilog for correctness, ideally with a formal verification tool.
//
//   You are welcome to redistribute vhd2vl under certain conditions.
//   See the license (GPLv2) file included with the source for details.

// The result of translation follows.  Its copyright status should be
// considered unchanged from the original VHDL.

// no timescale needed

module FFT_WB(
DAT_I,
DAT_O,
ADR_I,
STB_I,
RST_I,
CLK_I,
WE_I,
ACK_O
);

parameter [31:0] WB_Width=32;
parameter [31:0] Adress_wordwidth=32;
parameter [31:0] N=1024;
parameter [31:0] Log2N=10;
parameter [31:0] reg_control=0;
parameter [31:0] reg_data=4;
parameter [31:0] reg_status=8;
parameter [31:0] reg_memory=12;
input [WB_Width - 1:0] DAT_I;
output [WB_Width - 1:0] DAT_O;
input [Adress_wordwidth - 1:0] ADR_I;
input STB_I, RST_I, CLK_I, WE_I;
output ACK_O;

wire [WB_Width - 1:0] DAT_I;
wire [WB_Width - 1:0] DAT_O;
wire [Adress_wordwidth - 1:0] ADR_I;
wire STB_I;
wire RST_I;
wire CLK_I;
wire WE_I;
wire ACK_O;


wire [Log2N - 1:0] index_aux;
wire frame_ready_aux; wire enable_out_aux; wire FFT_enable_aux; wire clear_aux;
wire [WB_Width - 1:0] Data1_aux; wire [WB_Width - 1:0] Data2_aux;

  //Instancia del FFT-Core
  fft_core_pipeline1 #(
      .input_width(WB_Width / 2),
    .twiddle_width(WB_Width / 2),
    .N(N),
    .add_g(0),
    .mult_g(0))
  FFT(
      .clock(CLK_I),
    .resetn(RST_I),
    .enable(FFT_enable_aux),
    .clear(clear_aux),
    .enable_out(enable_out_aux),
    .frame_ready(frame_ready_aux),
    .index(index_aux),
    .xin_r(Data1_aux[WB_Width - 1:WB_Width / 2]),
    .xin_i(Data1_aux[((WB_Width / 2)) - 1:0]),
    .Xout_r(Data2_aux[WB_Width - 1:WB_Width / 2]),
    .Xout_i(Data2_aux[((WB_Width / 2)) - 1:0]));

  //Instancia de interfaz con arbitro Wishbone 
  interface_slave_fft #(
      .N(N),
    .data_wordwidth(WB_Width),
    .adress_wordwidth(Adress_wordwidth),
    .reg_control(reg_control),
    .reg_data(reg_data),
    .reg_status(reg_status),
    .reg_memory(reg_memory))
  Interface(
      .ACK_O(ACK_O),
    .ADR_I(ADR_I),
    .ADR_FFT(index_aux),
    .DAT_I(DAT_I),
    .sDAT_I(Data2_aux),
    .DAT_O(DAT_O),
    .sDAT_O(Data1_aux),
    .STB_I(STB_I),
    .WE_I(WE_I),
    .FFT_finish_in(frame_ready_aux),
    .FFT_enable(FFT_enable_aux),
    .enable_in(enable_out_aux),
    .clear_out(clear_aux),
    .clk(CLK_I));


endmodule
