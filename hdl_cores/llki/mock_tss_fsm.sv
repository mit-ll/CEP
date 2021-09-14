//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       mock_tss_fsm.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     This file is intended as a common finite state machine
//                  for all instances of the Mock Technique Specific Shim.
//                  How the llkid_key_regis ter is used is up to the higher
//                  level module
// Notes:           The KEY_WORDS parameters is used to define the number
//                  "words" (64-bits) you need for the current instance.
//
//                  Additionally, the worrd load order is from High --> Low
//************************************************************************
`timescale 1ns/1ns

module mock_tss_fsm import llki_pkg::*; #(
  parameter int KEY_WORDS       = 2
) (

  // Clock and reset
  input                             clk,
  input                             rst,

  // LLKI Discrete I/O
  input [63:0]                      llkid_key_data,
  input                             llkid_key_valid,
  output reg                        llkid_key_ready,
  output reg                        llkid_key_complete,
  input                             llkid_clear_key,
  output reg                        llkid_clear_key_ack,

  // Key Register Output
  output reg [(64*KEY_WORDS) - 1:0] llkid_key_register
);

  // Internal signals
  reg [7:0]                         llkid_key_word_counter;
  reg [7:0]                         wait_state_counter;
  MOCKTSS_STATE_TYPE                current_state;

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
      llkid_key_word_counter  <= 0; 
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
          llkid_clear_key_ack     <= '0;
          llkid_key_word_counter  <= 0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_IDLE;

          // If a clear key is requested while in the IDLE state, the STM
          // will immediately acknowledge the clearing AND clear the key
          if (llkid_clear_key) begin
            llkid_clear_key_ack     <= '1;
            llkid_key_complete      <= '0;
            llkid_key_register      <= '0;
          // Load the first key word into the appropriate register based on the current
          // state of the llkid_key_word_counter
          // Words coming from the SRoT are Little Endian
          end else if (llkid_key_valid) begin
            llkid_key_ready                                       <= '0;
            llkid_key_register[(64*llkid_key_word_counter) +: 64] <= llkid_key_data;

            // Jump to the next state
            current_state                                         <= ST_MOCKTSS_WAIT_STATE0;
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

          // When the wait has reached zero, jump to the next state deepending
          // on the value of the word counter (do we have any more words left
          // for the key)
          if (wait_state_counter == 0) begin

            // No more words exist, now jump to another wait state
            if (llkid_key_word_counter == KEY_WORDS - 1) begin 
              wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
              current_state           <= ST_MOCKTSS_WAIT_STATE2;
            end else begin
              // Increment the word counter
              llkid_key_word_counter  <= llkid_key_word_counter + 1;

              // Jump to the next state
              current_state           <= ST_MOCKTSS_WAIT_FOR_NEXT_KEY_WORD;

            end // end if (llkid_key_word_counter == 0)
          // A clear key request has been received
          end else if (llkid_clear_key) begin
            current_state             <= ST_MOCKTSS_CLEAR_KEY;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE0
        //------------------------------------------------------------------
        // Mock TSS - Wait for another key word
        //------------------------------------------------------------------
        ST_MOCKTSS_WAIT_FOR_NEXT_KEY_WORD   : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_WAIT_FOR_NEXT_KEY_WORD;

          // If a clear key, jump to the clear key state
          if (llkid_clear_key) begin
            current_state                                         <= ST_MOCKTSS_CLEAR_KEY;
          // Load the next key word
          end else if (llkid_key_valid) begin
            llkid_key_ready                                       <= '0;
            llkid_key_register[(64*llkid_key_word_counter) +: 64] <= llkid_key_data;

            // Jump to the next state
            current_state                                         <= ST_MOCKTSS_WAIT_STATE0;
          end // end if (llkid_clear_key)
        end // ST_MOCKTSS_WAIT_FOR_NEXT_KEY_WORD
        //------------------------------------------------------------------
        // Mock TSS - Clear Key State
        //------------------------------------------------------------------
        ST_MOCKTSS_CLEAR_KEY      : begin
          // Default signal assignments
          llkid_key_ready         <= '0;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          llkid_key_register      <= '0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_WAIT_STATE1;
        end
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
            llkid_clear_key_ack     <= '1;
            current_state           <= ST_MOCKTSS_IDLE;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE1
        //------------------------------------------------------------------
        // Mock TSS - Wait State 2 - Wait for some cycles after the
        //   final key word is loaded before assert llkid_key_complete
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
            llkid_key_complete      <= '1;
            current_state           <= ST_MOCKTSS_IDLE;
          end // end if (wait_state_counter == 0)

        end // ST_MOCKTSS_WAIT_STATE2
        //------------------------------------------------------------------
        // Mock TSS - Trap State
        //------------------------------------------------------------------
        default                   : begin
          // Default signal assignments
          llkid_key_ready         <= '1;
          llkid_key_complete      <= '0;
          llkid_clear_key_ack     <= '0;
          llkid_key_register      <= '0;
          llkid_key_word_counter  <= 0;
          wait_state_counter      <= MOCKTSS_WAIT_STATE_COUNTER_INIT;
          current_state           <= ST_MOCKTSS_IDLE;
        end
      endcase
    end // end if (rst)
  end // end always
  //------------------------------------------------------------------


endmodule // end mock_tss_fsm

