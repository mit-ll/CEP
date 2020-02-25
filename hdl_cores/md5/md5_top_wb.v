//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : md5_top.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides a wishbone based-MD5 core
//

module md5_top_wb(
           wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
           wb_stb_i, wb_we_i,
           wb_ack_o, wb_err_o, wb_dat_o,
           wb_clk_i, wb_rst_i, int_o
       );

parameter DW = 32;
parameter AW = 32;

input [AW-1:0] wb_adr_i;
input wb_cyc_i;
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

assign wb_ack_o = wb_stb_i;
assign wb_err_o = 1'b0;
assign int_o = 1'b0;

// Internal registers
reg startHash, startHash_r;
reg [31:0] data [15:0];
reg message_reset, message_reset_r;

wire [511:0] bigData = {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]};
wire [127:0] hash;
wire ready;
wire hashValid;

// Implement MD5 I/O memory map interface
// Write side
always @(posedge wb_clk_i)
    begin
        if(wb_rst_i)
            begin
                startHash       <= 0;
                startHash_r     <= 0;
                message_reset   <= 0;
                message_reset_r <= 0;
                data[0]         <= 0;
                data[1]         <= 0;
                data[2]         <= 0;
                data[3]         <= 0;
                data[4]         <= 0;
                data[5]         <= 0;
                data[6]         <= 0;
                data[7]         <= 0;
                data[8]         <= 0;
                data[9]         <= 0;
                data[10]        <= 0;
                data[11]        <= 0;
                data[12]        <= 0;
                data[13]        <= 0;
                data[14]        <= 0;
                data[15]        <= 0;
            end
        else begin 

            // Generate a registered versions of startHash and message_reset
            startHash_r         <= startHash;
            message_reset_r     <= message_reset;

            // Perform a write
            if(wb_stb_i & wb_we_i) begin
                case(wb_adr_i[7:0])
                    0: startHash        <= wb_dat_i[0];
                    1: data[15]         <= wb_dat_i;
                    2: data[14]         <= wb_dat_i;
                    3: data[13]         <= wb_dat_i;
                    4: data[12]         <= wb_dat_i;
                    5: data[11]         <= wb_dat_i;
                    6: data[10]         <= wb_dat_i;
                    7: data[9]          <= wb_dat_i;
                    8: data[8]          <= wb_dat_i;
                    9: data[7]          <= wb_dat_i;
                    10: data[6]         <= wb_dat_i;
                    11: data[5]         <= wb_dat_i;
                    12: data[4]         <= wb_dat_i;
                    13: data[3]         <= wb_dat_i;
                    14: data[2]         <= wb_dat_i;
                    15: data[1]         <= wb_dat_i;
                    16: data[0]         <= wb_dat_i;
                    22: message_reset   <= wb_dat_i[0];
                default:
                    ;
                endcase
            end else begin // if(wb_stb_i & wb_we_i)
                startHash           <= 1'b0;
                message_reset       <= 1'b0;

            end // end else
        end // else
    end  // end always

// Implement MD5 I/O memory map interface
// Read side
always @(*)
    begin
        case(wb_adr_i[7:0])
            0: wb_dat_o = {31'b0, ready};
            1: wb_dat_o = data[15];
            2: wb_dat_o = data[14];
            3: wb_dat_o = data[13];
            4: wb_dat_o = data[12];
            5: wb_dat_o = data[11];
            6: wb_dat_o = data[10];
            7: wb_dat_o = data[9];
            8: wb_dat_o = data[8];
            9: wb_dat_o = data[7];
            10: wb_dat_o = data[6];
            11: wb_dat_o = data[5];
            12: wb_dat_o = data[4];
            13: wb_dat_o = data[3];
            14: wb_dat_o = data[2];
            15: wb_dat_o = data[1];
            16: wb_dat_o = data[0];
            17: wb_dat_o = {31'b0, hashValid};
            21: wb_dat_o = hash[127:96];
            20: wb_dat_o = hash[95:64];
            19: wb_dat_o = hash[63:32];
            18: wb_dat_o = hash[31:0];
            default:
                wb_dat_o = 32'b0;
        endcase
    end

pancham pancham(
            .clk(wb_clk_i),
            .rst(wb_rst_i | (message_reset & ~message_reset_r)),
            .msg_padded(bigData),
            .msg_in_valid(startHash && ~startHash_r),
            .msg_output(hash),
            .msg_out_valid(hashValid),
            .ready(ready)
        );

endmodule
