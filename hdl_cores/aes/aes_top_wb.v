//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : aes_top_wb.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides a wishbone based-AES core
//

module aes_top_wb(
           wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
           wb_stb_i, wb_we_i,
           wb_ack_o, wb_err_o, wb_dat_o,
           wb_clk_i, wb_rst_i, int_o
       );

parameter           DW = 32;
parameter           AW = 32;

input wire [AW-1:0] wb_adr_i;
input wire          wb_cyc_i;
input wire [DW-1:0] wb_dat_i;
input wire [3:0]    wb_sel_i;
input wire          wb_stb_i;
input wire          wb_we_i;

output wire         wb_ack_o;
output wire         wb_err_o;
output reg [DW-1:0] wb_dat_o;
output wire         int_o;

input wire          wb_clk_i;
input wire          wb_rst_i;

// As of now, no errors are generated
assign wb_err_o     = 1'b0;

// Interrupt is unused
assign int_o        = 1'b0;

// Internal registers
reg                 start, start_r;
reg [31:0]          pt [0:3];
reg [31:0]          key [0:5];

wire [127:0]        pt_big  = {pt[0], pt[1], pt[2], pt[3]};
wire [191:0]        key_big = {key[0], key[1], key[2], key[3], key[4], key[5]};
wire [127:0]        ct;
wire                ct_valid;
reg                 wb_stb_i_r;

// Generate the acknowledgement signal
assign wb_ack_o     = wb_stb_i;

// Implement MD5 I/O memory map interface
// Write side
always @(posedge wb_clk_i)
    begin
        if(wb_rst_i)
            begin
                start       <= 0;
                start_r     <= 0;
                pt[0]       <= 0;
                pt[1]       <= 0;
                pt[2]       <= 0;
                pt[3]       <= 0;
                key[0]      <= 0;
                key[1]      <= 0;
                key[2]      <= 0;
                key[3]      <= 0;
                key[4]      <= 0;
                key[5]      <= 0;
                wb_stb_i_r  <= 0;
            end
        else begin

            // Generate registered version of some signals
            start_r         <= start;
            wb_stb_i_r      <= wb_stb_i;

            // Perform a write
            if(wb_stb_i & wb_we_i) begin
    
                case(wb_adr_i[3:0])
                    0:  start <= wb_dat_i[0];
                    1:  pt[3] <= wb_dat_i;
                    2:  pt[2] <= wb_dat_i;
                    3:  pt[1] <= wb_dat_i;
                    4:  pt[0] <= wb_dat_i;
                    5:  key[5] <= wb_dat_i;
                    6:  key[4] <= wb_dat_i;
                    7:  key[3] <= wb_dat_i;
                    8:  key[2] <= wb_dat_i;
                    9:  key[1] <= wb_dat_i;
                    10: key[0] <= wb_dat_i;
                default:
                    ;
            endcase
            end else begin // end if wb_stb_i & wb_we_i
                start       <= 1'b0;
            end
        end     // end else begin
    end // always @ (posedge wb_clk_i)

// Implement MD5 I/O memory map interface
// Read side
always @(*)
    begin
        case(wb_adr_i[3:0])
            1:
                wb_dat_o = pt[3];
            2:
                wb_dat_o = pt[2];
            3:
                wb_dat_o = pt[1];
            4:
                wb_dat_o = pt[0];
            5:
                wb_dat_o = key[5];
            6:
                wb_dat_o = key[4];
            7:
                wb_dat_o = key[3];
            8:
                wb_dat_o = key[2];
            9:
                wb_dat_o = key[1];
            10:
                wb_dat_o = key[0];
            11:
                wb_dat_o = {31'b0, ct_valid};
            12:
                wb_dat_o = ct[127:96];
            13:
                wb_dat_o = ct[95:64];
            14:
                wb_dat_o = ct[63:32];
            15:
                wb_dat_o = ct[31:0];
            default:
                wb_dat_o = 32'b0;
        endcase
    end // always @ (*)

aes_192 aes(
            .clk(wb_clk_i),
            .rst(wb_rst_i),
            .state(pt_big),
            .key(key_big),
            .start(start && ~start_r),  // start on detected position edge
            .out(ct),
            .out_valid(ct_valid)
        );

endmodule
