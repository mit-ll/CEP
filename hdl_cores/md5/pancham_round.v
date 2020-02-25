/*****************************************************************
 Pancham is an MD5 compliant IP core for cryptographic applicati
 -ons. 
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
 * pancham_round performs the rounds 1-4 of the MD5 algorithm 
 * described in RFC1321 for a 128-bit long input message. 
 * 
 * Inputs: [abcd m[k] s abs(sin(2*pi*t/64))] as described 
 *         in RFC1321.Also the round number (1-4).
 * 
 * Outputs: the modified 'a' value as describes in RFC1321
 *         on the left hand side of the round #n equation.
 * 
 */

`define ROUND1 2'b00
`define ROUND2 2'b01
`define ROUND3 2'b10
`define ROUND4 2'b11

module pancham_round (
           a
           , b
           , c
           , d
           , m
           , s
           , t
           , round

           , next_a
       );

input  [31:0] a;
input  [31:0] b;
input  [31:0] c;
input  [31:0] d;
input  [31:0] m;          // Note that for a 128-bit long input message, X[k] = M[k] = m
input  [31:0] s;
input  [31:0] t;          // t-th sample of abs(sin(i)), i = 1, 2, ..., 64
input   [1:0] round;      // round number (1-4).

output [31:0] next_a;

wire  [31:0] a;         //
wire  [31:0] b;
wire  [31:0] c;
wire  [31:0] d;
wire  [31:0] m;
wire  [31:0] s;
wire  [31:0] t;
wire   [1:0] round;

reg   [31:0] next_a;

reg   [31:0] add_result;
reg   [31:0] rotate_result1;
reg   [31:0] rotate_result2;

always @(a
             or    b
             or    c
             or    d
             or    m
             or    s
             or    t
             or    round)
    begin // {
        case (round)
            `ROUND1:
                begin // {
                    add_result = (a + F(b,c,d) + m + t);
                    rotate_result1 = add_result << s;
                    rotate_result2 = add_result >> (32-s);
                    next_a = b + (rotate_result1 | rotate_result2);
                end // }
            `ROUND2:
                begin // {
                    add_result = (a + G(b,c,d) + m + t);
                    rotate_result1 = add_result << s;
                    rotate_result2 = add_result >> (32-s);
                    next_a = b + (rotate_result1 | rotate_result2);
                end // }
            `ROUND3:
                begin // {
                    add_result = (a + H(b,c,d) + m + t);
                    rotate_result1 = add_result << s;
                    rotate_result2 = add_result >> (32-s);
                    next_a = b + (rotate_result1 | rotate_result2);
                end // }
            `ROUND4:
                begin // {
                    add_result = (a + I(b,c,d) + m + t);
                    rotate_result1 = add_result << s;
                    rotate_result2 = add_result >> (32-s);
                    next_a = b + (rotate_result1 | rotate_result2);
                end // }
        endcase
    end // }

//--------------------------------
//
// Function declarations
//
//--------------------------------
// Step 4 functions F, G, H and I
function [31:0] F;
    input [31:0] x, y, z;
    begin // {
        F = (x&y)|((~x)&z);
    end // }
endfunction // }

function [31:0] G;
    input [31:0] x, y, z;
    begin // {
        G = (x&z)|(y&(~z));
    end // }
endfunction

function [31:0] H;
    input [31:0] x, y, z;
    begin // {
        H = (x^y^z);
    end // }
endfunction

function [31:0] I;
    input [31:0] x, y, z;
    begin // {
        I = (y^(x|(~z)));
    end // }
endfunction

endmodule
