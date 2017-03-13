`include "orpsoc-defines.v"

// 2 Masters, a few slaves
module arbiter_dbus
  (
   // or1200 data master
   // Wishbone Master interface
   wbm0_adr_o,
   wbm0_dat_o,
   wbm0_sel_o,
   wbm0_we_o,
   wbm0_cyc_o,
   wbm0_stb_o,
   wbm0_cti_o,
   wbm0_bte_o,
  
   wbm0_dat_i,
   wbm0_ack_i,
   wbm0_err_i,
   wbm0_rty_i,

   // or1200 debug master
   // Wishbone Master interface
   wbm1_adr_o,
   wbm1_dat_o,
   wbm1_sel_o,
   wbm1_we_o,
   wbm1_cyc_o,
   wbm1_stb_o,
   wbm1_cti_o,
   wbm1_bte_o,
  
   wbm1_dat_i,
   wbm1_ack_i,
   wbm1_err_i,
   wbm1_rty_i,

   // Slave one - DDR2
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

   // Slave two - Ethernet
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


   // Slave three - Byte bus - default
   // Wishbone Slave interface
   wbs2_adr_i,
   wbs2_dat_i,
   wbs2_sel_i,		    
   wbs2_we_i,
   wbs2_cyc_i,
   wbs2_stb_i,
   wbs2_cti_i,
   wbs2_bte_i,
  
   wbs2_dat_o,
   wbs2_ack_o,
   wbs2_err_o,
   wbs2_rty_o,

   // Slave four - ROM/RAM
   // Wishbone Slave interface
   wbs3_adr_i,
   wbs3_dat_i,
   wbs3_sel_i,		    
   wbs3_we_i,
   wbs3_cyc_i,
   wbs3_stb_i,
   wbs3_cti_i,
   wbs3_bte_i,
  
   wbs3_dat_o,
   wbs3_ack_o,
   wbs3_err_o,
   wbs3_rty_o,
  
   wb_clk,
   wb_rst
   );

   parameter wb_dat_width = 32;
   parameter wb_adr_width = 32;

   parameter wb_addr_match_width = 8;
   parameter wb_num_slaves = 4; // defined in orpsoc-params and set in orpsoc_top

   // Slave addresses - these should be defparam'd from top level
   // Declare them as you need them
   parameter slave0_adr = 0;
   parameter slave1_adr = 0;
   parameter slave2_adr = 0;
   parameter slave3_adr = 0;
   parameter slave0_addr_width = 28; // low
   parameter slave1_addr_width = 8;  // high
   parameter slave2_addr_width = 8;  // high
   parameter slave3_addr_width = 12; // low


   // Select for slave 0
`define WB_ARB_ADDR_MATCH_SEL wb_adr_width-1:wb_adr_width-wb_addr_match_width

   input wb_clk;
   input wb_rst;
   
   // WB Master one
   input [wb_adr_width-1:0] wbm0_adr_o;
   input [wb_dat_width-1:0] wbm0_dat_o;
   input [3:0] 		    wbm0_sel_o;
   input 		    wbm0_we_o;
   input 		    wbm0_cyc_o;
   input 		    wbm0_stb_o;
   input [2:0] 		    wbm0_cti_o;
   input [1:0] 		    wbm0_bte_o;
   output [wb_dat_width-1:0] wbm0_dat_i;   
   output 		     wbm0_ack_i;
   output 		     wbm0_err_i;
   output 		     wbm0_rty_i;   
   
   
   input [wb_adr_width-1:0]  wbm1_adr_o;
   input [wb_dat_width-1:0]  wbm1_dat_o;
   input [3:0] 		     wbm1_sel_o;   
   input 		     wbm1_we_o;
   input 		     wbm1_cyc_o;
   input 		     wbm1_stb_o;
   input [2:0] 		     wbm1_cti_o;
   input [1:0] 		     wbm1_bte_o;
   output [wb_dat_width-1:0] wbm1_dat_i;   
   output 		     wbm1_ack_i;
   output 		     wbm1_err_i;
   output 		     wbm1_rty_i;   

   
   // Slave one
   // Wishbone Slave interface
   output [wb_adr_width-1:0] wbs0_adr_i;
   output [wb_dat_width-1:0] wbs0_dat_i;
   output [3:0]		     wbs0_sel_i;
   output 		     wbs0_we_i;
   output 		     wbs0_cyc_i;
   output 		     wbs0_stb_i;
   output [2:0] 	     wbs0_cti_i;
   output [1:0] 	     wbs0_bte_i;
   input [wb_dat_width-1:0]  wbs0_dat_o;
   input 		     wbs0_ack_o;
   input 		     wbs0_err_o;
   input 		     wbs0_rty_o;
   

   // Wishbone Slave interface
   output [wb_adr_width-1:0] wbs1_adr_i;
   output [wb_dat_width-1:0] wbs1_dat_i;
   output [3:0]		     wbs1_sel_i;
   output 		     wbs1_we_i;
   output 		     wbs1_cyc_i;
   output 		     wbs1_stb_i;
   output [2:0] 	     wbs1_cti_i;
   output [1:0] 	     wbs1_bte_i;
   input [wb_dat_width-1:0]  wbs1_dat_o;
   input 		     wbs1_ack_o;
   input 		     wbs1_err_o;
   input 		     wbs1_rty_o;
   

   // Wishbone Slave interface
   output [wb_adr_width-1:0] wbs2_adr_i;
   output [wb_dat_width-1:0] wbs2_dat_i;
   output [3:0]		     wbs2_sel_i;
   output 		     wbs2_we_i;
   output 		     wbs2_cyc_i;
   output 		     wbs2_stb_i;
   output [2:0] 	     wbs2_cti_i;
   output [1:0] 	     wbs2_bte_i;
   input [wb_dat_width-1:0]  wbs2_dat_o;
   input 		     wbs2_ack_o;
   input 		     wbs2_err_o;
   input 		     wbs2_rty_o;

   // Wishbone Slave interface
   output [wb_adr_width-1:0] wbs3_adr_i;
   output [wb_dat_width-1:0] wbs3_dat_i;
   output [3:0]		     wbs3_sel_i;
   output 		     wbs3_we_i;
   output 		     wbs3_cyc_i;
   output 		     wbs3_stb_i;
   output [2:0] 	     wbs3_cti_i;
   output [1:0] 	     wbs3_bte_i;
   input [wb_dat_width-1:0]  wbs3_dat_o;
   input 		     wbs3_ack_o;
   input 		     wbs3_err_o;
   input 		     wbs3_rty_o;

   reg 		     watchdog_err;
   
   // Master input mux output wires
   wire [wb_adr_width-1:0]   wbm_adr_o;
   wire [wb_dat_width-1:0]   wbm_dat_o;
   wire [3:0] 		     wbm_sel_o;
   wire 		     wbm_we_o;
   wire 		     wbm_cyc_o;
   wire 		     wbm_stb_o;
   wire [2:0] 		     wbm_cti_o;
   wire [1:0] 		     wbm_bte_o;
   
   // Master select
   wire [1:0] 		     master_sel;
   // priority to wbm1, the debug master
   assign master_sel[0] = wbm0_cyc_o & !wbm1_cyc_o;
   assign master_sel[1] = wbm1_cyc_o;


   // Master input mux, priority to debug master
   assign wbm_adr_o = master_sel[1] ? wbm1_adr_o :
		      wbm0_adr_o;
   
   assign wbm_dat_o = master_sel[1] ? wbm1_dat_o :
		      wbm0_dat_o;
   
   assign wbm_sel_o = master_sel[1] ? wbm1_sel_o :
		      wbm0_sel_o;

   assign wbm_we_o = master_sel[1] ? wbm1_we_o :
		      wbm0_we_o;
   
   assign wbm_cyc_o = master_sel[1] ? wbm1_cyc_o :
		      wbm0_cyc_o;

   assign wbm_stb_o = master_sel[1] ? wbm1_stb_o :
		     wbm0_stb_o;

   assign wbm_cti_o = master_sel[1] ? wbm1_cti_o :
		     wbm0_cti_o;

   assign wbm_bte_o = master_sel[1] ? wbm1_bte_o :
		      wbm0_bte_o;


   wire [wb_dat_width-1:0]   wbm_dat_i;   
   wire 		     wbm_ack_i;
   wire 		     wbm_err_i;
   wire 		     wbm_rty_i;   

   // Control what master gets the slave's response
   assign wbm0_dat_i = wbm_dat_i;
   assign wbm0_ack_i = wbm_ack_i & master_sel[0];
   assign wbm0_err_i = wbm_err_i & master_sel[0];
   assign wbm0_rty_i = wbm_rty_i & master_sel[0];
   
   assign wbm1_dat_i = wbm_dat_i;
   assign wbm1_ack_i = wbm_ack_i & master_sel[1];
   assign wbm1_err_i = wbm_err_i & master_sel[1];
   assign wbm1_rty_i = wbm_rty_i & master_sel[1];
   
   // Slave select wire
   wire [wb_num_slaves-1:0]  wb_slave_sel;
   reg [wb_num_slaves-1:0]   wb_slave_sel_r;

   // Register wb_slave_sel_r to break combinatorial loop when selecting default
   // slave
   always @(posedge wb_clk)
     wb_slave_sel_r <= wb_slave_sel;
   
   // Slave out mux in wires   
   wire [wb_dat_width-1:0]   wbs_dat_o_mux_i [0:wb_num_slaves-1];
   wire 		     wbs_ack_o_mux_i [0:wb_num_slaves-1];
   wire 		     wbs_err_o_mux_i [0:wb_num_slaves-1];
   wire 		     wbs_rty_o_mux_i [0:wb_num_slaves-1];

   //
   // Slave selects
   //
     // ROM/RAM - Given priority
   assign wb_slave_sel[3] = ~|wbm_adr_o[wb_adr_width - 1:slave3_addr_width];
     // DDR
   assign wb_slave_sel[0] = ~wb_slave_sel[3] & ~|wbm_adr_o[wb_adr_width-1:slave0_addr_width];
     // Ethernet
   assign wb_slave_sel[1] = wbm_adr_o[`WB_ARB_ADDR_MATCH_SEL] == slave1_adr;
     // Auto select last slave when others are not selected
   assign wb_slave_sel[2] = ~(wb_slave_sel_r[3] | wb_slave_sel_r[0] | wb_slave_sel_r[1]);
   

`ifdef ARBITER_DBUS_WATCHDOG
   reg [`ARBITER_DBUS_WATCHDOG_TIMER_WIDTH:0] watchdog_timer;
   reg 			     wbm_stb_r; // Register strobe
   wire 		     wbm_stb_edge; // Detect its edge

   always @(posedge wb_clk)
     wbm_stb_r <= wbm_stb_o;

   assign wbm_stb_edge = (wbm_stb_o & !wbm_stb_r);
   
   // Counter logic
   always @(posedge wb_clk)
     if (wb_rst) watchdog_timer <= 0;
     else if (wbm_ack_i) // When we see an ack, turn off timer
       watchdog_timer <= 0;
     else if (wbm_stb_edge) // New access means start timer again
       watchdog_timer <= 1;
     else if (|watchdog_timer) // Continue counting if counter > 0
       watchdog_timer <= watchdog_timer + 1;

   always @(posedge wb_clk) 
     watchdog_err <= (&watchdog_timer);

   
`else // !`ifdef ARBITER_DBUS_WATCHDOG
   
   always @(posedge wb_clk) 
     watchdog_err <= 0;
   
`endif // !`ifdef ARBITER_DBUS_WATCHDOG
   

   
   // Slave 0 inputs
   assign wbs0_adr_i = wbm_adr_o;
   assign wbs0_dat_i = wbm_dat_o;
   assign wbs0_sel_i = wbm_sel_o;
   assign wbs0_cyc_i = wbm_cyc_o & wb_slave_sel_r[0];
   assign wbs0_stb_i = wbm_stb_o & wb_slave_sel_r[0];   
   assign wbs0_we_i =  wbm_we_o;
   assign wbs0_cti_i = wbm_cti_o;
   assign wbs0_bte_i = wbm_bte_o;
   assign wbs_dat_o_mux_i[0] = wbs0_dat_o;
   assign wbs_ack_o_mux_i[0] = wbs0_ack_o & wb_slave_sel_r[0];
   assign wbs_err_o_mux_i[0] = wbs0_err_o & wb_slave_sel_r[0];
   assign wbs_rty_o_mux_i[0] = wbs0_rty_o & wb_slave_sel_r[0];


   // Slave 1 inputs
   assign wbs1_adr_i = wbm_adr_o;
   assign wbs1_dat_i = wbm_dat_o;
   assign wbs1_sel_i = wbm_sel_o;
   assign wbs1_cyc_i = wbm_cyc_o & wb_slave_sel_r[1];
   assign wbs1_stb_i = wbm_stb_o & wb_slave_sel_r[1];   
   assign wbs1_we_i =  wbm_we_o;
   assign wbs1_cti_i = wbm_cti_o;
   assign wbs1_bte_i = wbm_bte_o;
   assign wbs_dat_o_mux_i[1] = wbs1_dat_o;
   assign wbs_ack_o_mux_i[1] = wbs1_ack_o & wb_slave_sel_r[1];
   assign wbs_err_o_mux_i[1] = wbs1_err_o & wb_slave_sel_r[1];
   assign wbs_rty_o_mux_i[1] = wbs1_rty_o & wb_slave_sel_r[1];


   // Slave 2 inputs
   assign wbs2_adr_i = wbm_adr_o;
   assign wbs2_dat_i = wbm_dat_o;
   assign wbs2_sel_i = wbm_sel_o;
   assign wbs2_cyc_i = wbm_cyc_o & wb_slave_sel_r[2];
   assign wbs2_stb_i = wbm_stb_o & wb_slave_sel_r[2];   
   assign wbs2_we_i =  wbm_we_o;
   assign wbs2_cti_i = wbm_cti_o;
   assign wbs2_bte_i = wbm_bte_o;
   assign wbs_dat_o_mux_i[2] = wbs2_dat_o;
   assign wbs_ack_o_mux_i[2] = wbs2_ack_o & wb_slave_sel_r[2];
   assign wbs_err_o_mux_i[2] = wbs2_err_o & wb_slave_sel_r[2];
   assign wbs_rty_o_mux_i[2] = wbs2_rty_o & wb_slave_sel_r[2];

   // Slave 3 inputs
   assign wbs3_adr_i = wbm_adr_o;
   assign wbs3_dat_i = wbm_dat_o;
   assign wbs3_sel_i = wbm_sel_o;
   assign wbs3_cyc_i = wbm_cyc_o & wb_slave_sel_r[3];
   assign wbs3_stb_i = wbm_stb_o & wb_slave_sel_r[3];   
   assign wbs3_we_i =  wbm_we_o;
   assign wbs3_cti_i = wbm_cti_o;
   assign wbs3_bte_i = wbm_bte_o;
   assign wbs_dat_o_mux_i[3] = wbs3_dat_o;
   assign wbs_ack_o_mux_i[3] = wbs3_ack_o & wb_slave_sel_r[3];
   assign wbs_err_o_mux_i[3] = wbs3_err_o & wb_slave_sel_r[3];
   assign wbs_rty_o_mux_i[3] = wbs3_rty_o & wb_slave_sel_r[3];

   // Master out mux from slave in data
   assign wbm_dat_i = wb_slave_sel_r[0] ? wbs_dat_o_mux_i[0] :
		      wb_slave_sel_r[1] ? wbs_dat_o_mux_i[1] :
		      wb_slave_sel_r[2] ? wbs_dat_o_mux_i[2] :
		      wb_slave_sel_r[3] ? wbs_dat_o_mux_i[3] :
		      wbs_dat_o_mux_i[0];
   
   // Master out acks, or together
   assign wbm_ack_i = wbs_ack_o_mux_i[0] |
		      wbs_ack_o_mux_i[1] |
		      wbs_ack_o_mux_i[2] |
		      wbs_ack_o_mux_i[3] ;
   
   assign wbm_err_i = wbs_err_o_mux_i[0] |
		      wbs_err_o_mux_i[1] |
		      wbs_err_o_mux_i[2] |
		      wbs_err_o_mux_i[3] ;
   
   assign wbm_rty_i = wbs_rty_o_mux_i[0] |
		      wbs_rty_o_mux_i[1] |
		      wbs_rty_o_mux_i[2] |
		      wbs_rty_o_mux_i[3] ;
   
endmodule // arbiter_dbus

