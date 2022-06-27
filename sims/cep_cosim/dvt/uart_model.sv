//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      uart_model.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    CEP Co-Simulation Top Level Testbench 
// Notes:          UART Model that will receive and log a line of text
//
//--------------------------------------------------------------------------------------

`include "suite_config.v"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_cmds.incl"
`include "v2c_top.incl"

module uart_model #(
  parameter   BIT_RATE        = 9600,
  parameter   CLK_HZ          = 50_000_000,
  parameter   PAYLOAD_BITS    = 8,
  parameter   STOP_BITS       = 1
) (
  input  wire       clk          ,
  input  wire       resetn       ,
  input  wire       uart_rxd     ,
  input  wire       uart_rx_en
);

  localparam LINE_BUFFER_MAX_LENGTH     = 132;
  reg[LINE_BUFFER_MAX_LENGTH*8 - 1:0]   line_buffer_reg = 0;
  wire                                  uart_rx_valid;
  wire                                  uart_rx_break;
  wire [7:0]                            uart_rx_data;

  // Testbench UART receiver
  uart_rx #(
    .BIT_RATE(BIT_RATE),
    .CLK_HZ(CLK_HZ),
    .PAYLOAD_BITS(PAYLOAD_BITS),
    .STOP_BITS(STOP_BITS)
  ) uart_rx_inst (
    .clk              (clk),
    .resetn           (resetn),
    .uart_rxd         (uart_rxd),
    .uart_rx_en       (uart_rx_en),
    .uart_rx_break    (uart_rx_break),
    .uart_rx_valid    (uart_rx_valid),
    .uart_rx_data     (uart_rx_data)
  );  


  always @(posedge clk)
  begin
    if (uart_rx_valid) begin
      `logI("TB_UART = %x", uart_rx_data);

      line_buffer_reg = {line_buffer_reg[LINE_BUFFER_MAX_LENGTH*8 - 9:0], uart_rx_data};

      if (uart_rx_data == 8'h0a) begin
        `logI("TB_UART_LINE: %0s", line_buffer_reg);
        line_buffer_reg = 0;
      end

    end // end if (uart_rx_valid)
  end   // always @(clk)

endmodule
