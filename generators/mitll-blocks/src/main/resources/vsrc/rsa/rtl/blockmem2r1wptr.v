//======================================================================
//
// blockmem2r1wptr.v
// -----------------
// Synchronous block memory with two read ports and one write port.
// For port 1 the address is implicit and instead given by the
// internal pointer. The pointer is automatically increased
// when the cs signal is set. The pointer is reset to zero when
// the rst signal is asserted.
//
//
// NOTE: This memory needs to be rebuilt if interface 0 is changed
// to use bigger operand widths and fewer words than interface 1.
// This adaption is NOT automatic.
//
//
// The memory is used in the modexp core.
//
//
// Author: Joachim Strombergson
// Copyright (c) 2015, NORDUnet A/S All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the name of the NORDUnet nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//======================================================================

module blockmem2r1wptr #(parameter OPW = 32, parameter ADW = 8)
       (
           input wire                  clk,
           input wire                  reset_n,

           input wire  [(ADW - 1) : 0] read_addr0,
           output wire [(OPW - 1) : 0] read_data0,

           output wire [31 : 0]        read_data1,
           input wire                  rst,
           input wire                  cs,
           input wire                  wr,
           input wire  [31 : 0]        write_data
       );


//----------------------------------------------------------------
// Memories and regs including update variables and write enable.
//----------------------------------------------------------------
reg [(OPW - 1) : 0] mem [0 : ((2**ADW) - 1)];
reg [(OPW - 1) : 0] tmp_read_data0;
reg [31 : 0]        tmp_read_data1;

reg [7 : 0]         ptr_reg;
reg [7 : 0]         ptr_new;
reg                 ptr_we;


//----------------------------------------------------------------
// Concurrent connectivity for ports etc.
//----------------------------------------------------------------
assign read_data0 = tmp_read_data0;
assign read_data1 = tmp_read_data1;


//----------------------------------------------------------------
// mem_update
//
// Clocked update of memory This should cause
// the memory to be implemented as a block memory.
//----------------------------------------------------------------
always @ (posedge clk)
    begin : mem_update
        if (wr)
            mem[ptr_reg] <= write_data;

        tmp_read_data0 <= mem[read_addr0];
        tmp_read_data1 <= mem[ptr_reg];
    end


//----------------------------------------------------------------
// ptr_update
//----------------------------------------------------------------
always @ (posedge clk or negedge reset_n)
    begin : ptr_update
        if (!reset_n)
            ptr_reg <= 8'h00;

        else
            if (ptr_we)
                ptr_reg <= ptr_new;
    end


//----------------------------------------------------------------
// ptr_logic
//----------------------------------------------------------------
always @*
    begin : ptr_logic
        ptr_new = 8'h00;
        ptr_we  = 1'b0;

        if (rst)
            begin
                ptr_new = 8'h00;
                ptr_we  = 1'b1;
            end

        if (cs)
            begin
                ptr_new = ptr_reg + 1'b1;
                ptr_we  = 1'b1;
            end
    end

endmodule // blockmem2r1wptr

          //======================================================================
          // EOF blockmem2r1wptr.v
          //======================================================================
