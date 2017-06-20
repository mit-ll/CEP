// File interface_slave_fft.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//use work.fft_pkg.all;
// no timescale needed

module interface_slave_fft(
ACK_O,
ADR_I,
ADR_FFT,
DAT_I,
sDAT_I,
DAT_O,
sDAT_O,
STB_I,
WE_I,
FFT_finish_in,
FFT_enable,
enable_in,
clear_out,
clk
);

parameter [31:0] N=1024;
parameter [31:0] Log2N=10;
parameter [31:0] Log2N3=13;
parameter [31:0] data_wordwidth=32;
parameter [31:0] adress_wordwidth=32;
parameter [31:0] reg_control=0;
parameter [31:0] reg_data=4;
parameter [31:0] reg_status=8;
parameter [31:0] reg_memory=12;
output ACK_O;
//to MASTER
input [adress_wordwidth - 1:0] ADR_I;
input [Log2N - 1:0] ADR_FFT;
input [data_wordwidth - 1:0] DAT_I;
//from MASTER
input [data_wordwidth - 1:0] sDAT_I;
//from SLAVE
output [data_wordwidth - 1:0] DAT_O;
//to MASTER
output [data_wordwidth - 1:0] sDAT_O;
//to SLAVE
input STB_I;
//from MASTER
input WE_I;
//from MASTER
input FFT_finish_in;
//from SLAVE	
output FFT_enable;
//to SLAVE	
input enable_in;
//from SLAVE	
output clear_out;
//to SLAVE
input clk;

wire ACK_O;
wire [adress_wordwidth - 1:0] ADR_I;
wire [Log2N - 1:0] ADR_FFT;
wire [data_wordwidth - 1:0] DAT_I;
wire [data_wordwidth - 1:0] sDAT_I;
reg [data_wordwidth - 1:0] DAT_O;
reg [data_wordwidth - 1:0] sDAT_O;
wire STB_I;
wire WE_I;
wire FFT_finish_in;
reg FFT_enable;
wire enable_in;
reg clear_out;
wire clk;


wire [Data_wordwidth - 1:0] OUT_AUX; wire [Data_wordwidth - 1:0] OUT_AUX1; wire [Data_wordwidth - 1:0] ZERO;
wire [Log2N3 - 1:0] ADD_aux;
reg ack_r; wire ack_w;

  assign OUT_AUX = DAT_I;
  assign ZERO = ((0));
  assign ACK_O = ack_r | ack_w;
  //Reconocimiento de escritura sin estado de espera
  assign ack_w = (STB_I == 1'b 1 && WE_I == 1'b 1) ? 1'b 1 : 1'b 0;
  //Reconocimiento de lectura con 1 estado de espera, caso RAM sincrona
  always @(posedge clk) begin
    if((STB_I == 1'b 1 && WE_I == 1'b 0)) begin
      ack_r <= 1'b 1;
    end
    else begin
      ack_r <= 1'b 0;
    end
  end

  //Se elimina offset para direcciones de lectura de memoria RAM 
  assign ADD_aux = (((ADR_I[Log2N3 - 1:0])) - ((reg_memory)));
  RAM_Memory #(
      .Add_WordWidth(Log2N),
    .Data_WordWidth(Data_WordWidth))
  RAM(
      .DATi(sDAT_I),
    .DATo(OUT_AUX1),
    //Divide entre cuatro, direcciones alineadas cada 4 bytes
    .ADR_WB(ADD_aux[Log2N + 1:2]),
    .ADR_FFT(ADR_FFT[Log2N - 1:0]),
    .W_R(enable_in),
    .clk(clk));

  //Decodificador de escritura
  always @(ADR_I or STB_I or WE_I or ZERO or OUT_AUX or OUT_AUX1 or FFT_finish_in) begin
    if((WE_I == 1'b 1 && STB_I == 1'b 1)) begin
      //ESCRIBIR EN FFT
      case(ADR_I[Log2N3 - 1:0])
        Reg_control: begin
          clear_out<='1';
          FFT_enable<='1';												                 sDAT_O<=ZERO;						
        end
        Reg_data,Log2N3)): begin
          sDAT_O<=OUT_AUX;
          FFT_enable<='1';
          clear_out<='0';
        end
        default : begin
          sDAT_O <= ZERO;
          clear_out <= 1'b 0;
          FFT_enable <= 1'b 0;
        end
      endcase
    end
    else begin
      sDAT_O <= ZERO;
      clear_out <= 1'b 0;
      FFT_enable <= 1'b 0;
    end
  end

  //Decodificador de lectura
  always @(ADR_I or STB_I or WE_I or ZERO or OUT_AUX or OUT_AUX1 or FFT_finish_in) begin
    if((WE_I == 1'b 0 && STB_I == 1'b 1)) begin
      if(ADR_I[Log2N3 - 1:0] == (((Reg_status)))) begin
        DAT_O[0] <= FFT_finish_in;
        DAT_O[Data_wordwidth - 1:1] <= ZERO[Data_wordwidth - 1:1];
      end
      else begin
        DAT_O <= OUT_AUX1;
      end
    end
    else begin
      DAT_O <= ZERO;
    end
  end


endmodule
