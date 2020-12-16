//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:       aes_192_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module aes_192_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst, 

    // Core I/O
    input wire            start,
    input wire [127:0]    state,
    input wire [191:0]    key,
    output wire [127:0]   out,
    output wire           out_valid,

    // LLKI Discrete I/O
    input [63:0]          llkid_key_data,
    input                 llkid_key_valid,
    output reg            llkid_key_ready,
    output reg            llkid_key_complete,
    input                 llkid_clear_key,
    output reg            llkid_clear_key_ack

);

  reg [127:0]           llkid_key_register;
  wire [127:0]          mock_tss_state;
  reg [7:0]             wait_state_counter;
  MOCKTSS_STATE_TYPE    current_state;

  //------------------------------------------------------------------
  // Mock TSS State Machine
  //
  // The Mock TSS introduces artificial wait states to demonstrate
  // a delay when loading or clearing keys
  //------------------------------------------------------------------
  always @(posedge clk or posedge rst)
  begin
    if (rst) begin
      llkid_key_ready         <= '1;
      llkid_key_complete      <= '0;
      llkid_clear_key_ack     <= '0;
      llkid_key_register      <= '0;
      wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
      current_state           <= ST_MOCKTSS_IDLE;
    end else begin
      case (current_state)
        //------------------------------------------------------------------
        // Mock TSS - Idle State
        //------------------------------------------------------------------
        ST_MOCKTSS_IDLE         : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          llkid_key_register       <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_IDLE;

          // If a clear key is requested while in the IDLE state, the STM
          // will immediately acknowledge the clearing
          if (llkid_clear_key) begin
            llkid_clear_key_ack   <= '1;
          end else if (llkid_key_valid) begin
            llkid_key_ready           <= '0;
            llkid_key_register[63:0]  <= llkid_key_data;
            current_state             <= ST_MOCKTSS_WAIT_STATE0;
          end // end if (llkid_clear_key)

        end
        //------------------------------------------------------------------
        // Mock TSS - Wait State 0
        //------------------------------------------------------------------
        ST_MOCKTSS_WAIT_STATE0    : begin
          // Default signal assignments
          llkid_key_ready         <= '0;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          current_state           <= ST_MOCKTSS_WAIT_STATE0;

          // Decrement the wait state counter
          wait_state_counter      <= wait_state_counter - 1;

          // Jump when the wait state counter has reached zero
          if (wait_state_counter == 0) begin
            current_state         <= ST_MOCKTSS_KEY0_LOADED;
          end else if (llkid_clear_key) begin
            current_state         <= ST_MOCKTSS_CLEAR_KEY;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE0
        //------------------------------------------------------------------
        // Mock TSS - Key Word 0 has been loaded
        //------------------------------------------------------------------
        ST_MOCKTSS_KEY0_LOADED    : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_KEY0_LOADED;

          // If a clear key, jump to the clear key state
          if (llkid_clear_key) begin
            current_state               <= ST_MOCKTSS_CLEAR_KEY;
          end else if (llkid_key_valid) begin
            llkid_key_ready             <= '0;
            llkid_key_register[127:64]  <= llkid_key_data;
            current_state               <= ST_MOCKTSS_WAIT_STATE1;
          end // end if (llkid_clear_key)
        end // ST_MOCKTSS_KEY0_LOADED  
        //------------------------------------------------------------------
        // Mock TSS - Wait State 1
        //------------------------------------------------------------------
        ST_MOCKTSS_WAIT_STATE1    : begin
          // Default signal assignments
          llkid_key_ready         <= '0;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          current_state           <= ST_MOCKTSS_WAIT_STATE1;

          // Decrement the wait state counter
          wait_state_counter      <= wait_state_counter - 1;

          // Jump when the wait state counter has reached zero
          if (wait_state_counter == 0) begin
            current_state           <= ST_MOCKTSS_KEY1_LOADED;
          end else if (llkid_clear_key) begin
            current_state         <= ST_MOCKTSS_CLEAR_KEY;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE1
        //------------------------------------------------------------------
        // Mock TSS - Key Word 1 has been loaded.  Sit here until
        //   the key is cleared OR an attempt to reload the key occurs.
        //
        // Note: Accepting a new key while in this state.  Either
        //   this state needs to be able to accept a new key OR care
        //   must be take verify key load state (through a key status
        //   requyest) before loading a new key.  
        //
        //   Additional "robustness" will be explored in future LLKI
        //   releases.
        //
        //------------------------------------------------------------------
        ST_MOCKTSS_KEY1_LOADED    : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '1;
          llkid_clear_key_ack     <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_KEY1_LOADED;

          // If a clear key, jump to the clear key state.  
          if (llkid_clear_key) begin
            current_state         <= ST_MOCKTSS_CLEAR_KEY;
          end else if (llkid_key_valid) begin
            llkid_key_ready           <= '0;
            llkid_key_register[63:0]  <= llkid_key_data;
            current_state             <= ST_MOCKTSS_WAIT_STATE0;
          end // end if (llkid_clear_key)
        end // ST_MOCKTSS_KEY0_LOADED  
        //------------------------------------------------------------------
        // Mock TSS - Clear Key State
        //------------------------------------------------------------------
        ST_MOCKTSS_CLEAR_KEY      : begin
          // Default signal assignments
          llkid_key_ready         <= '0;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          llkid_key_register       <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_WAIT_STATE2;
        end
        //------------------------------------------------------------------
        // Mock TSS - Wait State 2
        //------------------------------------------------------------------
        ST_MOCKTSS_WAIT_STATE2    : begin
          // Default signal assignments
          llkid_key_ready         <= '0;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          current_state           <= ST_MOCKTSS_WAIT_STATE2;

          // Decrement the wait state counter
          wait_state_counter      <= wait_state_counter - 1;

          // Jump when the wait state counter has reached zero
          if (wait_state_counter == 0) begin
            llkid_clear_key_ack     <= '1;
            current_state           <= ST_MOCKTSS_IDLE;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE1
        //------------------------------------------------------------------
        // Mock TSS - Trap State
        //------------------------------------------------------------------
        default                   : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          llkid_key_register       <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_IDLE;
        end
      endcase
    end // end if (rst)
  end // end always
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Create the Mock TSS input into the AES core
  //------------------------------------------------------------------
  assign mock_tss_state[63:0]   = AES_MOCK_TSS_KEY_0 ^
                                  llkid_key_register[63:0] ^
                                  state[63:0];
  assign mock_tss_state[127:64] = AES_MOCK_TSS_KEY_1 ^
                                  llkid_key_register[127:64] ^
                                  state[127:64];
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the AES core
  //------------------------------------------------------------------
  aes_192 aes_192_inst (
    .clk          (clk),
    .rst          (rst),
    .start        (start),
    .state        (mock_tss_state),
    .key          (key),
    .out          (out),
    .out_valid    (out_valid)
  );
  //------------------------------------------------------------------

endmodule

