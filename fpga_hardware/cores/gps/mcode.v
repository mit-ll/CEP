module mcode(
    inital, clk, rst
    ve, e, p, l, vl
);

   parameter width = 32;
   parameter loc_ve = 4;
   parameter loc_e = 8;
   parameter loc_p = 12;
   parameter loc_l = 16;
   parameter loc_vl = 20;
   
   input [width-1:0] inital;
   input clk;
   input rst;
   output ve;
   output e;
   output p;
   output l;
   output vl;
   
   reg[width-1:0] register;
   
   always @
   
   
