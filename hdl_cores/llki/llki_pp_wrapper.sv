//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       llki_pp_wrapper.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     This file provides a Verilog <-> SystemVerilog adapter
//                  allowing connection of TL-UL interface to the Chisel
//                  blackbox.  
//
//                  It also implements the LLKI Protocol Processing
//                  block's State Machine.
// Notes:           The underlying TL-UL package is from the OpenTitan
//                  project
//
//                  The "tl" parameters have been taken from the
//                  OpenTitan ecosystem
//
//                  Send / Recv FIFOs have specifically NOT been
//                  implemented here in an effort to minimize the number
//                  of memory blocks that ever touch key material.
//
//                  The LLKI Protocol Procesing block is intended to be
//                  common across ALL LLKI enabled cores.  Unique address
//                  decoding will be facilitated through the use of the
//                  CTRLSTS_ADDR and SENDRECV_ADDR parameters (which are
//                  passed down from Chisel)
//
//************************************************************************
`timescale 1ns/1ns

module llki_pp_wrapper import tlul_pkg::*; import llki_pkg::*; #(
  parameter int CTRLSTS_ADDR    = 32'h00000000,   // These default values MUST be overwritten
  parameter int SENDRECV_ADDR   = 32'h00000008    // These default values MUST be overwritten
 ) (

  // Clock and reset
  input                         clk,
  input                         rst,

  // Slave Interface - A channel
  input [2:0]                   slave_a_opcode,
  input [2:0]                   slave_a_param,
  input [top_pkg::TL_SZW-1:0]   slave_a_size,
  input [top_pkg::TL_AIW-1:0]   slave_a_source,
  input [top_pkg::TL_AW-1:00]   slave_a_address,
  input [top_pkg::TL_DBW-1:0]   slave_a_mask,
  input [top_pkg::TL_DW-1:0]    slave_a_data,
  input                         slave_a_corrupt,
  input                         slave_a_valid,
  output                        slave_a_ready,

  // Slave interface D channel
  output [2:0]                  slave_d_opcode,
  output [2:0]                  slave_d_param,
  output [top_pkg::TL_SZW-1:0]  slave_d_size,
  output [top_pkg::TL_AIW-1:0]  slave_d_source,
  output [top_pkg::TL_DIW-1:0]  slave_d_sink,
  output                        slave_d_denied,
  output [top_pkg::TL_DW-1:0]   slave_d_data,
  output                        slave_d_corrupt,
  output                        slave_d_valid,
  input                         slave_d_ready,

  // LLKI discrete I/O
  output reg [63:0]             llkid_key_data,
  output reg                    llkid_key_valid,
  input                         llkid_key_ready,
  input                         llkid_key_complete,
  output reg                    llkid_clear_key,
  input                         llkid_clear_key_ack

);

  // Create the structures for communicating with OpenTitan-based Tilelink
  tl_h2d_t                      slave_tl_h2d;
  tl_d2h_t                      slave_tl_d2h;

  // Make Slave A channel connections
  assign slave_tl_h2d.a_valid     = slave_a_valid;
  assign slave_tl_h2d.a_opcode    = ( slave_a_opcode == 3'h0) ? PutFullData : 
                                  ((slave_a_opcode == 3'h1) ? PutPartialData : 
                                  ((slave_a_opcode == 3'h4) ? Get : 
                                    Get));                                   
  assign slave_tl_h2d.a_param     = slave_a_param;
  assign slave_tl_h2d.a_size      = slave_a_size;
  assign slave_tl_h2d.a_source    = slave_a_source;
  assign slave_tl_h2d.a_address   = slave_a_address;
  assign slave_tl_h2d.a_mask      = slave_a_mask;
  assign slave_tl_h2d.a_data      = slave_a_data;
  assign slave_tl_h2d.a_user      = tl_a_user_t'('0);  // User field is unused by Rocket Chip
  assign slave_tl_h2d.d_ready     = slave_d_ready;
  
  // Make Slave D channel connections
  // Converting from the OpenTitan enumerated type to specific bit mappings
  assign slave_d_opcode         = ( slave_tl_d2h.d_opcode == AccessAck)     ? 3'h0 :
                                  ((slave_tl_d2h.d_opcode == AccessAckData) ? 3'h1 :
                                    3'h0);
  assign slave_d_param          = slave_tl_d2h.d_param;
  assign slave_d_size           = slave_tl_d2h.d_size;
  assign slave_d_source         = slave_tl_d2h.d_source;
  assign slave_d_sink           = slave_tl_d2h.d_sink;
  assign slave_d_denied         = slave_tl_d2h.d_error; // Open
  assign slave_d_data           = slave_tl_d2h.d_data;
  assign slave_d_corrupt        = slave_tl_d2h.d_error;
  assign slave_d_valid          = slave_tl_d2h.d_valid;
  assign slave_a_ready          = slave_tl_d2h.a_ready;

  // Define some of the wires and registers associated with the tlul_adapter_reg
  wire                          reg_we_o;
  wire                          reg_re_o;
  reg                           reg_we_o_d1;
  wire [top_pkg::TL_AW-1:0]     reg_addr_o;
  wire [top_pkg::TL_DW-1:0]     reg_wdata_o;
  reg [top_pkg::TL_DW-1:0]      reg_rdata_i;
  wire                          ack_i;
  reg                           reg_error_i;

  // Misc. signals
  reg [1:0]                     llkipp_ctrlstatus_register;   // Bit definition can be found in llki_pkg.sv
                                                              // Register width explicitly minimized to only
                                                              // the required bits (to increaase coverage)
  reg                           write_error;
  reg                           read_error;

  //------------------------------------------------------------------------
  // Instantitate a tlul_adapter_reg to adapt the TL Slave Interface
  //------------------------------------------------------------------------
  tlul_adapter_reg #(
    .RegAw          (top_pkg::TL_AW   ),
    .RegDw          (top_pkg::TL_DW   )
  ) u_tlul_adapter_reg_inst (
    .clk_i          (clk              ),
    .rst_ni         (~rst             ),

    .tl_i           (slave_tl_h2d     ),
    .tl_o           (slave_tl_d2h     ),

    .we_o           (reg_we_o         ),
    .re_o           (reg_re_o         ),
    .addr_o         (reg_addr_o       ),
    .wdata_o        (reg_wdata_o      ),
    .be_o           (                 ),  // Accesses are assumed to be word-wide
    .rdata_i        (reg_rdata_i      ),
    .ack_i          (ack_i            ),    // External acknowledgement of the
                                            // transaction
    .error_i        (reg_error_i      )
  );

  // The reg_error_i will be asserted if either a read or write error occurs
  assign reg_error_i    = read_error || write_error;

  // The acknowledgement signal allows for latching of the read data when
  // available (if it is a read) and/or proper processing of the
  // external error
  assign ack_i          = reg_re_o ||  reg_we_o_d1;
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Write Decode Process (writing to the state machine sendrecv address
  // be handled in the LLKI PP state machine)
  //------------------------------------------------------------------------
  reg                         llkipp_response_waiting;
  reg [top_pkg::TL_DW-1:0]    llkipp_response_word;

  always @(posedge clk or posedge rst)
  begin
    if (rst) begin
      llkipp_ctrlstatus_register    <= '0;
      write_error                   <= 1'b0;
      reg_we_o_d1                   <= 1'b0;
    end else begin

      // Registered version of the write enable
      reg_we_o_d1                   <= reg_we_o;

      // Default signal assignments
      write_error                   <= 1'b0;

      // Capture the message available state (setting and clearing
      // of the source bit will occur within the state machine always block)
      llkipp_ctrlstatus_register[LLKIKL_CTRLSTS_RESP_WAITING]   <= llkipp_response_waiting;

      // The LLKI-PP provides minimal buffering for messages (and key words) received from
      // Thus,  the SRoT can poll the ready for key bit in the LLKI-PP Control/Status register
      // to determine that the LLKI-PP (and TSS) is ready to receive the next key word
      llkipp_ctrlstatus_register[LLKIKL_CTRLSTS_READY_FOR_KEY]  <= llkid_key_ready;
      
      if (reg_we_o) begin
        case (reg_addr_o)
          // Currently, no Ctrl/Status bits are writeable via TileLink
          CTRLSTS_ADDR    : begin
            ;
          end
          // Write to Send/Recv Address - Data "capture" occurs within the
          // LLKI PP State Machine block, thus we have a null action here
          SENDRECV_ADDR  : begin
            ;
          end
          // Trap State - Currently, the control/status register has no
          // LLKI-PP writable bits and thus c
          default             : begin
            write_error               <= 1'b1;
          end
        endcase
      end // end if (reg_we_o)

    end // end if (rst)
  end // end always
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Read decode process
  //------------------------------------------------------------------------
  always @*
  begin

    // Default signal assignments
    reg_rdata_i             = '0;
    read_error              = 1'b0;

    if (reg_re_o) begin
      case (reg_addr_o)
        // Currently, no Ctrl/Status bits are writeable via TileLink
        // CTRLSTS_ADDR explicitly mapped (see llkipp_ctrlstatus_register decleration above)
        CTRLSTS_ADDR    : begin
          reg_rdata_i[1:0]  = llkipp_ctrlstatus_register;
        end
        // Write to Send/Recv Address - Data "capture" occurs within the
        // LLKI PP State Machine block, thus we have a null action here
        SENDRECV_ADDR  : begin
          reg_rdata_i       = llkipp_response_word;
        end
        // Trap State - Invalid addresses
        default             : begin
          read_error        = 1'b1;
        end
      endcase
    end // end if (reg_re_o)
  end // end always
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // LLKI PP State Machine
  //------------------------------------------------------------------------
  LLKIPP_STATE_TYPE           llkipp_current_state;
  reg [7:0]                   msg_id;
  reg [7:0]                   status;
  reg [15:0]                  msg_len;

  always @(posedge clk or posedge rst)
  begin
    if (rst) begin
      llkid_key_data                <= '0;
      llkid_key_valid               <= '0;
      llkid_clear_key               <= '0;
      llkipp_response_waiting       <= '0;
      llkipp_response_word          <= '0;
      msg_id                        <= '0;
      status                        <= '0;
      msg_len                       <= '0;
      llkipp_current_state          <= ST_LLKIPP_IDLE;
    end else begin
      case (llkipp_current_state)
        //------------------------------------------------------------------
        // IDLE State
        //------------------------------------------------------------------
        ST_LLKIPP_IDLE                  : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          msg_id                        <= '0;
          status                        <= '0;
          msg_len                       <= '0;
          llkipp_current_state          <= ST_LLKIPP_IDLE;

          // If a write occurs AND it's to the SENDRECV_ADDR
          if (reg_we_o && reg_addr_o == SENDRECV_ADDR) begin
            msg_id                      <= reg_wdata_o[7:0];
            status                      <= reg_wdata_o[15:8];
            msg_len                     <= reg_wdata_o[31:16];

            // Now that we have captured the message, time to process
            llkipp_current_state        <= ST_LLKIPP_MESSAGE_CHECK;

          end // end if (reg_we_o && reg_addr_o == SENDRECV_ADDR)

        end   // ST_LLKIPP_IDLE
        //------------------------------------------------------------------
        // Message Check State
        //------------------------------------------------------------------
        ST_LLKIPP_MESSAGE_CHECK         : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          llkipp_current_state          <= ST_LLKIPP_MESSAGE_CHECK;

          // Make some checks and decision based on the Message ID
          case (msg_id) 
            LLKI_MID_KLLOADKEYREQ       : begin
              // A message length of 1 (or zero) for a load key request
              // is invalid
              if (msg_len <= 1) begin
                msg_id                  <= LLKI_MID_KLERRORRESP;
                status                  <= LLKI_STATUS_KL_REQ_BAD_MSG_LEN;
                llkipp_current_state    <= ST_LLKIPP_RESPONSE;
              // If a key load attempt occurs when there is already a key, then
              // generate an error
              end else if (llkid_key_complete) begin
                msg_id                  <= LLKI_MID_KLERRORRESP;
                status                  <= LLKI_STATUS_KL_KEY_OVERWRITE;
                llkipp_current_state    <= ST_LLKIPP_RESPONSE;
              // A load key request has been issued, jump to the
              // load key words and wait for the next word (which
              // will be treated as a key word)
              end else
                llkipp_current_state    <= ST_LLKIPP_LOAD_KEY_WORDS;
            end
            LLKI_MID_KLCLEARKEYREQ      : begin
              // The only valid message length is 1
              if (msg_len != 1) begin
                msg_id                  <= LLKI_MID_KLERRORRESP;
                status                  <= LLKI_STATUS_KL_REQ_BAD_MSG_LEN;
                llkipp_current_state    <= ST_LLKIPP_RESPONSE;
              end else 
                llkipp_current_state    <= ST_LLKIPP_CLEAR_KEY;
            end
            LLKI_MID_KLKEYSTATUSREQ     : begin
              // The only valid message length is 1
              if (msg_len != 1) begin
                msg_id                  <= LLKI_MID_KLERRORRESP;
                status                  <= LLKI_STATUS_KL_REQ_BAD_MSG_LEN;
                llkipp_current_state    <= ST_LLKIPP_RESPONSE;
              end else begin
                msg_id                  <= LLKI_MID_KLKEYSTATUSRESP;
                // Set the status based on what state the LLKI-Discrete
                // interface indicates
                if (llkid_key_complete)
                  status                 <= LLKI_STATUS_KEY_PRESENT;
                else
                  status                 <= LLKI_STATUS_KEY_NOT_PRESENT;
                llkipp_current_state    <= ST_LLKIPP_RESPONSE;
              end
            end
            // All other message ID (error condition)
            default                     : begin
              msg_id                    <= LLKI_MID_KLERRORRESP;
              status                    <= LLKI_STATUS_KL_REQ_BAD_MSG_ID;
              llkipp_current_state      <= ST_LLKIPP_RESPONSE;
            end
          endcase   // msg id
        end   // ST_LLKIPP_MESSAGE_CHECK
        //------------------------------------------------------------------
        // Load Key Words State - This state will "pass" the key words
        // received over the LLKI-PP TileLink interface to the
        // LLKI-Discrete interface.
        //
        // It is the responsibility of the SRoT to read the status of the
        // key ready bit between EACH KEY WORD before sending the next one
        //------------------------------------------------------------------
        ST_LLKIPP_LOAD_KEY_WORDS        : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          llkipp_current_state          <= ST_LLKIPP_LOAD_KEY_WORDS;

          // Another key word has been received, time to do some checks
          if (reg_we_o && reg_addr_o == SENDRECV_ADDR) begin

            // If for some reason, the LLKI-Discrete is not ready, we'll
            // ignore the key word, and send an error response (which the
            // SRoT will need to check for)
            if (~llkid_key_ready) begin
              msg_id                    <= LLKI_MID_KLERRORRESP;
              status                    <= LLKI_STATUS_KL_LOSS_OF_SYNC;
              llkipp_current_state      <= ST_LLKIPP_RESPONSE;
            // We have attempted to load a key whose length does not match the expected
            // key length for this core.  Clear the core, and send an error response
            end else if (llkid_key_complete) begin
              msg_id                    <= LLKI_MID_KLERRORRESP;
              status                    <= LLKI_STATUS_KL_BAD_KEY_LEN;
              llkipp_current_state      <= ST_LLKIPP_CLEAR_KEY;
            // If a key word has been received and msg_len == 2, then this
            // is the LAST word of the load key request (understanding the
            // msg_len also includes the header and we are using it for
            // a counter.
            end else if (msg_len == 2) begin
              llkid_key_data            <= reg_wdata_o;
              llkid_key_valid           <= 1;

              msg_id                    <= LLKI_MID_KLLOADKEYACK;
              status                    <= LLKI_STATUS_GOOD;
              llkipp_current_state      <= ST_SROT_KL_WAIT_FOR_COMPLETE;
            // This is not the last word of the key load, load the key word
            // via the LLKI-Discrete and just wait for the next word
            end else begin
              llkid_key_data            <= reg_wdata_o;
              llkid_key_valid           <= 1;

              // Decrement the message length (be used to count the remaining
              // number of key words to load)
              msg_len                   <= msg_len - 1;
            end // end if (~llkid_key_ready)
          end // end if (reg_we_o && reg_addr_o == SENDRECV_ADDR)

        end   // ST_LLKIPP_LOAD_KEY_WORDS
        //------------------------------------------------------------------
        // After all key words have been loaded into the selected TSS,
        // wait for llkid_key_complete to be asserted before sending
        // the response 
        //------------------------------------------------------------------
        ST_SROT_KL_WAIT_FOR_COMPLETE  : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          llkipp_current_state          <= ST_SROT_KL_WAIT_FOR_COMPLETE;

          // When llkid_key_complete is asserted, then jump to sending the response
          if (llkid_key_complete) begin
            llkipp_current_state      <= ST_LLKIPP_RESPONSE;
          end // end if (llkid_key_complete)

        end // ST_SROT_KL_WAIT_FOR_COMPLETE
        //------------------------------------------------------------------
        // Clear Key State
        //------------------------------------------------------------------
        ST_LLKIPP_CLEAR_KEY             : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '1;  // Instruct TSS to clear the key
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          llkipp_current_state          <= ST_LLKIPP_CLEAR_KEY;

          // Clear Key has been acknowledged, time to send the response
          if (llkid_clear_key_ack) begin

            // If we got here due to a normal clear key request, then
            // we need to assign the msg_id and status, otherwise
            // just jump to the response state
            if (msg_id != LLKI_MID_KLERRORRESP) begin
              msg_id                    <= LLKI_MID_KLCLEARKEYACK;
              status                    <= LLKI_STATUS_GOOD;
            end

            // Jump to the response state
            llkipp_current_state        <= ST_LLKIPP_RESPONSE;
          end

        end // ST_LLKIPP_CLEAR_KEY
        //------------------------------------------------------------------
        // Response Message state
        //
        // As there is no FIFO in the LLKI-PP, the State Machine more
        // closurely tracks the message exchange.  Here, when we are
        // sending a response, the STM will advance when the response
        // is read.
        //
        //------------------------------------------------------------------
        ST_LLKIPP_RESPONSE              : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          llkipp_current_state          <= ST_LLKIPP_RESPONSE;

          // Build the response word and indicate that a response is waiting (remember, no FIFOs)
          llkipp_response_word[7:0]     <= msg_id;
          llkipp_response_word[15:8]    <= status;
          llkipp_response_word[31:16]   <= 1;       // All responses have a message len of 1
          llkipp_response_waiting       <= '1;

          // When the SEND/RECV address is read via the register interface, we can return to idle
          if (reg_re_o && reg_addr_o == SENDRECV_ADDR) begin
            llkipp_current_state        <= ST_LLKIPP_IDLE;
          end   // end if (reg_re_o && reg_addr_o == SENDRECV_ADDR)

        end
        //------------------------------------------------------------------
        // Trap State
        //------------------------------------------------------------------
        default                       : begin
          // Default signal assignments
          llkid_key_data                <= '0;
          llkid_key_valid               <= '0;
          llkid_clear_key               <= '0;
          llkipp_response_waiting       <= '0;
          llkipp_response_word          <= '0;
          msg_id                        <= '0;
          status                        <= '0;
          msg_len                       <= '0;
          llkipp_current_state          <= ST_LLKIPP_IDLE;
        end
      endcase // llkipp_current_state
    end // end if (rst)
  end   // end always
  //------------------------------------------------------------------------


endmodule   // endmodule llki_pp_wrapper
