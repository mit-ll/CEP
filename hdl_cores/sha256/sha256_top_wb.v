//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : sha256_top_wb.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides a wishbone based-SHA256 core
//

module sha256_top_wb(
           wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
           wb_stb_i, wb_we_i,
           wb_ack_o, wb_err_o, wb_dat_o,
           wb_clk_i, wb_rst_i, int_o
       );

parameter DW = 32;
parameter AW = 32;

input [AW-1:0] wb_adr_i;
input   wb_cyc_i;
input [DW-1:0]  wb_dat_i;
input [3:0]   wb_sel_i;
input   wb_stb_i;
input   wb_we_i;

output   wb_ack_o;
output   wb_err_o;
output reg [DW-1:0]  wb_dat_o;
output  int_o;

input   wb_clk_i;
input   wb_rst_i;

assign wb_ack_o = 1'b1;
assign wb_err_o = 1'b0;
assign int_o = 1'b0;

// Internal registers
reg startHash, startHash_r;
reg newMessage, newMessage_r;
reg [31:0] data [0:15];

wire [511:0] bigData = {data[15], data[14], data[13], data[12], data[11], data[10], data[9], data[8], data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]};
wire [255:0] hash;
wire ready;
wire hashValid;

// Implement SHA256 I/O memory map interface
// Write side
always @(posedge wb_clk_i)
    begin
        if(wb_rst_i)
            begin
                startHash       <= 0;
                startHash_r     <= 0;
                newMessage      <= 0;
                newMessage_r    <= 0;
                data[0] <= 0;
                data[1] <= 0;
                data[2] <= 0;
                data[3] <= 0;
                data[4] <= 0;
                data[5] <= 0;
                data[6] <= 0;
                data[7] <= 0;
                data[8] <= 0;
                data[9] <= 0;
                data[10] <= 0;
                data[11] <= 0;
                data[12] <= 0;
                data[13] <= 0;
                data[14] <= 0;
                data[15] <= 0;
            end
        else begin

            // Generate a registered versions of startHash and newMessage
            startHash_r         <= startHash;
            newMessage_r        <= newMessage;

            // Perform a write
            if(wb_stb_i & wb_we_i) begin
                case(wb_adr_i[4:0])
                    0:  begin
                            startHash <= wb_dat_i[0];
                            newMessage <= wb_dat_i[1];
                        end
                    1:  data[0] <= wb_dat_i;
                    2:  data[1] <= wb_dat_i;
                    3:  data[2] <= wb_dat_i;
                    4:  data[3] <= wb_dat_i;
                    5:  data[4] <= wb_dat_i;
                    6:  data[5] <= wb_dat_i;
                    7:  data[6] <= wb_dat_i;
                    8:  data[7] <= wb_dat_i;
                    9:  data[8] <= wb_dat_i;
                    10: data[9] <= wb_dat_i;
                    11: data[10] <= wb_dat_i;
                    12: data[11] <= wb_dat_i;
                    13: data[12] <= wb_dat_i;
                    14: data[13] <= wb_dat_i;
                    15: data[14] <= wb_dat_i;
                    16: data[15] <= wb_dat_i;
                    default:
                        ;
                endcase
            end else begin
                startHash           <= 1'b0;
                newMessage          <= 1'b0;
            end // end else
        end
    end // end always

// Implement SHA256 I/O memory map interface
// Read side
always @(*)
    begin
        case(wb_adr_i[4:0])
            0:  wb_dat_o = {31'b0, ready};
            1:  wb_dat_o = data[0];
            2:  wb_dat_o = data[1];
            3:  wb_dat_o = data[2];
            4:  wb_dat_o = data[3];
            5:  wb_dat_o = data[4];
            6:  wb_dat_o = data[5];
            7:  wb_dat_o = data[6];
            8:  wb_dat_o = data[7];
            9:  wb_dat_o = data[8];
            10: wb_dat_o = data[9];
            11: wb_dat_o = data[10];
            12: wb_dat_o = data[11];
            13: wb_dat_o = data[12];
            14: wb_dat_o = data[13];
            15: wb_dat_o = data[14];
            16: wb_dat_o = data[15];
            17: wb_dat_o = {31'b0, hashValid};
            18: wb_dat_o = hash[31:0];
            19: wb_dat_o = hash[63:32];
            20: wb_dat_o = hash[95:64];
            21: wb_dat_o = hash[127:96];
            22: wb_dat_o = hash[159:128];
            23: wb_dat_o = hash[191:160];
            24: wb_dat_o = hash[223:192];
            25: wb_dat_o = hash[255:224];
            default:
                wb_dat_o = 32'b0;
        endcase
    end

sha256 sha256(
           .clk(wb_clk_i),
           .rst(wb_rst_i),
           .init(startHash && ~startHash_r),
           .next(newMessage && ~newMessage_r),
           .block(bigData),
           .digest(hash),
           .digest_valid(hashValid),
           .ready(ready)
       );

endmodule
