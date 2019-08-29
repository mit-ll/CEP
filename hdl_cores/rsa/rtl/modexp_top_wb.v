//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : modexp_top_wb.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides a wishbone based-RSA core
//

module modexp_top_wb(
           wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
           wb_stb_i, wb_we_i,
           wb_ack_o, wb_err_o, wb_dat_o,
           wb_clk_i, wb_rst_i, int_o
       );

parameter DW = 32;
parameter AW = 32;

input [AW-1:0]     wb_adr_i; //Address
input           wb_cyc_i; //bus cycle
input [DW-1:0]      wb_dat_i; //Data IN
input [3:0]       wb_sel_i; //Select Input Array
input           wb_stb_i; //Chip Select
input           wb_we_i;  //Write Or Read Enabled

output           wb_ack_o; //Acknowledge
output           wb_err_o; //Error
output reg [DW-1:0]  wb_dat_o; //Data OUT
output               int_o;    //Interrupt

input           wb_clk_i; //Clk
input           wb_rst_i; //Reset


assign wb_ack_o = 1'b1;
assign wb_err_o = 1'b0;
assign int_o = 1'b0;
wire reset_n=!wb_rst_i;

//----------------------------------------------------------------
// Internal constant and parameter definitions.
//----------------------------------------------------------------
// The operand width is the internal operand width in bits.
// The address width is the size of the address space used. This
// value must be balances with OPERAND_WIDTH to allow a total
// of 8192 bits of data. OPERAND_WIDTH * (ADDRESS_WIDTH ** 2)
// is the formula. Note that the API data with is always 32 bits.
localparam OPERAND_WIDTH         = 32;
localparam ADDRESS_WIDTH         = 8;

localparam ADDR_NAME0            = 8'h00;
localparam ADDR_NAME1            = 8'h01;
localparam ADDR_VERSION          = 8'h02;

localparam ADDR_CTRL             = 8'h08;
localparam CTRL_INIT_BIT         = 0;
localparam CTRL_NEXT_BIT         = 1;

localparam ADDR_STATUS           = 8'h09;
localparam STATUS_READY_BIT      = 0;

localparam ADDR_CYCLES_HIGH      = 8'h10;
localparam ADDR_CYCLES_LOW       = 8'h11;

localparam ADDR_MODULUS_LENGTH   = 8'h20;
localparam ADDR_EXPONENT_LENGTH  = 8'h21;

localparam ADDR_MODULUS_PTR_RST  = 8'h30;
localparam ADDR_MODULUS_DATA     = 8'h31;

localparam ADDR_EXPONENT_PTR_RST = 8'h40;
localparam ADDR_EXPONENT_DATA    = 8'h41;

localparam ADDR_MESSAGE_PTR_RST  = 8'h50;
localparam ADDR_MESSAGE_DATA     = 8'h51;

localparam ADDR_RESULT_PTR_RST   = 8'h60;
localparam ADDR_RESULT_DATA      = 8'h61;

localparam DEFAULT_MODLENGTH     = 8'h80; // 2048 bits.
localparam DEFAULT_EXPLENGTH     = 8'h80;

localparam CORE_NAME0            = 32'h6d6f6465; // "mode"
localparam CORE_NAME1            = 32'h78702020; // "xp  "
localparam CORE_VERSION          = 32'h302e3532; // "0.52"

//----------------------------------------------------------------
// Registers including update variables and write enable.
//----------------------------------------------------------------
reg [07 : 0] exponent_length_reg;
reg [07 : 0] exponent_length_new;
reg          exponent_length_we;

reg [07 : 0] modulus_length_reg;
reg [07 : 0] modulus_length_new;
reg          modulus_length_we;

reg          start_reg;
reg          start_new;


//----------------------------------------------------------------
// Wires.
//----------------------------------------------------------------
reg           exponent_mem_api_rst;
reg           exponent_mem_api_cs;
reg           exponent_mem_api_wr;
wire [31 : 0] exponent_mem_api_read_data;

reg           modulus_mem_api_rst;
reg           modulus_mem_api_cs;
reg           modulus_mem_api_wr;
wire [31 : 0] modulus_mem_api_read_data;

reg           message_mem_api_rst;
reg           message_mem_api_cs;
reg           message_mem_api_wr;
wire [31 : 0] message_mem_api_read_data;

reg           result_mem_api_rst;
reg           result_mem_api_cs;
wire [31 : 0] result_mem_api_read_data;

wire          ready;
wire [63 : 0] cycles;

//reg [31 : 0]  wb_dat_o;

//----------------------------------------------------------------
// Concurrent connectivity for ports etc.
//----------------------------------------------------------------
//assign wb_dat_o = wb_dat_o;

//----------------------------------------------------------------
// core instantiations.
//----------------------------------------------------------------
modexp_core #(.OPW(OPERAND_WIDTH), .ADW(ADDRESS_WIDTH))
            core_inst(
                .clk(wb_clk_i),
                .reset_n(reset_n),

                .start(start_reg),
                .ready(ready),

                .exponent_length(exponent_length_reg),
                .modulus_length(modulus_length_reg),

                .cycles(cycles),

                .exponent_mem_api_cs(exponent_mem_api_cs),
                .exponent_mem_api_wr(exponent_mem_api_wr),
                .exponent_mem_api_rst(exponent_mem_api_rst),
                .exponent_mem_api_write_data(wb_dat_i),
                .exponent_mem_api_read_data(exponent_mem_api_read_data),

                .modulus_mem_api_cs(modulus_mem_api_cs),
                .modulus_mem_api_wr(modulus_mem_api_wr),
                .modulus_mem_api_rst(modulus_mem_api_rst),
                .modulus_mem_api_write_data(wb_dat_i),
                .modulus_mem_api_read_data(modulus_mem_api_read_data),

                .message_mem_api_cs(message_mem_api_cs),
                .message_mem_api_wr(message_mem_api_wr),
                .message_mem_api_rst(message_mem_api_rst),
                .message_mem_api_write_data(wb_dat_i),
                .message_mem_api_read_data(message_mem_api_read_data),

                .result_mem_api_cs(result_mem_api_cs),
                .result_mem_api_rst(result_mem_api_rst),
                .result_mem_api_read_data(result_mem_api_read_data)
            );

//----------------------------------------------------------------
// reg_update
//
// Update functionality for all registers in the core.
// All registers are positive edge triggered with asynchronous
// active low reset. All registers have write enable.
//----------------------------------------------------------------
always @ (posedge wb_clk_i or negedge reset_n)
    begin
        if (!reset_n)
            begin
                start_reg           <= 1'b0;
                exponent_length_reg <= DEFAULT_EXPLENGTH;
                modulus_length_reg  <= DEFAULT_MODLENGTH;
            end
        else
            begin
                start_reg <= start_new;

                if (exponent_length_we)
                    begin
                        exponent_length_reg <= wb_dat_i[7 : 0];
                    end

                if (modulus_length_we)
                    begin
                        modulus_length_reg <= wb_dat_i[7 : 0];
                    end
            end
    end // reg_update

//----------------------------------------------------------------
// api
//
// The interface command decoding logic.
//----------------------------------------------------------------
always @*
    begin : api
        modulus_length_we    = 1'b0;
        exponent_length_we   = 1'b0;
        start_new            = 1'b0;

        modulus_mem_api_rst  = 1'b0;
        modulus_mem_api_cs   = 1'b0;
        modulus_mem_api_wr   = 1'b0;

        exponent_mem_api_rst = 1'b0;
        exponent_mem_api_cs  = 1'b0;
        exponent_mem_api_wr  = 1'b0;

        message_mem_api_rst  = 1'b0;
        message_mem_api_cs   = 1'b0;
        message_mem_api_wr   = 1'b0;

        result_mem_api_rst   = 1'b0;
        result_mem_api_cs    = 1'b0;

        wb_dat_o        = 32'h00000000;

        if (wb_stb_i)
            begin
                if (wb_we_i)
                    begin
                        case (wb_adr_i[7:0])
                            ADDR_CTRL:
                                begin
                                    start_new = wb_dat_i[0];
                                end

                            ADDR_MODULUS_LENGTH:
                                begin
                                    modulus_length_we = 1'b1;
                                end

                            ADDR_EXPONENT_LENGTH:
                                begin
                                    exponent_length_we = 1'b1;
                                end

                            ADDR_MODULUS_PTR_RST:
                                begin
                                    modulus_mem_api_rst = 1'b1;
                                end

                            ADDR_MODULUS_DATA:
                                begin
                                    modulus_mem_api_cs = 1'b1;
                                    modulus_mem_api_wr = 1'b1;
                                end

                            ADDR_EXPONENT_PTR_RST:
                                begin
                                    exponent_mem_api_rst = 1'b1;
                                end

                            ADDR_EXPONENT_DATA:
                                begin
                                    exponent_mem_api_cs = 1'b1;
                                    exponent_mem_api_wr = 1'b1;
                                end

                            ADDR_MESSAGE_PTR_RST:
                                begin
                                    message_mem_api_rst = 1'b1;
                                end

                            ADDR_MESSAGE_DATA:
                                begin
                                    message_mem_api_cs = 1'b1;
                                    message_mem_api_wr = 1'b1;
                                end

                            ADDR_RESULT_PTR_RST:
                                begin
                                    result_mem_api_rst = 1'b1;
                                end

                            default:
                                begin
                                end
                        endcase // case (wb_adr_i[7 : 0])
                    end // if (wb_we_i)
                else
                    begin
                        case (wb_adr_i[7:0])
                            ADDR_NAME0:
                                wb_dat_o = CORE_NAME0;

                            ADDR_NAME1:
                                wb_dat_o = CORE_NAME1;

                            ADDR_VERSION:
                                wb_dat_o = CORE_VERSION;

                            ADDR_CTRL:
                                wb_dat_o = {31'h00000000, start_reg};

                            ADDR_STATUS:
                                wb_dat_o = {31'h00000000, ready};

                            ADDR_CYCLES_HIGH:
                                wb_dat_o = cycles[63 : 32];

                            ADDR_CYCLES_LOW:
                                wb_dat_o = cycles[31 : 0];

                            ADDR_MODULUS_LENGTH:
                                wb_dat_o = {24'h000000, modulus_length_reg};

                            ADDR_EXPONENT_LENGTH:
                                wb_dat_o = {24'h000000, exponent_length_reg};

                            ADDR_MODULUS_DATA:
                                begin
                                    modulus_mem_api_cs = 1'b1;
                                    wb_dat_o      = modulus_mem_api_read_data;
                                end

                            ADDR_EXPONENT_DATA:
                                begin
                                    exponent_mem_api_cs = 1'b1;
                                    wb_dat_o       = exponent_mem_api_read_data;
                                end

                            ADDR_MESSAGE_DATA:
                                begin
                                    message_mem_api_cs = 1'b1;
                                    wb_dat_o      = message_mem_api_read_data;
                                end

                            ADDR_RESULT_DATA:
                                begin
                                    result_mem_api_cs = 1'b1;
                                    wb_dat_o     = result_mem_api_read_data;
                                end

                            default:
                                begin
                                end
                        endcase // case (wb_adr_i)
                    end // else: !if(wb_we_i)
            end // if (wb_stb_i)
    end // block: api

endmodule // modexp_top_wb
