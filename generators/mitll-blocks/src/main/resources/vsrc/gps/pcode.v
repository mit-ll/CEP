//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      pcode.v
// Program:        Common Evaluation Platform (CEP)
// Description:    P-code generator
//
//************************************************************************
module pcode(
           clk, reset, prn_changed, en, sat,
           preg, xn_cnt_speed, z_cnt_speed,
           ini_x1a, ini_x1b, ini_x2a, ini_x2b
       );
parameter SAT_WIDTH = 6;
parameter SREG_WIDTH = 37;
parameter XREG_WIDTH = 12;

input clk;
input reset;
input prn_changed;
input en;
input [ SAT_WIDTH-1:0] sat;
output wire preg; //Combinational output

//Verification-focused inputs
input [11:0] xn_cnt_speed; //default must be 1.
input [18:0] z_cnt_speed; //default must be 1.
input [11:0] ini_x1a;     //default must be 12'b001001001000;
input [11:0] ini_x1b;     //default must be 12'b010101010100;
input [11:0] ini_x2a;     //default must be 12'b100100100101;
input [11:0] ini_x2b;     //default must be 12'b010101010100;


wire rst = reset | prn_changed;
   
reg[XREG_WIDTH-1:0] x1a;
reg[XREG_WIDTH-1:0] x1b;
reg[XREG_WIDTH-1:0] x2a;
reg[XREG_WIDTH-1:0] x2b;
reg [SREG_WIDTH:0] sreg; //Note that sat is 1-based!

wire x1a_rst, x1b_rst, x2a_rst, x2b_rst;
wire x1a_cnt_d, x1b_cnt_d, x2a_cnt_d, x2b_cnt_d, x_cnt_d, z_cnt_last, z_cnt_eow, x1a_cnt_last;
reg[XREG_WIDTH-1:0] x1a_cnt, x1b_cnt, x2a_cnt, x2b_cnt;
reg[SAT_WIDTH-1:0] x_cnt;
reg[18:0] z_cnt;

wire x1b_en, x2a_en, x2b_en;
reg x1b_en_r, x2a_en_r, x2b_en_r;
wire x1b_res, x2a_res, x2b_res;
wire x1b_halt, x2a_halt, x2b_halt;

//////////////////////////////////////////
//Control Signals
//////////////////////////////////////////
assign x1b_res = x1a_cnt_d & x1a_rst;
assign x1b_halt= (x1b_cnt_d|x1a_cnt_last) & x1b_rst;

assign x2a_res = /*!x1a_cnt_d  &*/ (z_cnt_eow|x_cnt_d); //If on last cycle of X1A, do not continue
assign x2a_halt= (z_cnt_eow|x2a_cnt_d|x1a_cnt_last) & x2a_rst;

assign x2b_res = x2a_res;
assign x2b_halt= (z_cnt_eow|x2b_cnt_d|x1a_cnt_last) & x2b_rst;

assign z_cnt_last = (z_cnt >= 19'd403200-z_cnt_speed);
assign z_cnt_eow = (z_cnt_last & x1b_res);

assign x1a_cnt_last = x1a_cnt_d & z_cnt_last;

//////////////////////////////////////////
//Clock Control Signals
//////////////////////////////////////////
assign x1b_en = x1b_en_r & !x1b_halt;
assign x2a_en = x2a_en_r & !x2a_halt;
assign x2b_en = x2b_en_r & !x2b_halt;

always @(posedge clk)
    begin
        if(rst)
            x1b_en_r<=1;
        else if(en) begin
            if(x1b_res)
                x1b_en_r<=1;
            else if(x1b_halt)
                x1b_en_r<=0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x2a_en_r<=1;
        else if(en) begin
            if(x2a_res)
                x2a_en_r<=1;
            else if(x2a_halt)
                x2a_en_r<=0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x2b_en_r<=1;
        else if(en) begin
            if(x2b_res)
                x2b_en_r<=1;
            else if(x2b_halt)
                x2b_en_r<=0;
        end
    end

//////////////////////////////////////////
//Decoders
//////////////////////////////////////////
assign x1a_rst = (x1a==12'b000100100100); //4092nd state
assign x1b_rst = (x1b==12'b001010101010); //4093rd state
assign x2a_rst = (x2a==12'b110010010010); //4092nd state
assign x2b_rst = (x2b==12'b001010101010); //4093rd state

//////////////////////////////////////////
//Counters
//////////////////////////////////////////
assign x1a_cnt_d = (x1a_cnt>=12'd3750-xn_cnt_speed); //Comparison instead of equality due to configurable counter increment
assign x1b_cnt_d = (x1b_cnt>=12'd3749-xn_cnt_speed);
assign x2a_cnt_d = (x2a_cnt>=12'd3750-xn_cnt_speed);
assign x2b_cnt_d = (x2b_cnt>=12'd3749-xn_cnt_speed);
assign x_cnt_d   = (x_cnt  ==6'd37);

always @(posedge clk)
    begin
        if(rst)
            x1a_cnt  <=0;
        else if(en & x1a_rst) begin
            if (!x1a_cnt_d)
                x1a_cnt <= x1a_cnt+xn_cnt_speed;
            else
                x1a_cnt <= 12'd0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x1b_cnt <=0;
        else if(en & x1b_rst & x1b_en_r) begin //TODO: x1b_en_r?
            if (!x1b_cnt_d & !x1a_cnt_last)
                x1b_cnt <= x1b_cnt+xn_cnt_speed;
            else
                x1b_cnt <= 12'd0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x2a_cnt<=0;
        else if(en & x2a_rst & x2a_en_r) begin
            if (!x2a_cnt_d & !x1a_cnt_last)
                x2a_cnt <= x2a_cnt+xn_cnt_speed;
            else
                x2a_cnt <=12'd0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x2b_cnt  <=0;
        else if(en & x2b_rst & x2b_en_r) begin
            if (!x2b_cnt_d & !x1a_cnt_last)
                x2b_cnt <= x2b_cnt+xn_cnt_speed;
            else
                x2b_cnt <=12'd0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            x_cnt<=0;
        else if(en & ((x2a_rst & x2a_cnt_d) | (x_cnt!=0))) begin
            if(x_cnt < 6'd37)
                x_cnt <= x_cnt+1;
            else
                x_cnt <= 6'd0;
        end
    end

always @(posedge clk)
    begin
        if(rst)
            z_cnt<=0;
        else if(en & x1b_res) begin
            if (!z_cnt_last)
                z_cnt <= z_cnt+z_cnt_speed;
            else
                z_cnt <=19'd0;
        end
    end

//////////////////////////////////////////
//4 shift registers
//////////////////////////////////////////
always @(posedge clk)
    begin
        if(rst|x1a_rst)
            x1a<=ini_x1a;
        else if(en)
            x1a<={x1a[XREG_WIDTH-2:0],x1a[5]^x1a[7]^x1a[10]^x1a[11]};
    end

always @(posedge clk)
    begin
        //If enabled, handle resume/reset immediately.
        //Keep in mind that *_en update is delayed 1 cycle from *_halt, but need to halt immediately.
        if(rst | (en & (x1b_res | (x1b_rst & x1b_en))))
            x1b<=ini_x1b;
        else if (en & x1b_en)
            x1b<={x1b[XREG_WIDTH-2:0],x1b[0]^x1b[1]^x1b[4]^x1b[7]^x1b[8]^x1b[9]^x1b[10]^x1b[11]};
    end

always @(posedge clk)
    begin
        if(rst | (en & (x2a_res | (x2a_rst & x2a_en))))
            x2a<=ini_x2a;
        else if (en & x2a_en)
            x2a<={x2a[XREG_WIDTH-2:0],x2a[0]^x2a[2]^x2a[3]^x2a[4]^x2a[6]^x2a[7]^x2a[8]^x2a[9]^x2a[10]^x2a[11]};
    end

always @(posedge clk)
    begin
        if (rst | (en & (x2b_res | (x2b_rst & x2b_en))))
            x2b<=ini_x2b;
        else if (en & x2b_en)
            x2b<={x2b[XREG_WIDTH-2:0],x2b[1]^x2b[2]^x2b[3]^x2b[7]^x2b[8]^x2b[11]};
    end


//x2 shift register
always @(posedge clk)
    begin
        if(rst)
            sreg<={SREG_WIDTH{1'b1}};
        else if(en)
            sreg<={sreg[SREG_WIDTH-1:0], (x2a[XREG_WIDTH-1]^x2b[XREG_WIDTH-1])};
    end


//Output
assign preg = (rst | (sat==0)) ? 1'b0 : (x1a[XREG_WIDTH-1]^x1b[XREG_WIDTH-1]) ^ sreg[sat-1]; 


endmodule
