//======================================================================
//
// blockmem1rw1.v
// --------------
// Synchronous block memory with one read and one write port.
// The data size is the same for both read and write operations.
//
// The memory is used in the modexp core.
//
// paremeter OPW is operand word width in bits.
// parameter ADW is address width in bits.
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

module blockmem1r1w #(parameter OPW = 32, parameter ADW = 8)
       (
           input wire                  clk,

           input wire  [(ADW - 1) : 0] read_addr,
           output wire [(OPW - 1) : 0] read_data,

           input wire                  wr,
           input wire  [(ADW - 1) : 0] write_addr,
           input wire  [(OPW - 1) : 0] write_data
       );

reg [(OPW - 1) : 0] mem [0 : ((2**ADW) - 1)];
reg [(OPW - 1) : 0] tmp_read_data;

assign read_data = tmp_read_data;

always @ (posedge clk)
    begin : reg_mem
        if (wr)
            mem[write_addr] <= write_data;

        tmp_read_data <= mem[read_addr];
    end

endmodule // blockmem1r1w

          //======================================================================
          // EOF blockmem1r1w.v
          //======================================================================
