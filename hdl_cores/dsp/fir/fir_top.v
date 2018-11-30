//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : fir_top.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides a wishbone based-FIR core
//

module fir_top(
           wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
           wb_stb_i, wb_we_i,
           wb_ack_o, wb_err_o, wb_dat_o,
           wb_clk_i, wb_rst_i, int_o
       );

parameter dw = 32;
parameter aw = 32;

input [aw-1:0]      wb_adr_i;
input               wb_cyc_i;
input [dw-1:0]      wb_dat_i;
input [3:0]         wb_sel_i;
input               wb_stb_i;
input               wb_we_i;

output              wb_ack_o;
output              wb_err_o;
output reg [dw-1:0] wb_dat_o;
output              int_o;

input               wb_clk_i;
input               wb_rst_i;

assign wb_ack_o = 1'b1;
assign wb_err_o = 1'b0;
assign int_o = 1'b0;

// Internal registers
reg next;
reg [31:0] dataX [0:31];
reg [31:0] dataY [0:31];
reg [5:0] xSel=6'b0;
reg [5:0] ySel=6'b0;
reg [5:0] count;
wire [31:0] dataIn, dataOut, data_Out;
reg [31:0] data_In_data, data_In_addr, data_Out_addr;

reg data_valid, data_In_write;
wire next_out;

// Implement MD5 I/O memory map interface
// Write side
always @(posedge wb_clk_i)
    begin
        if(wb_rst_i)
            begin
                next          <= 0;
                data_In_write <= 0;
                data_In_addr  <= 0;
                data_In_data  <= 0;
            end
        else if(wb_stb_i & wb_we_i)
            case(wb_adr_i[5:2])
                0:
                    next          <= wb_dat_i[0];
                1:
                    data_In_write <= wb_dat_i[0];
                2:
                    data_In_addr  <= wb_dat_i;
                3:
                    data_In_data  <= wb_dat_i;
                4:
                    data_Out_addr <= wb_dat_i;
                default:
                    ;
            endcase
    end // always @ (posedge wb_clk_i)

// Implement MD5 I/O memory map interface
// Read side
always @(*)
    begin
        case(wb_adr_i[5:2])
            0:
                wb_dat_o = {31'b0, next};
            1:
                wb_dat_o = {31'b0, data_In_write};
            2:
                wb_dat_o = data_In_addr;
            3:
                wb_dat_o = data_In_data;
            4:
                wb_dat_o = data_Out_addr;
            5:
                wb_dat_o = data_Out;
            6:
                wb_dat_o = {31'b0, data_valid};
            default:
                wb_dat_o = 32'b0;
        endcase
    end // always @ (*)

FIR_filter FIR_filter(
               .clk(wb_clk_i),
               .reset(~wb_rst_i),
               .inData(dataIn),
               .outData(dataOut));

reg data_In_write_r;
always @(posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            data_In_write_r <= 0;    
        else
            data_In_write_r <= data_In_write;
    end

wire data_In_write_posedge = data_In_write & ~data_In_write_r;

reg [15:0] i;

always @ (posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            for (i = 0; i < 32; i = i + 1)
                dataX[i]        <= 0;
        else
            dataX[data_In_addr] <= data_In_data;
    end
assign data_Out = dataY[data_Out_addr];

reg next_r;
always @(posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            next_r      <= 0;
        else
            next_r      <= next;
    end

wire next_posedge = next & ~next_r;

always @ (posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            xSel    <= 0;
        else if(next_posedge)
            xSel    <= 6'h00;
        else if(xSel<6'b100000)
            xSel    <= xSel + 1;
    end
assign dataIn = (xSel<6'b100000) ? dataX[xSel] : 32'b0;

always @ (posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            count       <= 0;
        else if(next_posedge)
            begin
                count <= 6'h00;
            end
        else if(xSel<4'b1010)
            begin
                count <= count +1;
            end
    end

assign next_out = (count == 4'b1000);

reg next_out_r;
always @(posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            next_out_r  <= 0;
        else
            next_out_r <= next_out;
    end

wire next_out_posedge = next_out & ~next_out_r;

always @ (posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i) begin
            ySel    <= 0;
            for (i = 0; i < 32; i = i + 1)
                dataY[i]   <= 0;
        end else if(next_out_posedge)
            begin
                ySel <= 6'h00;
            end
        else if(ySel<6'b100000)
            begin
                ySel <= ySel +1;
                dataY[ySel] = dataOut;
            end
    end

always @ (posedge wb_clk_i or posedge wb_rst_i)
    begin
        if (wb_rst_i)
            data_valid     <= 0;
        else if(next_posedge)
            begin
                data_valid <= 0;
            end
        else if(next_out_posedge)
            begin
                data_valid <= 1;
            end
    end
endmodule
