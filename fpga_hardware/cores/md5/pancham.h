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
