//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
module pcode(
           clk, rst, en, sat,
           preg
       );
parameter SAT_WIDTH = 6;
parameter SREG_WIDTH = 37;
parameter XREG_WIDTH = 12;
//parameter PREG_WIDTH = 32;
parameter ini_x1a=12'b001001001000;
parameter ini_x1b=12'b010101010100;
parameter ini_x2a=12'b100100100101;
parameter ini_x2b=12'b010101010100;

input clk;
input rst;
input en;
input [ SAT_WIDTH-1:0] sat;
`ifdef PREG_WIDTH
output reg [PREG_WIDTH-1:0] preg;
`else
output reg preg;
`endif

reg[XREG_WIDTH-1:0] x1a;
reg[XREG_WIDTH-1:0] x1b;
reg[XREG_WIDTH-1:0] x2a;
reg[XREG_WIDTH-1:0] x2b;
reg[SREG_WIDTH-1:0] sreg;

wire x1a_rst, x1b_rst, x2a_rst, x2b_rst;
wire x1a_cnt_d, x1b_cnt_d, x2a_cnt_d, x2b_cnt_d, x_cnt_d, z_cnt_eow, z_cnt_sow;
reg[XREG_WIDTH-1:0] x1a_cnt, x1b_cnt, x2a_cnt, x2b_cnt;
reg[SAT_WIDTH-1:0] x_cnt;
reg[18:0] z_cnt;

reg x1b_en, x2a_en, x2b_en;
wire x1b_res, x2a_res, x2b_res;
wire x1b_halt, x2a_halt, x2b_halt;

//////////////////////////////////////////
//Control Signals
//////////////////////////////////////////
assign x1b_res = x1a_cnt_d & x1a_rst;
assign x1b_halt= x1b_cnt_d & x1b_rst;

assign x2a_res = z_cnt_sow|x_cnt_d;
assign x2a_halt= (z_cnt_eow|x2a_cnt_d) & x2a_rst;

assign x2b_res = x2a_res;
assign x2b_halt= (z_cnt_eow|x2b_cnt_d) & x2b_rst;

//////////////////////////////////////////
//Clock Control Signals
//////////////////////////////////////////
always @(posedge clk)
    begin
        if(rst)
            x1b_en<=1;
        else if(en)
            begin
                if(x1b_halt)
                    x1b_en<=0;
                else if(x1b_res)
                    x1b_en<=1;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x2a_en<=1;
        else if(en)
            begin
                if(x2a_halt)
                    x2a_en<=0;
                else if(x2a_res)
                    x2a_en<=1;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x2b_en<=1;
        else if(en)
            begin
                if(x2b_halt)
                    x2b_en<=0;
                else if(x2b_res)
                    x2b_en<=1;
            end
    end

//////////////////////////////////////////
//Decoders
//////////////////////////////////////////
assign x1a_rst = (x1a==12'd4092) ? 1:0;
assign x1b_rst = (x1b==12'd4093) ? 1:0;
assign x2a_rst = (x2a==12'd4092) ? 1:0;
assign x2b_rst = (x2b==12'd4093) ? 1:0;

//////////////////////////////////////////
//Counters
//////////////////////////////////////////
assign x1a_cnt_d = (x1a_cnt==12'd3750)   ? 1:0;
assign x1b_cnt_d = (x1b_cnt==12'd3749)   ? 1:0;
assign x2a_cnt_d = (x2a_cnt==12'd3750)   ? 1:0;
assign x2b_cnt_d = (x2b_cnt==12'd3749)   ? 1:0;
assign x_cnt_d   = (x_cnt  ==6'd37)      ? 1:0;
assign z_cnt_sow = ((z_cnt  ==19'd000000)& x1b_res==1) ? 1:0;
assign z_cnt_eow = ((z_cnt  ==19'd403200)& x1b_res==1) ? 1:0;

always @(posedge clk)
    begin
        if(rst)
            x1a_cnt  <=0;
        else if(en & x1a_rst)
            begin
                if(x1a_cnt < 12'd3750)
                    x1a_cnt <= x1a_cnt+1;
                else
                    x1a_cnt <= 12'd0;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x1b_cnt <=0;
        else if(en &x1b_rst)
            begin
                if(x1b_cnt < 12'd3749)
                    x1b_cnt <= x1b_cnt+1;
                else
                    x1b_cnt <= 12'd0;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x2a_cnt<=0;
        else if(en &x2a_rst)
            begin
                if(x2a_cnt < 12'd3750)
                    x2a_cnt <= x2a_cnt+1;
                else
                    x2a_cnt <=12'd0;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x2b_cnt  <=0;
        else if(en &x2b_rst)
            begin
                if(x2b_cnt < 12'd3749)
                    x2b_cnt <= x2b_cnt+1;
                else
                    x2b_cnt <=12'd0;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            x_cnt<=0;
        else if(en & x2a_res & x2a_cnt_d)
            begin
                if(x_cnt < 6'd37)
                    x_cnt <= x_cnt+1;
                else
                    x_cnt <=19'd0;
            end
    end

always @(posedge clk)
    begin
        if(rst)
            z_cnt<=0;
        else if(en & x1b_res)
            begin
                if(z_cnt < 19'd403200)
                    z_cnt <= z_cnt+1;
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
        if(rst|x1b_rst)
            x1b<=ini_x1b;
        else if (en & x1b_en)
            x1b<={x1b[XREG_WIDTH-2:0],x1b[0]^x1b[1]^x1b[4]^x1b[7]^x1b[8]^x1b[9]^x1b[10]^x1b[11]};
    end

always @(posedge clk)
    begin
        if(rst|x2a_rst)
            x2a<=ini_x2a;
        else if (en & x2a_en)
            x2a<={x2a[XREG_WIDTH-2:0],x2a[0]^x2a[2]^x2a[3]^x2a[4]^x2a[6]^x2a[7]^x2a[8]^x2a[9]^x2a[10]^x2a[11]};
    end

always @(posedge clk)
    begin
        if(rst|x2b_rst)
            x2b<=ini_x2b;
        else if (en & x2b_en)
            x2b<={x2b[XREG_WIDTH-2:0],x2b[1]^x2b[2]^x2b[3]^x2b[7]^x2b[8]^x2b[11]};
    end

//Output
always @(posedge clk)
    begin
        if(rst)
            sreg<=32'b0;
        else if(en)
            sreg<={sreg[SREG_WIDTH-2:0],(x2a[XREG_WIDTH-1]^x2b[XREG_WIDTH-1])};
    end

always @(posedge clk)
    begin
        if(rst)
`ifdef PREG_WIDTH

            preg<=32'b0;
`else
            preg<=1'b0;
`endif

        else if(en)
`ifdef PREG_WIDTH

            preg<={preg[PREG_WIDTH-2:0],(x1a[XREG_WIDTH-1]^x1b[XREG_WIDTH-1])^sreg[sat]};
`else
            preg<={(x1a[XREG_WIDTH-1]^x1b[XREG_WIDTH-1])^sreg[sat]};
`endif

    end

endmodule
