/*****************************************************************
 Pancham is an MD5 compliant IP core for cryptographic 
 applications. 
 Copyright (C) 2003  Swapnajit Mittra, Project VeriPage
 (Contact email: verilog_tutorial at hotmail.com
  Website      : http://www.angelfire.com/ca/verilog)
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the 
 
 Free Software Foundation, Inc.
 59 Temple Place, Suite 330
 Boston, MA  02111-1307 
 USA
 ******************************************************************/
/*
 * This is the main module that computes a 128-bit message 
 * digest from a maximum of 512-bit long input message using
 * MD5 algorithm.
 *
 * Modified by Matthew Hicks (mdhicks@gmail.com)
 *   Input from 128-bit to 512-bit
 *   Hold valid outputs
 *   Carry over constants between encryptions
 *   Support for arbitrary size encryption
 *   Address some Verilator warnings
 *   Convert vector direction
 *
 */

// ROUNDs

`define ROUND1 2'b00
`define ROUND2 2'b01
`define ROUND3 2'b10
`define ROUND4 2'b11

// T_i = 4294967296*abs(sin(i))

`define T_1  32'hd76aa478
`define T_2  32'he8c7b756
`define T_3  32'h242070db
`define T_4  32'hc1bdceee
`define T_5  32'hf57c0faf
`define T_6  32'h4787c62a
`define T_7  32'ha8304613
`define T_8  32'hfd469501
`define T_9  32'h698098d8
`define T_10 32'h8b44f7af
`define T_11 32'hffff5bb1
`define T_12 32'h895cd7be
`define T_13 32'h6b901122
`define T_14 32'hfd987193
`define T_15 32'ha679438e
`define T_16 32'h49b40821
`define T_17 32'hf61e2562
`define T_18 32'hc040b340
`define T_19 32'h265e5a51
`define T_20 32'he9b6c7aa
`define T_21 32'hd62f105d
`define T_22  32'h2441453
`define T_23 32'hd8a1e681
`define T_24 32'he7d3fbc8
`define T_25 32'h21e1cde6
`define T_26 32'hc33707d6
`define T_27 32'hf4d50d87
`define T_28 32'h455a14ed
`define T_29 32'ha9e3e905
`define T_30 32'hfcefa3f8
`define T_31 32'h676f02d9
`define T_32 32'h8d2a4c8a
`define T_33 32'hfffa3942
`define T_34 32'h8771f681
`define T_35 32'h6d9d6122
`define T_36 32'hfde5380c
`define T_37 32'ha4beea44
`define T_38 32'h4bdecfa9
`define T_39 32'hf6bb4b60
`define T_40 32'hbebfbc70
`define T_41 32'h289b7ec6
`define T_42 32'heaa127fa
`define T_43 32'hd4ef3085
`define T_44  32'h4881d05
`define T_45 32'hd9d4d039
`define T_46 32'he6db99e5
`define T_47 32'h1fa27cf8
`define T_48 32'hc4ac5665
`define T_49 32'hf4292244
`define T_50 32'h432aff97
`define T_51 32'hab9423a7
`define T_52 32'hfc93a039
`define T_53 32'h655b59c3
`define T_54 32'h8f0ccc92
`define T_55 32'hffeff47d
`define T_56 32'h85845dd1
`define T_57 32'h6fa87e4f
`define T_58 32'hfe2ce6e0
`define T_59 32'ha3014314
`define T_60 32'h4e0811a1
`define T_61 32'hf7537e82
`define T_62 32'hbd3af235
`define T_63 32'h2ad7d2bb
`define T_64 32'heb86d391

/* verilator lint_off UNOPTFLAT */

module pancham (
           clk
           , rst
           , init     // Tony D. 05/12/20: to remove reset dependency		
           , msg_padded
           , msg_in_valid

           , msg_output
           , msg_out_valid
           , ready
       );

//--------------------------------
//
// Input/Output declarations
//
//--------------------------------
   input          clk;                      // input clock
   input 	  rst;                    // global rst
   input 	  init;   // clear internal state for new encryption                    
   input [511:0]  msg_padded;               // input message, already padded
   input          msg_in_valid;             // input message is valid, active high
   
   output [127:0] msg_output;               // output message, always 128 bit wide
   output         msg_out_valid;            // if asserted, output message is valid
   output         ready;                    // the core is ready for an input message

//--------------------------------
//
// Variable declarations
//
//--------------------------------
// inputs
wire          clk;
wire          rst;
wire  [511:0] msg_padded;
wire          msg_in_valid;
reg          msg_in_valid_reg;
wire         msg_in_valid_pos_edge;
// output
wire  [127:0] msg_output;
reg           msg_out_valid;
wire          ready;

// scratch pads
reg    [1:0] round;
reg   [31:0] a;
reg   [31:0] A;
reg   [31:0] AA;
reg   [31:0] next_A;
reg   [31:0] b;
reg   [31:0] B;
reg   [31:0] BB;
reg   [31:0] next_B;
reg   [31:0] c;
reg   [31:0] C;
reg   [31:0] CC;
reg   [31:0] next_C;
reg   [31:0] d;
reg   [31:0] D;
reg   [31:0] DD;
reg   [31:0] next_D;
reg   [31:0] m;
reg   [31:0] s;
reg   [31:0] t;
reg    [3:0] phase;         // Counter to determine 16 phases within each round.
wire  [31:0] next_a;
reg    [7:0] current_state;
reg    [7:0] next_state;
reg [8*11:1] ascii_state;

//--------------------------------
//
// Parameter definitions
//
//--------------------------------

parameter SALT_A         = 32'h67452301;
parameter SALT_B         = 32'hefcdab89;
parameter SALT_C         = 32'h98badcfe;
parameter SALT_D         = 32'h10325476;

parameter ONE            = 72'h1;

parameter IDLE_BIT       = 0;
parameter IDLE           = ONE << IDLE_BIT;

parameter ROUND1_BIT     = 1;
parameter ROUND1         = ONE << ROUND1_BIT;

parameter ROUND2_BIT     = 2;
parameter ROUND2         = ONE << ROUND2_BIT;

parameter ROUND3_BIT     = 3;
parameter ROUND3         = ONE << ROUND3_BIT;

parameter ROUND4_BIT     = 4;
parameter ROUND4         = ONE << ROUND4_BIT;

parameter FINISH_OFF_BIT = 5;
parameter FINISH_OFF     = ONE << FINISH_OFF_BIT;

parameter TURN_ARND_BIT  = 6;
parameter TURN_ARND      = ONE << TURN_ARND_BIT;


//--------------------------------
//
// Submodule instantiation
//
//--------------------------------
pancham_round ROUND (
                  .a      (a    )
                  , .b      (b    )
                  , .c      (c    )
                  , .d      (d    )
                  , .m      (m    )
                  , .s      (s    )
                  , .t      (t    )
                  , .round  (round)

                  , .next_a (next_a)
              );

wire [31:0] m0 = msg_padded[31:0];
wire [31:0] m1 = msg_padded[63:32];
wire [31:0] m2 = msg_padded[95:64];
wire [31:0] m3 = msg_padded[127:96];
wire [31:0] m4 = msg_padded[159:128];
wire [31:0] m5 = msg_padded[191:160];
wire [31:0] m6 = msg_padded[223:192];
wire [31:0] m7 = msg_padded[255:224];
wire [31:0] m8 = msg_padded[287:256];
wire [31:0] m9 = msg_padded[319:288];
wire [31:0] m10 = msg_padded[351:320];
wire [31:0] m11 = msg_padded[383:352];
wire [31:0] m12 = msg_padded[415:384];
wire [31:0] m13 = msg_padded[447:416];
wire [31:0] m14 = msg_padded[479:448];
wire [31:0] m15 = msg_padded[511:480];



always @ (posedge (clk))begin
    msg_in_valid_reg <= msg_in_valid;
end

assign msg_in_valid_pos_edge = msg_in_valid & ~msg_in_valid_reg;


//--------------------------------
//
// Actual code starts here
//
//--------------------------------

always @(current_state
             or    msg_in_valid_pos_edge
             or    A
             or    B
             or    C
             or    D
             or    phase
             or    msg_padded
             or    next_a
             or    AA
             or    BB
             or    CC
             or    DD

            )
    begin // {
        round = `ROUND1;
        next_A = A;
        next_B = B;
        next_C = C;
        next_D = D;

        a = 32'h0;
        b = 32'h0;
        c = 32'h0;
        d = 32'h0;
        m = 32'h0;
        s = 32'h0;
        t = 32'h0;

        next_state = current_state;

        case (1'b1) // synopsys full_case parallel_case

            current_state[IDLE_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "IDLE";
                    // synopsys translate_on

                    if (msg_in_valid_pos_edge)
                        next_state = ROUND1[7:0];
                end // }

            //----------------------------------------------------------------
            //--------------------------- ROUND 1 ----------------------------
            //----------------------------------------------------------------
            current_state[ROUND1_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "ROUND1";
                    // synopsys translate_on

                    round = `ROUND1;

                    case (phase)
                        4'b0000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m0;
                                s=32'd07;
                                t= `T_1;
                                next_A=next_a;
                            end
                        4'b0001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m1;
                                s=32'd12;
                                t= `T_2;
                                next_D=next_a;
                            end
                        4'b0010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m2;
                                s=32'd17;
                                t= `T_3;
                                next_C=next_a;
                            end
                        4'b0011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m3;
                                s=32'd22;
                                t= `T_4;
                                next_B=next_a;
                            end
                        4'b0100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m4;
                                s=32'd07;
                                t= `T_5;
                                next_A=next_a;
                            end
                        4'b0101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m5;
                                s=32'd12;
                                t= `T_6;
                                next_D=next_a;
                            end
                        4'b0110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m6;
                                s=32'd17;
                                t= `T_7;
                                next_C=next_a;
                            end
                        4'b0111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m7;
                                s=32'd22;
                                t= `T_8;
                                next_B=next_a;
                            end
                        4'b1000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m8;
                                s=32'd07;
                                t= `T_9;
                                next_A=next_a;
                            end
                        4'b1001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m9;
                                s=32'd12;
                                t=`T_10;
                                next_D=next_a;
                            end
                        4'b1010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m10;
                                s=32'd17;
                                t=`T_11;
                                next_C=next_a;
                            end
                        4'b1011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m11;
                                s=32'd22;
                                t=`T_12;
                                next_B=next_a;
                            end
                        4'b1100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m12;
                                s=32'd7;
                                t=`T_13;
                                next_A=next_a;
                            end
                        4'b1101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m13;
                                s=32'd12;
                                t=`T_14;
                                next_D=next_a;
                            end
                        4'b1110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m14;
                                s=32'd17;
                                t=`T_15;
                                next_C=next_a;
                            end
                        4'b1111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m15;
                                s=32'd22;
                                t=`T_16;
                                next_B=next_a;
                            end
                    endcase

                    if (phase == 4'b1111)
                        next_state = ROUND2[7:0];
                end // }
            //----------------------------------------------------------------
            //--------------------------- ROUND 2 ----------------------------
            //----------------------------------------------------------------

            current_state[ROUND2_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "ROUND2";
                    // synopsys translate_on

                    round = `ROUND2;
                    case (phase)
                        4'b0000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m1;
                                s=32'd05;
                                t=`T_17;
                                next_A=next_a;
                            end
                        4'b0001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m6;
                                s=32'd09;
                                t=`T_18;
                                next_D=next_a;
                            end
                        4'b0010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m11;
                                s=32'd14;
                                t=`T_19;
                                next_C=next_a;
                            end
                        4'b0011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m0;
                                s=32'd20;
                                t=`T_20;
                                next_B=next_a;
                            end
                        4'b0100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m5;
                                s=32'd05;
                                t=`T_21;
                                next_A=next_a;
                            end
                        4'b0101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m10;
                                s=32'd09;
                                t=`T_22;
                                next_D=next_a;
                            end
                        4'b0110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m15;
                                s=32'd14;
                                t=`T_23;
                                next_C=next_a;
                            end
                        4'b0111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m4;
                                s=32'd20;
                                t=`T_24;
                                next_B=next_a;
                            end
                        4'b1000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m9;
                                s=32'd05;
                                t=`T_25;
                                next_A=next_a;
                            end
                        4'b1001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m14;
                                s=32'd9;
                                t=`T_26;
                                next_D=next_a;
                            end
                        4'b1010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m3;
                                s=32'd14;
                                t=`T_27;
                                next_C=next_a;
                            end
                        4'b1011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m8;
                                s=32'd20;
                                t=`T_28;
                                next_B=next_a;
                            end
                        4'b1100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m13;
                                s=32'd05;
                                t=`T_29;
                                next_A=next_a;
                            end
                        4'b1101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m2;
                                s=32'd09;
                                t=`T_30;
                                next_D=next_a;
                            end
                        4'b1110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m7;
                                s=32'd14;
                                t=`T_31;
                                next_C=next_a;
                            end
                        4'b1111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m12;
                                s=32'd20;
                                t=`T_32;
                                next_B=next_a;
                            end
                    endcase

                    if (phase == 4'b1111)
                        next_state = ROUND3[7:0];
                end // }
            //----------------------------------------------------------------
            //--------------------------- ROUND 3 ----------------------------
            //----------------------------------------------------------------

            current_state[ROUND3_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "ROUND3";
                    // synopsys translate_on

                    round = `ROUND3;
                    case (phase)
                        4'b0000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m5;
                                s=32'd04;
                                t=`T_33;
                                next_A=next_a;
                            end
                        4'b0001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m8;
                                s=32'd11;
                                t=`T_34;
                                next_D=next_a;
                            end
                        4'b0010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m11;
                                s=32'd16;
                                t=`T_35;
                                next_C=next_a;
                            end
                        4'b0011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m14;
                                s=32'd23;
                                t=`T_36;
                                next_B=next_a;
                            end
                        4'b0100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m1;
                                s=32'd04;
                                t=`T_37;
                                next_A=next_a;
                            end
                        4'b0101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m4;
                                s=32'd11;
                                t=`T_38;
                                next_D=next_a;
                            end
                        4'b0110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m7;
                                s=32'd16;
                                t=`T_39;
                                next_C=next_a;
                            end
                        4'b0111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m10;
                                s=32'd23;
                                t=`T_40;
                                next_B=next_a;
                            end
                        4'b1000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m13;
                                s=32'd04;
                                t=`T_41;
                                next_A=next_a;
                            end
                        4'b1001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m0;
                                s=32'd11;
                                t=`T_42;
                                next_D=next_a;
                            end
                        4'b1010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m3;
                                s=32'd16;
                                t=`T_43;
                                next_C=next_a;
                            end
                        4'b1011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m6;
                                s=32'd23;
                                t=`T_44;
                                next_B=next_a;
                            end
                        4'b1100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m9;
                                s=32'd04;
                                t=`T_45;
                                next_A=next_a;
                            end
                        4'b1101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m12;
                                s=32'd11;
                                t=`T_46;
                                next_D=next_a;
                            end
                        4'b1110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m15;
                                s=32'd16;
                                t=`T_47;
                                next_C=next_a;
                            end
                        4'b1111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m2;
                                s=32'd23;
                                t=`T_48;
                                next_B=next_a;
                            end
                    endcase

                    if (phase == 4'b1111)
                        next_state = ROUND4[7:0];
                end // }
            //----------------------------------------------------------------
            //--------------------------- ROUND 4 ----------------------------
            //----------------------------------------------------------------

            current_state[ROUND4_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "ROUND4";
                    // synopsys translate_on

                    round = `ROUND4;
                    case (phase)
                        4'b0000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m0;
                                s=32'd06;
                                t=`T_49;
                                next_A=next_a;
                            end
                        4'b0001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m7;
                                s=32'd10;
                                t=`T_50;
                                next_D=next_a;
                            end
                        4'b0010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m14;
                                s=32'd15;
                                t=`T_51;
                                next_C=next_a;
                            end
                        4'b0011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m5;
                                s=32'd21;
                                t=`T_52;
                                next_B=next_a;
                            end
                        4'b0100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m12;
                                s=32'd06;
                                t=`T_53;
                                next_A=next_a;
                            end
                        4'b0101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m3;
                                s=32'd10;
                                t=`T_54;
                                next_D=next_a;
                            end
                        4'b0110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m10;
                                s=32'd15;
                                t=`T_55;
                                next_C=next_a;
                            end
                        4'b0111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m1;
                                s=32'd21;
                                t=`T_56;
                                next_B=next_a;
                            end
                        4'b1000:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m8;
                                s=32'd06;
                                t=`T_57;
                                next_A=next_a;
                            end
                        4'b1001:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m15;
                                s=32'd10;
                                t=`T_58;
                                next_D=next_a;
                            end
                        4'b1010:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m6;
                                s=32'd15;
                                t=`T_59;
                                next_C=next_a;
                            end
                        4'b1011:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m13;
                                s=32'd21;
                                t=`T_60;
                                next_B=next_a;
                            end
                        4'b1100:
                            begin
                                a=A;
                                b=B;
                                c=C;
                                d=D;
                                m=m4;
                                s=32'd06;
                                t=`T_61;
                                next_A=next_a;
                            end
                        4'b1101:
                            begin
                                a=D;
                                b=A;
                                c=B;
                                d=C;
                                m=m11;
                                s=32'd10;
                                t=`T_62;
                                next_D=next_a;
                            end
                        4'b1110:
                            begin
                                a=C;
                                b=D;
                                c=A;
                                d=B;
                                m=m2;
                                s=32'd15;
                                t=`T_63;
                                next_C=next_a;
                            end
                        4'b1111:
                            begin
                                a=B;
                                b=C;
                                c=D;
                                d=A;
                                m=m9;
                                s=32'd21;
                                t=`T_64;
                                next_B=next_a;
                            end
                    endcase

                    if (phase == 4'b1111)
                        next_state = FINISH_OFF[7:0];
                end // }
            //----------------------------------------------------------------

            current_state[FINISH_OFF_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "FINISH_OFF";
                    // synopsys translate_on

                    next_A = AA + A;
                    next_B = BB + B;
                    next_C = CC + C;
                    next_D = DD + D;

                    next_state = TURN_ARND[7:0];
                end // }
            //----------------------------------------------------------------

            // One cycle for making the system to come to reset state
            current_state[TURN_ARND_BIT]:
                begin // {
                    // synopsys translate_off
                    ascii_state = "TURN_ARND";
                    // synopsys translate_on

                    next_state = IDLE[7:0];
                end // }

        endcase
    end // }

//--------------------------------
//
// Flops and other combinatorial
// logic definition
//
//--------------------------------

// Outputs
assign msg_output = {{A[7:0], A[15:8], A[23:16], A[31:24]}
                     ,{B[7:0], B[15:8], B[23:16], B[31:24]}
                     ,{C[7:0], C[15:8], C[23:16], C[31:24]}
                     ,{D[7:0], D[15:8], D[23:16], D[31:24]}};
always @(posedge clk)
    msg_out_valid <= current_state[FINISH_OFF_BIT];

assign ready = current_state[IDLE_BIT];

// Internal scratch pads
always @(posedge clk)
    if (next_state[ROUND1_BIT] && current_state[IDLE_BIT])
        begin // {
            AA <= A;
            BB <= B;
            CC <= C;
            DD <= D;
        end // }

// Initialize A, B, C and D and then compute them
always @(posedge clk)
    if (rst )
        begin // {
            A <= SALT_A;
            B <= SALT_B;
            C <= SALT_C;
            D <= SALT_D;
        end // }
    else if (init) begin // added 05/12/20
            A <= SALT_A;
            B <= SALT_B;
            C <= SALT_C;
            D <= SALT_D;       
    end
    else
        begin // {
            A <= next_A;
            B <= next_B;
            C <= next_C;
            D <= next_D;
        end // }

// Determine one of the 16 phases within each round
always @(posedge clk)
    if (rst)
        phase <= 4'b0;
    else if (init) // added 05/12/20
        phase <= 4'b0;      
    else if (next_state[ROUND1_BIT] && current_state[IDLE_BIT])
        phase <= 4'b0;
    else
        phase <= phase + 4'b1;

// Assign current_state based on rst and the next_state
always @(posedge clk)
    if (rst)
        current_state <= IDLE[7:0];
    else if (init)
        current_state <= IDLE[7:0];      
    else
        current_state <= next_state;
endmodule
