`include "v2c_top.incl"
`timescale 1ns/1ns
module ptw_monitor
  (
   input 	clk,
   input trace_valid,
   input 	pc_valid,
   input [63:0] pc,
   // requestor 0=Dcache, 1=Icache
   input 	io_requestor_x_req_ready,
   input 	io_requestor_x_req_valid,
   input [26:0] io_requestor_x_req_bits_bits_addr,
   input 	io_requestor_x_resp_valid,
   input 	io_requestor_x_resp_bits_ae,
   input [53:0] io_requestor_x_resp_bits_pte_ppn,
   input 	io_requestor_x_resp_bits_pte_d,
   input 	io_requestor_x_resp_bits_pte_a,
   input 	io_requestor_x_resp_bits_pte_g,
   input 	io_requestor_x_resp_bits_pte_u,
   input 	io_requestor_x_resp_bits_pte_x,
   input 	io_requestor_x_resp_bits_pte_w,
   input 	io_requestor_x_resp_bits_pte_r,
   input 	io_requestor_x_resp_bits_pte_v
   );

   //
   always @(posedge clk) begin
      if (io_requestor_x_resp_valid) begin
	 if (!io_requestor_x_resp_bits_pte_v) begin
	    `logI("-->PTW miss: VPN=0x%08x",io_requestor_x_req_bits_bits_addr);
	 end
	 else begin
	    `logI("-->PTW hit: VPN=0x%08x PPN=0x%05x d/a/g/u/x/w/r=%b_%b_%b_%b_%b_%b_%b",
		  io_requestor_x_req_bits_bits_addr,
		  io_requestor_x_resp_bits_pte_ppn[19:0],
		  io_requestor_x_resp_bits_pte_d,
		  io_requestor_x_resp_bits_pte_a,
		  io_requestor_x_resp_bits_pte_g,
		  io_requestor_x_resp_bits_pte_u,
		  io_requestor_x_resp_bits_pte_x,
		  io_requestor_x_resp_bits_pte_w,
		  io_requestor_x_resp_bits_pte_r);
	 end
      end // if (io_requestor_x_resp_valid)
   end // always @ (posedge clk)

endmodule // ptw_monitor
