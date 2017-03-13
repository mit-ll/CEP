`include "orpsoc-defines.v"

// One master, 2 slaves.
module arbiter_ibus
  (
   // instruction bus in
   // Wishbone Master interface
   wbm_adr_o,
   wbm_dat_o,
   wbm_sel_o,
   wbm_we_o,
   wbm_cyc_o,
   wbm_stb_o,
   wbm_cti_o,
   wbm_bte_o,
  
   wbm_dat_i,
   wbm_ack_i,
   wbm_err_i,
   wbm_rty_i,


   // Slave one
   // Wishbone Slave interface
   wbs0_adr_i,
   wbs0_dat_i,
   wbs0_sel_i,
   wbs0_we_i,
   wbs0_cyc_i,
   wbs0_stb_i,
   wbs0_cti_i,
   wbs0_bte_i,
  
   wbs0_dat_o,
   wbs0_ack_o,
   wbs0_err_o,
   wbs0_rty_o,

   // Slave two
   // Wishbone Slave interface
   wbs1_adr_i,
   wbs1_dat_i,
   wbs1_sel_i,
   wbs1_we_i,
   wbs1_cyc_i,
   wbs1_stb_i,
   wbs1_cti_i,
   wbs1_bte_i,
  
   wbs1_dat_o,
   wbs1_ack_o,
   wbs1_err_o,
   wbs1_rty_o,

   wb_clk,
   wb_rst
   );


   parameter wb_dat_width = 32;
   parameter wb_adr_width = 32;

   parameter slave0_addr_width = 12;
   parameter slave1_addr_width = 28;
   
   input wb_clk;
   input wb_rst;

   
   // WB Master
   input [wb_adr_width-1:0] wbm_adr_o;
   input [wb_dat_width-1:0] wbm_dat_o;
   input [3:0] 		    wbm_sel_o;   
   input 		    wbm_we_o;
   input 		    wbm_cyc_o;
   input 		    wbm_stb_o;
   input [2:0] 		    wbm_cti_o;
   input [1:0] 		    wbm_bte_o;
   output [wb_dat_width-1:0] wbm_dat_i;   
   output 		     wbm_ack_i;
   output 		     wbm_err_i;
   output 		     wbm_rty_i;   

   // WB Slave 0
   output [wb_adr_width-1:0] wbs0_adr_i;
   output [wb_dat_width-1:0] wbs0_dat_i;
   output [3:0] 	     wbs0_sel_i;
   output 		     wbs0_we_i;
   output 		     wbs0_cyc_i;
   output 		     wbs0_stb_i;
   output [2:0] 	     wbs0_cti_i;
   output [1:0] 	     wbs0_bte_i;
   input [wb_dat_width-1:0]  wbs0_dat_o;   
   input 		     wbs0_ack_o;
   input 		     wbs0_err_o;
   input 		     wbs0_rty_o;   

   // WB Slave 1
   output [wb_adr_width-1:0] wbs1_adr_i;
   output [wb_dat_width-1:0] wbs1_dat_i;
   output [3:0] 	     wbs1_sel_i;
   output 		     wbs1_we_i;
   output 		     wbs1_cyc_i;
   output 		     wbs1_stb_i;
   output [2:0] 	     wbs1_cti_i;
   output [1:0] 	     wbs1_bte_i;
   input [wb_dat_width-1:0]  wbs1_dat_o;   
   input 		     wbs1_ack_o;
   input 		     wbs1_err_o;
   input 		     wbs1_rty_o;   

   wire [1:0] 		     slave_sel; // One bit per slave

   reg 			     watchdog_err;
   
`ifdef ARBITER_IBUS_WATCHDOG
   reg [`ARBITER_IBUS_WATCHDOG_TIMER_WIDTH:0]  watchdog_timer;
   reg 			     wbm_stb_r; // Register strobe
   wire 		     wbm_stb_edge; // Detect its edge
   reg 			     wbm_stb_edge_r, wbm_ack_i_r; // Reg these, better timing

   always @(posedge wb_clk)
     wbm_stb_r <= wbm_stb_o;

   assign wbm_stb_edge = (wbm_stb_o & !wbm_stb_r);

   always @(posedge wb_clk)
     wbm_stb_edge_r <= wbm_stb_edge;
   
   always @(posedge wb_clk)
     wbm_ack_i_r <= wbm_ack_i;
   
   
   // Counter logic
   always @(posedge wb_clk)
     if (wb_rst) watchdog_timer <= 0;
     else if (wbm_ack_i_r) // When we see an ack, turn off timer
       watchdog_timer <= 0;
     else if (wbm_stb_edge_r) // New access means start timer again
       watchdog_timer <= 1;
     else if (|watchdog_timer) // Continue counting if counter > 0
       watchdog_timer <= watchdog_timer + 1;

   always @(posedge wb_clk) 
     watchdog_err <= (&watchdog_timer);
   
`else // !`ifdef ARBITER_IBUS_WATCHDOG
   
   always @(posedge wb_clk) 
     watchdog_err <= 0;

`endif // !`ifdef ARBITER_IBUS_WATCHDOG
   
   // Slave select
     // ROM/RAM
   assign slave_sel[0] = ~|wbm_adr_o[wb_adr_width - 1:slave0_addr_width];
     // DDR
   assign slave_sel[1] = ~slave_sel[0] & ~|wbm_adr_o[wb_adr_width - 1:slave1_addr_width];

   // Slave out assigns
   assign wbs0_adr_i = wbm_adr_o;
   assign wbs0_dat_i = wbm_dat_o;
   assign wbs0_we_i  = wbm_we_o;
   assign wbs0_sel_i = wbm_sel_o;
   assign wbs0_cti_i = wbm_cti_o;
   assign wbs0_bte_i = wbm_bte_o;
   assign wbs0_cyc_i = wbm_cyc_o & slave_sel[0];
   assign wbs0_stb_i = wbm_stb_o & slave_sel[0];

   assign wbs1_adr_i = wbm_adr_o;
   assign wbs1_dat_i = wbm_dat_o;
   assign wbs1_we_i  = wbm_we_o;
   assign wbs1_sel_i = wbm_sel_o;
   assign wbs1_cti_i = wbm_cti_o;
   assign wbs1_bte_i = wbm_bte_o;
   assign wbs1_cyc_i = wbm_cyc_o & slave_sel[1];
   assign wbs1_stb_i = wbm_stb_o & slave_sel[1];

   // Master out assigns
   // Don't care about none selected...
   assign wbm_dat_i = slave_sel[1] ? wbs1_dat_o :
		      wbs0_dat_o ;
   
   assign wbm_ack_i = (slave_sel[0] & wbs0_ack_o) |
		      (slave_sel[1] & wbs1_ack_o);
   
   
   assign wbm_err_i = (slave_sel[0] & wbs0_err_o) |
		      (slave_sel[1] & wbs1_err_o) |
		      watchdog_err;
   
   assign wbm_rty_i = (slave_sel[0] & wbs0_rty_o) |
		      (slave_sel[1] & wbs1_rty_o);
endmodule // arbiter_ibus

