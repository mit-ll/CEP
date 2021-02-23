//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:       srot_wrapper.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     This file provides a Verilog <-> SystemVerilog adapter
//                  allowing connection of TL-UL interface to the Chisel
//                  blackbox.  
// Notes:           The underlying TL-UL package is from the OpenTitan
//                  project
//
//                  The "tl" parameters have been taken from the
//                  OpenTitan ecosystem
//
//                  The SRoT is a SINGLE THREADED DEVICE.
//
//                  As such, care should be taken when using the
//                  SRoT in a multi-core environment.  Care should be
//                  take that multiple cores are NOT accessing the
//                  SRoT at the same time.
//
//************************************************************************
`timescale 1ns/1ns

module srot_wrapper import tlul_pkg::*; import llki_pkg::*; #(
  parameter int ADDRESS     = 32'h00000000,   // Currently unused as address selected occurs @ the SRoT Chisel Code
  parameter int DEPTH       = 32'h00000100,   // Currently unused as address selected occurs @ the SRoT Chisel Code
  parameter int FIFO_DEPTH  = 8,              // Define the depth of the LLKI FIFOs

  // Derived parameters
  localparam int DepthW     = prim_util_pkg::vbits(FIFO_DEPTH + 1)
 ) (

  // Clock and reset
  input                         clk,
  input                         rst,

  // Slave interface A channel
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

  // Master interface A channel
  output [2:0]                  master_a_opcode,
  output [2:0]                  master_a_param,
  output [top_pkg::TL_SZW-1:0]  master_a_size,
  output [top_pkg::TL_AIW-1:0]  master_a_source,
  output [top_pkg::TL_AW-1:00]  master_a_address,
  output [top_pkg::TL_DBW-1:0]  master_a_mask,
  output [top_pkg::TL_DW-1:0]   master_a_data,
  output                        master_a_corrupt,
  output                        master_a_valid,
  input                         master_a_ready,

  // Master interface D channel
  input [2:0]                   master_d_opcode,
  input [2:0]                   master_d_param,
  input [top_pkg::TL_SZW-1:0]   master_d_size,
  input [top_pkg::TL_AIW-1:0]   master_d_source,
  input [top_pkg::TL_DIW-1:0]   master_d_sink,
  input                         master_d_denied,
  input [top_pkg::TL_DW-1:0]    master_d_data,
  input                         master_d_corrupt,
  input                         master_d_valid,
  output                        master_d_ready

);

  // Create the structures for communicating with OpenTitan-based Tilelink
  tl_h2d_t                      slave_tl_h2d;
  tl_d2h_t                      slave_tl_d2h;
  tl_h2d_t                      master_tl_h2d;
  tl_d2h_t                      master_tl_d2h;

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

  // Make Master A channel connections
  assign master_a_opcode        = ( master_tl_h2d.a_opcode == PutFullData)    ? 3'h0 :
                                  ((master_tl_h2d.a_opcode == PutPartialData) ? 3'h1 :
                                  ((master_tl_h2d.a_opcode == Get)            ? 3'h4 :
                                    3'h4));
  assign master_a_param         = master_tl_h2d.a_param;
  assign master_a_size          = master_tl_h2d.a_size;
  assign master_a_source        = master_tl_h2d.a_source;
  assign master_a_address       = master_tl_h2d.a_address;
  assign master_a_mask          = master_tl_h2d.a_mask;
  assign master_a_data          = master_tl_h2d.a_data;
  assign master_a_corrupt       = 0;
  assign master_a_valid         = master_tl_h2d.a_valid;
  assign master_d_ready         = master_tl_h2d.d_ready;

  // Make Master D channel connections
  assign master_tl_d2h.d_opcode = ( master_d_opcode == 3'h0) ? AccessAck : 
                                  ((master_d_opcode == 3'h1) ? AccessAckData : 
                                    AccessAck); 
  assign master_tl_d2h.d_param  = master_d_param;
  assign master_tl_d2h.d_size   = master_d_size;
  assign master_tl_d2h.d_source = master_d_source;
  assign master_tl_d2h.d_sink   = master_d_sink;
  assign master_tl_d2h.d_data   = master_d_data;
  assign master_tl_d2h.d_user   = tl_a_user_t'('0);
  assign master_tl_d2h.d_error  = master_d_corrupt || master_d_denied;
  assign master_tl_d2h.d_valid  = master_d_valid;
  assign master_tl_d2h.a_ready  = master_a_ready;

  // Define some of the wires and registers associated with the tlul_adapter_reg
  wire                          reg_we_o;
  wire                          reg_re_o;
  wire [top_pkg::TL_AW-1:0]     reg_addr_o;
  wire [top_pkg::TL_DW-1:0]     reg_wdata_o;
  reg [top_pkg::TL_DW-1:0]      reg_rdata_i;
  reg                           reg_error_i;

  // Define some of the wires and registers associated with the tlul_adapter_host
  reg                           host_req_i;
  wire                          host_gnt_o;
  reg                           host_we_i;
  reg [top_pkg::TL_AW-1:0]      host_addr_i;
  reg [top_pkg::TL_DW-1:0]      host_wdata_i;
  wire                          host_valid_o;
  wire [top_pkg::TL_DW-1:0]     host_rdata_o;
  wire                          host_err_o;

  // Misc. signals
  reg [top_pkg::TL_DW-1:0]      srot_ctrlstatus_register; // Bit definition can be found in llki_pkg.sv
  reg [top_pkg::TL_DW-1:0]      srot_scratchpad0_register;
  reg [top_pkg::TL_DW-1:0]      srot_scratchpad1_register;
  reg                           write_error;
  reg                           read_error;
 


  //------------------------------------------------------------------------
  // Instantitate a tlul_adapter_reg to adapt the TL Slave Interface
  //------------------------------------------------------------------------
  tlul_adapter_reg #(
    .RegAw      (top_pkg::TL_AW   ),
    .RegDw      (top_pkg::TL_DW   )
  ) u_tlul_adapter_reg_inst (
    .clk_i      (clk              ),
    .rst_ni     (~rst             ),

    .tl_i       (slave_tl_h2d     ),
    .tl_o       (slave_tl_d2h     ),

    .we_o       (reg_we_o         ),
    .re_o       (reg_re_o         ),
    .addr_o     (reg_addr_o       ),
    .wdata_o    (reg_wdata_o      ),
    .be_o       (                 ),  // Accesses are assumed to be word-wide
    .rdata_i    (reg_rdata_i      ),
    .error_i    (reg_error_i      )
  );

  // The reg_error_i will be asserted if either a read or write error occurs
  assign reg_error_i = read_error || write_error;
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Instantiate a tlul_adapter_host fo connecting to the master interface
  //
  // Per the notes in tlul_adapter_host.sv, setting MAX_REQS = 1 results
  // in a purely combinatorial component.
  //------------------------------------------------------------------------
  tlul_adapter_host #(
    .MAX_REQS(1)
  ) u_tlul_adapter_host_inst (
    .clk_i    (clk                ),
    .rst_ni   (~rst               ),
    .req_i    (host_req_i         ),
    .gnt_o    (host_gnt_o         ),
    .addr_i   (host_addr_i        ),
    .we_i     (host_we_i          ),
    .wdata_i  (host_wdata_i       ),
    .be_i     ('1                 ), // All bytes always enabled
    .valid_o  (host_valid_o       ),
    .rdata_o  (host_rdata_o       ),
    .err_o    (host_err_o         ),
    .tl_o     (master_tl_h2d      ),
    .tl_i     (master_tl_d2h      )
  );
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Instantiate the LLKI C2 Send Message FIFO
  //------------------------------------------------------------------------
  // Define signals associated with the LLKI C2 Send Message FIFO
  reg                               llkic2_reqfifo_clr_i;
  reg                               llkic2_reqfifo_wvalid_i;
  wire                              llkic2_reqfifo_wready_o;
  reg [top_pkg::TL_DW - 1:0]        llkic2_reqfifo_wdata_i;
  wire                              llkic2_reqfifo_rvalid_o;
  reg                               llkic2_reqfifo_rready_i;
  wire [top_pkg::TL_DW - 1:0]       llkic2_reqfifo_rdata_o;
  wire [DepthW - 1 :0]              llkic2_reqfifo_depth_o;
  wire                              llkic2_reqfifo_empty;
  wire                              llkic2_reqfifo_full;

  // Instantiate the FIFO
  prim_fifo_sync #(
    .Width              (top_pkg::TL_DW),
    .Pass               (1'b0),
    .Depth              (FIFO_DEPTH),
    .OutputZeroIfEmpty  (1'b0)    // Enables "first word fall through"
  ) llkic2_reqfifo_inst (
    .clk_i              (clk),
    .rst_ni             (~rst),

    // Synchronous clear/flush
    .clr_i              (llkic2_reqfifo_clr_i),

    // Write Port
    .wvalid_i           (llkic2_reqfifo_wvalid_i),
    .wready_o           (llkic2_reqfifo_wready_o),
    .wdata_i            (llkic2_reqfifo_wdata_i),
  
    // Read Port
    .rvalid_o           (llkic2_reqfifo_rvalid_o),
    .rready_i           (llkic2_reqfifo_rready_i),
    .rdata_o            (llkic2_reqfifo_rdata_o),

    // Occupancy
    .depth_o            (llkic2_reqfifo_depth_o)
  );

  // Generate the full and empty signals for the LLKIC2 Send FIFO
  assign llkic2_reqfifo_empty = llkic2_reqfifo_depth_o == '0;
  assign llkic2_reqfifo_full  = llkic2_reqfifo_depth_o == FIFO_DEPTH;
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Instantiate the LLKI C2 Receive Message FIFO
  //------------------------------------------------------------------------
  // Define signals associated with the LLKI C2 Received Message FIFO
  reg                               llkic2_respfifo_clr_i;
  reg                               llkic2_respfifo_wvalid_i;
  wire                              llkic2_respfifo_wready_o;
  reg [top_pkg::TL_DW - 1:0]        llkic2_respfifo_wdata_i;
  wire                              llkic2_respfifo_rvalid_o;
  reg                               llkic2_respfifo_rready_i;
  reg [top_pkg::TL_DW - 1:0]        llkic2_respfifo_rdata_o;
  wire [DepthW - 1:0]               llkic2_respfifo_depth_o;
  wire                              llkic2_respfifo_empty;  
  wire                              llkic2_respfifo_full;  

  // Instantiate the FIFO
  prim_fifo_sync #(
    .Width              (top_pkg::TL_DW),
    .Pass               (1'b0),
    .Depth              (FIFO_DEPTH),
    .OutputZeroIfEmpty  (1'b0)    // Enables "first word fall through"
  ) llkic2_respfifo_inst (
    .clk_i              (clk),
    .rst_ni             (~rst),

    // Synchronous clear/flush
    .clr_i              (llkic2_respfifo_clr_i),

    // Write Port
    .wvalid_i           (llkic2_respfifo_wvalid_i),
    .wready_o           (llkic2_respfifo_wready_o),
    .wdata_i            (llkic2_respfifo_wdata_i),
  
    // Read Port
    .rvalid_o           (llkic2_respfifo_rvalid_o),
    .rready_i           (llkic2_respfifo_rready_i),
    .rdata_o            (llkic2_respfifo_rdata_o),

    // Occupancy
    .depth_o            (llkic2_respfifo_depth_o)
  );

  // Generate the full and empty signals for the Rx FIFO
  assign llkic2_respfifo_empty  = llkic2_respfifo_depth_o == '0;
  assign llkic2_respfifo_full   = llkic2_respfifo_depth_o == FIFO_DEPTH;
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Key Index RAM
  //------------------------------------------------------------------------
  reg                       keyindexram_a_write_i;
  reg [top_pkg::TL_AW-1:0]  keyindexram_a_addr_i;
  reg [top_pkg::TL_DW-1:0]  keyindexram_a_wdata_i;
  wire [top_pkg::TL_DW-1:0] keyindexram_a_rdata_o;

  reg [top_pkg::TL_AW-1:0]  keyindexram_b_addr_i;
  wire [top_pkg::TL_DW-1:0] keyindexram_b_rdata_o;

  prim_generic_ram_2p #(
    .Width              (top_pkg::TL_DW),
    .Depth              (SROT_KEYINDEXRAM_SIZE)
  ) key_index_ram_inst (
    .clk_a_i            (clk),
    .clk_b_i            (clk),

    .a_req_i            (1'b1),                     // Always selected
    .a_write_i          (keyindexram_a_write_i),
    .a_addr_i           (keyindexram_a_addr_i[$clog2(SROT_KEYINDEXRAM_SIZE) - 1:0]),
    .a_wdata_i          (keyindexram_a_wdata_i),
    .a_wmask_i          ('1),                       // Mask is unused
    .a_rdata_o          (keyindexram_a_rdata_o),      

    .b_req_i            (1'b1),                     // Always selected
    .b_write_i          (1'b0),                     // Port B is Read Only
    .b_addr_i           (keyindexram_b_addr_i[$clog2(SROT_KEYINDEXRAM_SIZE) - 1:0]),
    .b_wdata_i          ('0),                       // Port B is Read Only
    .b_wmask_i          ('1),                       // Mask is unused
    .b_rdata_o          (keyindexram_b_rdata_o)

  );
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Key RAM
  //------------------------------------------------------------------------
  reg                       keyram_a_write_i;
  reg [top_pkg::TL_AW-1:0]  keyram_a_addr_i;
  reg [top_pkg::TL_DW-1:0]  keyram_a_wdata_i;
  wire [top_pkg::TL_DW-1:0] keyram_a_rdata_o;

  reg [top_pkg::TL_AW-1:0]  keyram_b_addr_i;
  wire [top_pkg::TL_DW-1:0] keyram_b_rdata_o;

  prim_generic_ram_2p #(
    .Width              (top_pkg::TL_DW),
    .Depth              (SROT_KEYRAM_SIZE)
  ) key_ram_inst (
    .clk_a_i            (clk),
    .clk_b_i            (clk),

    .a_req_i            (1'b1),                 // Always selected
    .a_write_i          (keyram_a_write_i),
    .a_addr_i           (keyram_a_addr_i[$clog2(SROT_KEYRAM_SIZE) - 1:0]),
    .a_wdata_i          (keyram_a_wdata_i),
    .a_wmask_i          ('1),                   // Mask is unused
    .a_rdata_o          (keyram_a_rdata_o),      

    .b_req_i            (1'b1),                 // Always selected
    .b_write_i          (1'b0),                 // Port B is Read Only
    .b_addr_i           (keyram_b_addr_i[$clog2(SROT_KEYRAM_SIZE) - 1:0]),
    .b_wdata_i          ('0),                   // Port B is Read Only
    .b_wmask_i          ('1),                   // Mask is unused
    .b_rdata_o          (keyram_b_rdata_o)

  );
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Write Decode Process
  //------------------------------------------------------------------------
  always @(posedge clk or posedge rst)
  begin
    if (rst) begin
      srot_ctrlstatus_register    <= '0;
      srot_scratchpad0_register   <= '0;
      srot_scratchpad1_register   <= '0;
      write_error                 <= 1'b0;
      keyindexram_a_write_i       <= 1'b0;
      keyram_a_write_i            <= 1'b0;
      keyindexram_a_addr_i        <= '0;
      keyindexram_a_wdata_i       <= '0;
      keyram_a_addr_i             <= '0;
      keyram_a_wdata_i            <= '0;
      llkic2_reqfifo_wvalid_i     <= 1'b0;
      llkic2_reqfifo_wdata_i      <= 1'b0;
    end else begin
      
      // Default signal assignments
      write_error                 <= 1'b0;
      keyindexram_a_write_i       <= 1'b0;
      keyram_a_write_i            <= 1'b0;
      llkic2_reqfifo_wvalid_i     <= 1'b0;
      llkic2_reqfifo_wdata_i      <= 1'b0;
 
      // The Key Index and Key RAM A ports are registered to
      // align them with the corresponding write enables
      keyindexram_a_addr_i        <= (reg_addr_o - SROT_KEYINDEXRAM_ADDR) >> 3;
      keyindexram_a_wdata_i       <= reg_wdata_o;
      keyram_a_addr_i             <= (reg_addr_o - SROT_KEYRAM_ADDR) >> 3;
      keyram_a_wdata_i            <= reg_wdata_o;

      // Implement other bits of the Control / Status Register
      srot_ctrlstatus_register[SROT_CTRLSTS_RESP_WAITING]   <= ~llkic2_respfifo_empty;

      // A write has been requested
      if (reg_we_o) begin
        
        // Read or Writes to either RAMs while in operational mode should cause an error of some
        // sort, but we do not currently have a means of handling tilelink errors.  Thus, the
        // acccess will just be ignored

        // Decode to the Key RAM
        // Note: Addresses are in terms of bytes and the datapath is 64-bit wide
        if (reg_addr_o >= SROT_KEYRAM_ADDR && reg_addr_o <= (SROT_KEYRAM_ADDR + (SROT_KEYRAM_SIZE * 8) - 1)) begin
          
          // Both mode bits MUST BE ZERO to allow access to the key and key index RAMs
          if (srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_0] || srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_1])
            ;
          else
            keyram_a_write_i                <= 1'b1;

        // Decode to the Key Index RAM
        end else if (reg_addr_o >= SROT_KEYINDEXRAM_ADDR && reg_addr_o <= (SROT_KEYINDEXRAM_ADDR + (SROT_KEYINDEXRAM_SIZE * 8) - 1)) begin

          // Both mode bits MUST BE ZERO to allow access to the key and key index RAMs
          if (srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_0] || srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_1])
            ;
          else
            keyindexram_a_write_i           <= 1'b1;

        // All other write decode events
        end else begin
          case (reg_addr_o)
            //
            // SROT Control Status Register
            //
            SROT_CTRLSTS_ADDR         : begin
              if (reg_wdata_o[SROT_CTRLSTS_MODEBIT_0])  srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_0] <= 1'b1;
              if (reg_wdata_o[SROT_CTRLSTS_MODEBIT_1])  srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_1] <= 1'b1;
            end // end SROT_CTRLSTS_ADDR

            // Write to the LLKIC2 Send FIFO (writing to a full FIFO will cause an error)
            SROT_LLKIC2_SENDRECV_ADDR : begin
              // Overflow condition
              if (llkic2_reqfifo_full) begin
                write_error               <= 1'b1;
              end else begin
                llkic2_reqfifo_wdata_i   <= reg_wdata_o;
                llkic2_reqfifo_wvalid_i  <= 1'b1;
              end // end else llkic2_reqfifo_full
            end // end SROT_LLKIC2_SEND_ADDR

            // Write to the Scratchpad 0 Register
            SROT_LLKIC2_SCRATCHPAD0_ADDR : begin
              srot_scratchpad0_register   <= reg_wdata_o;
            end

            // Write to the Scratchpad 1 Register
            SROT_LLKIC2_SCRATCHPAD1_ADDR : begin
              srot_scratchpad1_register   <= reg_wdata_o;
            end

            //    
            // All other decodes
            //            
            default             :
              write_error     <= 1'b1;
          endcase   // endcase reg_addr_o
        end   // end if aaddress decode
      end else begin

          // No writes, no errors.
          write_error     <= 1'b0;

      end // end if reg_we_o
    end // end else if (rst)
  end // end always
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // Read decode process
  //------------------------------------------------------------------------
  always @*
  begin

    // Default signal assignments
    reg_rdata_i                   = '0;
    read_error                    = 1'b0;
    llkic2_respfifo_clr_i         = 1'b0;
    llkic2_respfifo_rready_i      = 1'b0;

    // A read has been requested
    if (reg_re_o) begin

      // Read or Writes to either RAMs while in operational mode should cause an error of some
      // sort, but we do not currently have a means of handling tilelink errors.  Thus, the
      // acccess will just be ignored

      // Decode to the Key RAM
      // Note: Addresses are in terms of bytes and the datapath is 64-bit wide
      if (reg_addr_o >= SROT_KEYRAM_ADDR && reg_addr_o <= (SROT_KEYRAM_ADDR + (SROT_KEYRAM_SIZE * 8) - 1)) begin
          
        // Both mode bits MUST BE ZERO to allow access to the key and key index RAMs
        if (srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_0] || srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_1])
          ;
        else
          reg_rdata_i                   = keyram_a_rdata_o;

      // Decode to the Key Index RAM
      end else if (reg_addr_o >= SROT_KEYINDEXRAM_ADDR && reg_addr_o <= (SROT_KEYINDEXRAM_ADDR + (SROT_KEYINDEXRAM_SIZE * 8) - 1)) begin

        // Both mode bits MUST BE ZERO to allow access to the key and key index RAMs
        if (srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_0] || srot_ctrlstatus_register[SROT_CTRLSTS_MODEBIT_1])
          ;
        else
          reg_rdata_i                   = keyindexram_a_rdata_o;

      // All other write decode events
      end else begin
        case (reg_addr_o)
          //
          // SROT Control Status Register
          //
          SROT_CTRLSTS_ADDR           : begin
            reg_rdata_i                 = srot_ctrlstatus_register; 
          end // end SROT_CTRLSTS_ADDR
          //
          // Decode to the Receive FIFO
          //            
          SROT_LLKIC2_SENDRECV_ADDR   : begin
            if (llkic2_respfifo_empty) begin
              read_error                = 1'b1;
            end else begin
              llkic2_respfifo_rready_i  = 1'b1;
              reg_rdata_i               = llkic2_respfifo_rdata_o;  
            end // end llkic2_respfifo_empty
          end // SROT_LLKIC2_RECV_ADDR
          //
          // Scratchpad 0 Register
          //
          SROT_LLKIC2_SCRATCHPAD0_ADDR : begin
            reg_rdata_i                 = srot_scratchpad0_register;
          end

          //
          // Scratchpad 1 Register
          //
          SROT_LLKIC2_SCRATCHPAD1_ADDR : begin
            reg_rdata_i                 = srot_scratchpad1_register;
          end

          //
          // All other decodes
          //            
          default             : begin
            read_error                  <= 1'b1;
          end // end default
        endcase   // endcase reg_addr_o
      end // end if address decode
    end // end if (reg_re_o)
  end // end always @(reg_addr_o or reg_re_o)
  //------------------------------------------------------------------------



  //------------------------------------------------------------------------
  // SRoT State Machine
  // Processes messages received via the SEND message FIFO, initiates
  // transactions with downstream LLKI Protocol Processing blocks via the
  // TL host interface, receives downstream responses, then generates the
  // responses back to the RISC-V.  Once the RISC-V initiates an LLKIC2
  // request, it should poll the SROT_CTRLSTS_RESP_WAITING bit in the
  // SROT_CTRLSTS_ADDR register to determine when a response has been
  // received.
  //------------------------------------------------------------------------
  SROT_STATE_TYPE       srot_current_state;
  reg [31:0]            rsvd;
  reg [7:0]             msg_id;
  reg [7:0]             status;
  reg [7:0]             msg_len;
  reg [7:0]             key_index;
  reg [15:0]            low_pointer;
  reg [15:0]            high_pointer;
  reg [15:0]            current_pointer;  // Used to track the currently selected
                                          // Key RAM word
  reg [7:0]             core_index;
  reg                   index_valid;
  reg [7:0]             wait_state_counter;

  // Perform a continuous assignment of the key index RAM fields,
  // making conding of the SRoT STM a bit cleaner
  always @*
  begin
    low_pointer         = keyindexram_b_rdata_o[15:0];
    high_pointer        = keyindexram_b_rdata_o[31:16];
    core_index          = keyindexram_b_rdata_o[39:32];
    index_valid         = keyindexram_b_rdata_o[63];
  end   // end always @*


  always @(posedge clk or posedge rst)
  begin
    if (rst) begin
      host_req_i                <= '0;
      host_addr_i               <= '0;
      host_we_i                 <= '0;
      host_wdata_i              <= '0;
      keyindexram_b_addr_i      <= '0;
      keyram_b_addr_i           <= '0;
      llkic2_reqfifo_clr_i     <= 1'b0;
      llkic2_reqfifo_rready_i  <= 1'b0;
      llkic2_respfifo_wvalid_i  <= 1'b0;
      llkic2_respfifo_wdata_i   <= '0;
      rsvd                      <= '0;
      msg_id                    <= '0;
      status                    <= '0;
      msg_len                   <= '0;
      key_index                 <= '0;
      current_pointer           <= '0;
      wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
      srot_current_state        <= ST_SROT_IDLE;
    end else begin
  
      // Case for the SRoT STM
      case (srot_current_state)
        //------------------------------------------------------------------
        // IDLE State
        //------------------------------------------------------------------
        ST_SROT_IDLE          : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyindexram_b_addr_i      <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= 1'b0;
          llkic2_respfifo_wvalid_i  <= 1'b0;
          llkic2_respfifo_wdata_i   <= '0;
          rsvd                      <= '0;
          msg_id                    <= '0;
          status                    <= '0;
          msg_len                   <= '0;
          key_index                 <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_IDLE;

          // The SRoT STM will stay idle until it determines that a message is
          // present in the Send FIFO.  All messages from RISC-V are expect to
          // be one 64-bit word in length.
          if (~llkic2_reqfifo_empty && llkic2_reqfifo_rvalid_o) begin

            // Capture the message elements (incoming status is ignored)
            msg_id                    <= llkic2_reqfifo_rdata_o[7:0];
            msg_len                   <= llkic2_reqfifo_rdata_o[23:16];
            key_index                 <= llkic2_reqfifo_rdata_o[31:24];
            rsvd                      <= llkic2_reqfifo_rdata_o[63:32];

            // Assert the Send FIFO read enable
            llkic2_reqfifo_rready_i  <= 1'b1;

            // Jump to the next state
            srot_current_state        <= ST_SROT_MESSAGE_CHECK;              
          end // end if (~llkic2_reqfifo_empty)

        end // end ST_SROT_IDLE
        //------------------------------------------------------------------
        // Message Check State
        //------------------------------------------------------------------
        ST_SROT_MESSAGE_CHECK   : begin
          // Default signal assignment
          host_req_i                <= '0;  
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyindexram_b_addr_i      <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_IDLE;

          case (msg_id)
            // A valid request haas been received via the C2 RISCV -> SRoT Inteface
            LLKI_MID_C2LOADKEYREQ,
            LLKI_MID_C2CLEARKEYREQ,
            LLKI_MID_C2KEYSTATUSREQ : begin
              keyindexram_b_addr_i    <= key_index;
              srot_current_state      <= ST_SROT_RETRIEVE_KEY_INDEX;
              ;
            end
            // All other message ID (error condition)
            // Clear the send FIFO as a precaution
            default                 : begin
              llkic2_reqfifo_clr_i    <= 1'b1;
              status                  <= LLKI_STATUS_BAD_MSG_ID;
              srot_current_state      <= ST_SROT_C2_RESPONSE;
            end 
          endcase   // end case (msg_id)

          // The only valid message length for requests is one
          if (msg_len != 1) begin
            llkic2_reqfifo_clr_i      <= 1'b1;
            status                    <= LLKI_STATUS_BAD_MSG_LEN;
            srot_current_state        <= ST_SROT_C2_RESPONSE;
          end // end if (msg_len != 1)

          // The Key Index must >= 0 and < SROT_KEYINDEXRAM_SIZE
          if (key_index >= SROT_KEYINDEXRAM_SIZE) begin
            llkic2_reqfifo_clr_i     <= 1'b1;
            status                    <= LLKI_STATUS_KEY_INDX_EXCEED;
            srot_current_state        <= ST_SROT_C2_RESPONSE;
          end // end if (key_index >= SROT_KEYINDEXRAM_SIZE)

        end // end ST_SROT_MESSAGE_CHECK
        //------------------------------------------------------------------
        // Retrieve (and check) key index State
        //------------------------------------------------------------------
        ST_SROT_RETRIEVE_KEY_INDEX    : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_RETRIEVE_KEY_INDEX;

          // Perform some error checking on the Key Index 
          // currently being referenced.  Error checking
          // is somewhat "hierchical in nature"
          // 
          // Key Index is NOT valid
          if (!index_valid) begin
            llkic2_reqfifo_clr_i    <= 1'b1;
            status                  <= LLKI_STATUS_KEY_INDEX_INVALID;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // Pointer related checks
          end else if (low_pointer   >= SROT_KEYRAM_SIZE  ||  // Low pointer exceeds the key RAM
                       high_pointer  >= SROT_KEYRAM_SIZE  ||  // High pointer exceeds the key RAM
                        low_pointer  > high_pointer) begin    // Low pointer > high pointer
            llkic2_reqfifo_clr_i    <= 1'b1;
            status                  <= LLKI_STATUS_BAD_POINTER_PAIR;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // The specified target core index exceeds the maximum entry
          // in the LLKI Core Index Array
          end else if (core_index > $high(LLKI_CORE_INDEX_ARRAY)) begin
            llkic2_reqfifo_clr_i    <= 1'b1;
            status                  <= LLKI_STATUS_BAD_CORE_INDEX;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // The received message has been checked for errors and the selected key
          // index looks ok.  
          end else begin
            // Jump to the next state
            srot_current_state      <= ST_SROT_KL_REQ_HEADER;
          end   // end if (!index_valid)

        end   // ST_RETRIEVE_KEY_INDEX
        //------------------------------------------------------------------
        // Create Header for the request to the specified LLKI-PP
        //------------------------------------------------------------------
        ST_SROT_KL_REQ_HEADER                   : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_REQ_ISSUE;

          // Save the low pointer, as it will used to determine how many key
          // words we will be sending as well as indexing the Key RAM
          current_pointer           <= low_pointer;

          // Header generation is message specific
          case (msg_id)
            LLKI_MID_C2LOADKEYREQ   : begin
              host_wdata_i[7:0]       <= LLKI_MID_KLLOADKEYREQ;
              // For the Load Key Request Message length is equal
              // to the 2 + (high_pointer - low pointer).
              host_wdata_i[23:16]     <= (high_pointer - low_pointer) + 2;
              host_addr_i             <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
              // Assert write enable and req (indicates a TL PutFullData operation)
              host_req_i              <= 1'b1;
              host_we_i               <= 1'b1;

            end
            LLKI_MID_C2CLEARKEYREQ  : begin
              host_wdata_i[7:0]       <= LLKI_MID_KLCLEARKEYREQ;
              host_wdata_i[23:16]     <= 8'h01;
              host_addr_i             <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
              // Assert write enable and req (indicates a TL PutFullData operation)
              host_req_i              <= 1'b1;
              host_we_i               <= 1'b1;
            end
            LLKI_MID_C2KEYSTATUSREQ : begin
              host_wdata_i[7:0]       <= LLKI_MID_KLKEYSTATUSREQ;
              host_wdata_i[23:16]     <= 8'h01;
              host_addr_i             <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
              // Assert write enable and req (indicates a TL PutFullData operation)
              host_req_i              <= 1'b1;
              host_we_i               <= 1'b1;
            end
            // Since we already checked for valid message IDs,
            // this truely is a trap condition
            default                 : begin
              llkic2_reqfifo_clr_i    <= 1'b1;
              status                  <= LLKI_STATUS_BAD_CORE_INDEX;
              srot_current_state      <= ST_SROT_C2_RESPONSE;
            end
          endcase

        end   // ST_SROT_KL_CREATE_HEADER
        //------------------------------------------------------------------
        // Assert host request in order to send the LLKI-PP Request
        //------------------------------------------------------------------
        ST_SROT_KL_REQ_ISSUE                : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_we_i                 <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_REQ_ISSUE;

          // Continue to assert write enable and req (indicates a TL PutFullData operation)
          host_req_i                <= 1'b1;
          host_we_i                 <= 1'b1;

          // Wait for grant indicating the write has been sent
          if (host_gnt_o) begin
            host_req_i              <= 1'b0;
            host_we_i               <= 1'b0;
            
            // Jump to the Request Wait for Ack state
            srot_current_state      <= ST_SROT_KL_REQ_WAIT_FOR_ACK;

            // Did an error get asserted?
            if (host_err_o) begin
              msg_id                <= LLKI_MID_C2ERRORRESP;        
              status                <= LLKI_STATUS_KL_TILELINK_ERROR;
              msg_len               <= 8'h01;
              srot_current_state    <= ST_SROT_C2_RESPONSE;
            end
          end // end if (host_gnt_o)
        end // ST_SROT_KL_REQ_WAIT_FOR_GRANT
        //------------------------------------------------------------------
        // Using the OpenTitan tlul_adapter_host module, even writes get
        // "acknowledged" through the assertion of the valid_o bit, which
        // is directly mapped from the TL-UL D channel valid bit
        //------------------------------------------------------------------
        ST_SROT_KL_REQ_WAIT_FOR_ACK         : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_we_i                 <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_REQ_WAIT_FOR_ACK;

          // The request has been acknowledged
          if (host_valid_o) begin

            // If this is a Load Key request, then we need to begin
            // the cycle of reading the ready bit from the selected
            // LLKI-PP block.  When ready, we can issue a key word
            // write.  Once all the key words have be written, then
            // we should look for a standard response from the LLKI-PP
            if (msg_id == LLKI_MID_C2LOADKEYREQ)
              srot_current_state    <= ST_SROT_KL_READ_READY_STATUS;
            else
              srot_current_state    <= ST_SROT_KL_READ_RESP_STATUS;

          end   // end if (host_valid_o)
        end // ST_SROT_KL_REQ_WAIT_FOR_ACK
        //------------------------------------------------------------------
        // Before writing a key word to the select LLKI-PP block, we need
        // read the ready bit
        //------------------------------------------------------------------
        ST_SROT_KL_READ_READY_STATUS        : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_READ_READY_STATUS;

          // We want to read the selected LLKI-PP Control/Status Register
          host_addr_i               <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_CTRLSTS_INDEX];
          host_req_i                <= 1'b1;

          // Did an error get asserted?
          if (host_err_o) begin
            msg_id                  <= LLKI_MID_C2ERRORRESP;        
            status                  <= LLKI_STATUS_KL_TILELINK_ERROR;
            msg_len                 <= 8'h01;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // Wait until the request has been granted, then jump to
          // the check status state
          end else if (host_gnt_o) begin
            // Point the Key RAM to the current word
            keyram_b_addr_i         <= current_pointer;

            // Jump to next 
            srot_current_state      <= ST_SROT_KL_CHECK_READY_STATUS;
          end
        end // ST_SROT_KL_READ_READY_STATUS
        //------------------------------------------------------------------
        // The Ready Status has been read, now time to check it
        //------------------------------------------------------------------
        ST_SROT_KL_CHECK_READY_STATUS       : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_CHECK_READY_STATUS;

          // Valid data has been returned
          if (host_valid_o) begin

            // The select LLKI-PP is ready for a key
            if (host_rdata_o[LLKIKL_CTRLSTS_READY_FOR_KEY] == 1'b1) begin
              // Issue the key write, understanding that if host_gnt_o is
              // already asserted, this will only be set for a single clock cycle
              host_addr_i               <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
              host_wdata_i              <= keyram_b_rdata_o;
              host_req_i                <= 1'b1;
              host_we_i                 <= 1'b1;

              srot_current_state        <= ST_SROT_KL_LOAD_KEY_WORD;
            // Response is NOT waiting, jump to wait state
            end else begin
              srot_current_state        <= ST_SROT_KL_READY_WAIT_STATE;
            end
          end // end else if (host_err_o)
        end // ST_SROT_KL_CHECK_READY_STATUS
        //------------------------------------------------------------------
        // Wait state to avoid spamming the LLKI-PP with requests
        //------------------------------------------------------------------
        ST_SROT_KL_READY_WAIT_STATE         : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          srot_current_state        <= ST_SROT_KL_READY_WAIT_STATE;

          // Decrement the wait state counter
          wait_state_counter        <= wait_state_counter - 1;

          // Did an error get asserted?
          if (host_err_o) begin
            msg_id                  <= LLKI_MID_C2ERRORRESP;        
            status                  <= LLKI_STATUS_KL_TILELINK_ERROR;
            msg_len                 <= 8'h01;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          end else if (wait_state_counter == 0) begin
            srot_current_state      <= ST_SROT_KL_READ_READY_STATUS;
          end

        end // ST_SROT_KL_READY_WAIT_STATE
        //------------------------------------------------------------------
        // We know the LLKI-PP is ready for a key word, so let's send it
        //------------------------------------------------------------------
        ST_SROT_KL_LOAD_KEY_WORD          : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_LOAD_KEY_WORD;

          // Hold the signals, until host_gnt_o is issued
          host_addr_i               <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
          host_wdata_i              <= keyram_b_rdata_o;
          host_req_i                <= 1'b1;
          host_we_i                 <= 1'b1;

          // The write request has been granted
          if (host_gnt_o) begin
            // Clear up some signals
            host_addr_i             <= '0;            
            host_wdata_i            <= '0;
            host_req_i              <= '0;
            host_we_i               <= '0;

            // Jump to the next state
            srot_current_state      <= ST_SROT_KL_LOAD_KEY_WORD_WAIT_FOR_ACK;
          end
        end // ST_SROT_KL_LOAD_KEY_WORD
        //------------------------------------------------------------------
        // The key word write has been issued, we need to wait until it
        // is acknowledged
        //------------------------------------------------------------------
        ST_SROT_KL_LOAD_KEY_WORD_WAIT_FOR_ACK   : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_LOAD_KEY_WORD_WAIT_FOR_ACK;

          // The write has been acknowledged
          if (host_valid_o) begin

            // Is this the last word of the load?  If yes, then jump to the
            // read response status state
            if (current_pointer == high_pointer) begin
              srot_current_state    <= ST_SROT_KL_READ_RESP_STATUS;
            // We have more words.  Increment the current pointer and
            // read the LLKI-PP ready status
            end else begin
              current_pointer       <= current_pointer + 1;
              srot_current_state    <= ST_SROT_KL_READ_READY_STATUS;
            end // end if (current_pointer == high_pointer)

          end
        end // ST_SROT_KL_LOAD_KEY_WORD_WAIT_FOR_ACK
        //------------------------------------------------------------------
        // Now, the request has been sent, it is time to poll the response
        // waiting bit in the target LLKI-PP
        //------------------------------------------------------------------
        ST_SROT_KL_READ_RESP_STATUS         : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_READ_RESP_STATUS;

          // We want to read the selected LLKI-PP Control/Status Register
          host_addr_i               <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_CTRLSTS_INDEX];
          host_req_i                <= 1'b1;

          // Did an error get asserted?
          if (host_err_o) begin
            msg_id                  <= LLKI_MID_C2ERRORRESP;        
            status                  <= LLKI_STATUS_KL_TILELINK_ERROR;
            msg_len                 <= 8'h01;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // Wait until the request has been granted, then jump to
          // the check status state
          end else if (host_gnt_o) begin
            srot_current_state      <= ST_SROT_KL_CHECK_RESP_STATUS;
          end
        end // ST_SROT_KL_RESP_READ_STATUS
        //------------------------------------------------------------------
        // The read request has been issued and granted, but now we need
        // to wait until valid data is returned
        //------------------------------------------------------------------
        ST_SROT_KL_CHECK_RESP_STATUS       : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_CHECK_RESP_STATUS;

          // Valid data has been returned
          if (host_valid_o) begin

            // Response is waiting in the LLKI PP.  Initiate the request 
            // as we tranisition to the next state            
            if (host_rdata_o[LLKIKL_CTRLSTS_RESP_WAITING] == 1'b1) begin
              // We want to read the selected LLKI-PP Send/Receive Queue
              host_addr_i               <= LLKI_CORE_INDEX_ARRAY[core_index][LLKI_SENDRECV_INDEX];
              host_req_i                <= 1'b1;

              srot_current_state        <= ST_SROT_KL_RESP_READ;
            // Response is NOT waiting, jump to wait state
            end else begin
              srot_current_state        <= ST_SROT_KL_RESP_WAIT_STATE;
            end
          end // end else if (host_err_o)
        end // ST_SROT_KL_RESP_CHECK_STATUS
        //------------------------------------------------------------------
        // This wait state is used to avoid spaming the Host interface
        // while waiting for the select LLKI-PP block to process a message
        //------------------------------------------------------------------
        ST_SROT_KL_RESP_WAIT_STATE        : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          srot_current_state        <= ST_SROT_KL_RESP_WAIT_STATE;

          // Decrement the wait state counter
          wait_state_counter        <= wait_state_counter - 1;

          // Did an error get asserted?
          if (host_err_o) begin
            msg_id                  <= LLKI_MID_C2ERRORRESP;        
            status                  <= LLKI_STATUS_KL_TILELINK_ERROR;
            msg_len                 <= 8'h01;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          end else if (wait_state_counter == 0) begin
            srot_current_state      <= ST_SROT_KL_READ_RESP_STATUS;
          end

        end // ST_SROT_KL_RESP_WAIT_STATE
        //------------------------------------------------------------------
        // Read LLKI-PP Reponse
        //------------------------------------------------------------------
        ST_SROT_KL_RESP_READ        : begin
          // Default signal assignment
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= 1'b0;
          llkic2_respfifo_wvalid_i  <= 1'b0;
          llkic2_respfifo_wdata_i   <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_KL_RESP_READ;

          // Did an error get asserted?
          if (host_err_o) begin
            msg_id                  <= LLKI_MID_C2ERRORRESP;        
            status                  <= LLKI_STATUS_KL_TILELINK_ERROR;
            msg_len                 <= 8'h01;
            srot_current_state      <= ST_SROT_C2_RESPONSE;
          // Valid data has been received
          end else if (host_valid_o) begin

            // Deassert request and address signals
            host_req_i              <= '0;
            host_addr_i             <= '0;

            // Capture the reponse fields from the LLKI-PP block
            msg_id                  <= host_rdata_o[7:0];
            status                  <= host_rdata_o[15:8];
            msg_len                 <= host_rdata_o[23:16];

            // Just to creating a C2 Response message
            srot_current_state      <= ST_SROT_C2_RESPONSE;

          end // end else if (host_err_o)
        end // end ST_SROT_KL_RESP_READ
        //------------------------------------------------------------------
        // Generate the LLKI C2 Response Message
        //------------------------------------------------------------------
        ST_SROT_C2_RESPONSE         : begin
          // Default signal states
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyindexram_b_addr_i      <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_rready_i   <= 1'b0;
          llkic2_respfifo_wdata_i   <= '0;
          llkic2_respfifo_wvalid_i  <= 1'b0;
          key_index                 <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_IDLE;

          // Create the response message.  Errors can be generated
          // in the following ways:
          //  - LLKIC2 message is bad
          //  - The reference key index is bad
          //  - A response received via the LLKI-KL interface indicates an error
          //  - A tilelink error is indicated on the LLKI-KL interface
          case (status)
            //------------------------------------------------------------------
            // Processing a good response received via the LLKI-KL interface
            //------------------------------------------------------------------
            LLKI_STATUS_GOOD,
            LLKI_STATUS_KEY_PRESENT,
            LLKI_STATUS_KEY_NOT_PRESENT    : begin

              // Set most of the LLKI-C2 response fields
              llkic2_respfifo_wdata_i[15:8]   <= status;
              llkic2_respfifo_wdata_i[23:16]  <= 8'h01;
              llkic2_respfifo_wdata_i[63:32]  <= rsvd;
              llkic2_respfifo_wvalid_i        <= 1'b1;

              // Determine the LLKI C2 Response ID
              case (msg_id)
                LLKI_MID_KLLOADKEYACK       : llkic2_respfifo_wdata_i[7:0]  <= LLKI_MID_C2LOADKEYACK;
                LLKI_MID_KLCLEARKEYACK      : llkic2_respfifo_wdata_i[7:0]  <= LLKI_MID_C2CLEARKEYACK;
                LLKI_MID_KLKEYSTATUSRESP    : llkic2_respfifo_wdata_i[7:0]  <= LLKI_MID_C2KEYSTATUSRESP;
                default                     : begin
                  llkic2_respfifo_wdata_i[7:0]  <= LLKI_MID_KLERRORRESP;
                  llkic2_respfifo_wdata_i[15:8] <= LLKI_STATUS_KL_RESP_BAD_MSG_ID;
                end
              endcase
            end
            //------------------------------------------------------------------
            // An expected error has occurred
            //------------------------------------------------------------------
            LLKI_STATUS_BAD_MSG_ID,
            LLKI_STATUS_BAD_MSG_LEN,
            LLKI_STATUS_KEY_INDX_EXCEED,
            LLKI_STATUS_KEY_INDEX_INVALID,
            LLKI_STATUS_BAD_POINTER_PAIR,
            LLKI_STATUS_BAD_CORE_INDEX,
            LLKI_STATUS_KL_REQ_BAD_MSG_ID,
            LLKI_STATUS_KL_REQ_BAD_MSG_LEN,
            LLKI_STATUS_KL_RESP_BAD_MSG_ID,            
            LLKI_STATUS_KL_TILELINK_ERROR     : begin
              llkic2_respfifo_wdata_i[7:0]    <= LLKI_MID_C2ERRORRESP;
              llkic2_respfifo_wdata_i[15:8]   <= status;
              llkic2_respfifo_wdata_i[23:16]  <= 8'h01;
              llkic2_respfifo_wdata_i[63:32]  <= rsvd;
              llkic2_respfifo_wvalid_i        <= 1'b1;
            end
            //------------------------------------------------------------------
            // Trap state - Create an LLKI-C2 message with an unknown error
            //------------------------------------------------------------------
            default                           : begin
              llkic2_respfifo_wdata_i[7:0]    <= LLKI_MID_C2ERRORRESP;
              llkic2_respfifo_wdata_i[15:8]   <= LLKI_STATUS_UNKNOWN_ERROR;
              llkic2_respfifo_wdata_i[23:16]  <= 8'h01;
              llkic2_respfifo_wdata_i[63:32]  <= rsvd;
              llkic2_respfifo_wvalid_i        <= 1'b1;
            end
          endcase // case (status)

          // Return to IDLE
          srot_current_state        <= ST_SROT_IDLE;
        end   // ST_SROT_ERROR_RESPONSE
        //------------------------------------------------------------------
        // Trap State
        //------------------------------------------------------------------
        default                       : begin
          host_req_i                <= '0;
          host_addr_i               <= '0;
          host_we_i                 <= '0;
          host_wdata_i              <= '0;
          keyindexram_b_addr_i      <= '0;
          keyram_b_addr_i           <= '0;
          llkic2_reqfifo_clr_i      <= '0;
          llkic2_reqfifo_rready_i   <= '0;
          llkic2_respfifo_wvalid_i  <= '0;
          llkic2_respfifo_wdata_i   <= '0;
          rsvd                      <= '0;
          msg_id                    <= '0;
          status                    <= '0;
          msg_len                   <= '0;
          key_index                 <= '0;
          current_pointer           <= '0;
          wait_state_counter        <= SROT_WAIT_STATE_COUNTER_INIT;
          srot_current_state        <= ST_SROT_IDLE;
        end
        //------------------------------------------------------------------
      endcase   // endcase srot_current_state

    end // end if rst
  end // end always

endmodule   // endmodule srot_wrapper
