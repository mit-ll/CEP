//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : orpsoc_defines.v
// Project      : Common Evaluation Platform (CEP)
// Description  : Defines file for the CEP
// Notes        : Core licensing information may be found in licenseLog.txt
//

`ifdef SYNTHESIS
    `define RESET_HIGH
`endif

// Define the Address and Data widths
`define CEP_AXI_ADDR_WIDTH  32
`define CEP_AXI_DATA_WIDTH  32

// Define the numnber of AXI4-Lite slaves (cores) in the CEP
`define CEP_NUM_OF_SLAVES   12  // Maximum number, not to be changed
                                // when cores are enabled/disabled
`define CEP_NUM_OF_MASTERS  2   // Instruction and Data Bus

// Set the default address mask for the AXI4-Lite Arbiter
`define CEP_ADDRESS_MASK    32'hFF00_0000

// Constants used to increase readbility (will be trimmed to one bit for assignment
// to the routing rules
`define CEP_SLAVE_ENABLED   32'h0000_0001
`define CEP_SLAVE_DISABLED  32'h0000_0000

// The following parameter will be used to define the rptuing rules
// for the AXI4-Lite crossbar within the CEP.  A total of three fields
// are present:
//      Field 0 (enabled)   - If set, then the relevant core's slave interface is enabled.
//          Additionally, this parameter will be used to control the instantiation of
//          the core in the CEP.  If not set, the slave port will be disabled and the
//          signals will be "stubbed out"
//      Field 1 (mask)      - Control the address masking.  For the CEP, all the slaves
//          will use the same mask
//      Field 2 (address)   - Sets the base address of corresponding slave (core)
//
// Slave assignments are including as inline comments.
//
// Note: The RAM and UART slaves MUST be enabled for a CEP to be functional
parameter [31:0] cep_routing_rules [`CEP_NUM_OF_SLAVES - 1:0][0:2] = '{
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h0000_0000},   // Slave 0  - RAM (Mandatory)
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9000_0000},   // Slave 1  - UART (Mandatory)
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9300_0000},   // Slave 2  - AES
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9400_0000},   // Slave 3  - MD5
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9500_0000},   // Slave 4  - SHA256
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9600_0000},   // Slave 5  - RSA
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9700_0000},   // Slave 6  - DES3
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9800_0000},   // Slave 7  - DFT
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9900_0000},   // Slave 8  - IDFT
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9A00_0000},   // Slave 9  - FIR
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9B00_0000},   // Slave 10 - IIR
    {`CEP_SLAVE_ENABLED, `CEP_ADDRESS_MASK, 32'h9C00_0000}    // Slave 11 - GPS
};

// A few defines to increase readability.  These must match the order of slaves
// as defined in the above cep_routing_rules!
`define RAM_SLAVE_NUMBER    0
`define UART_SLAVE_NUMBER   1
`define AES_SLAVE_NUMBER    2
`define MD5_SLAVE_NUMBER    3
`define SHA256_SLAVE_NUMBER 4
`define RSA_SLAVE_NUMBER    5
`define DES3_SLAVE_NUMBER   6
`define DFT_SLAVE_NUMBER    7
`define IDFT_SLAVE_NUMBER   8
`define FIR_SLAVE_NUMBER    9
`define IIR_SLAVE_NUMBER    10
`define GPS_SLAVE_NUMBER    11

`define CEP_RAM_SIZE        32'h0004_0000   // 256K Bytes (64K 4-byte words)