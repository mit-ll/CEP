module ram_wb_b3(
		 wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
		 wb_stb_i, wb_we_i,

		 wb_ack_o, wb_err_o, wb_rty_o, wb_dat_o,

		 wb_clk_i, wb_rst_i);

   parameter dw = 32;
   parameter aw = 32;

   input [aw-1:0]	wb_adr_i;
   input [1:0] 		wb_bte_i;
   input [2:0] 		wb_cti_i;
   input 		wb_cyc_i;
   input [dw-1:0] 	wb_dat_i;
   input [3:0] 		wb_sel_i;
   input 		wb_stb_i;
   input 		wb_we_i;
   
   output 		wb_ack_o;
   output 		wb_err_o;
   output 		wb_rty_o;
   output [dw-1:0] 	wb_dat_o;
   
   input 		wb_clk_i;
   input 		wb_rst_i;

   // Memory parameters
   parameter mem_size_bytes = 32'h0002_0000; // 128KBytes
   parameter mem_adr_width = 17;             //(log2(mem_size_bytes));
   
   parameter bytes_per_dw = (dw/8);
   parameter adr_width_for_num_word_bytes = 2; //(log2(bytes_per_dw))
   parameter mem_words = (mem_size_bytes/bytes_per_dw);   

   // synthesis attribute ram_style of mem is block
   reg [dw-1:0] 	mem [0:mem_words-1]   /* synthesis ram_style = block */;

   // Register to address internal memory array
   reg [(mem_adr_width-adr_width_for_num_word_bytes)-1:0] adr;
   
   wire [31:0] 			   wr_data;

   // Register to indicate if the cycle is a Wishbone B3-registered feedback 
   // type access
   reg 				   wb_b3_trans;
   wire 			   wb_b3_trans_start, wb_b3_trans_stop;
   
   // Register to use for counting the addresses when doing burst accesses
   reg [mem_adr_width-adr_width_for_num_word_bytes-1:0]  burst_adr_counter;
   reg [2:0] 			   wb_cti_i_r;
   reg [1:0] 			   wb_bte_i_r;
   wire 			   using_burst_adr;
   wire 			   burst_access_wrong_wb_adr;

   // Wire to indicate addressing error
   wire 			   addr_err;   
   
   
   // Logic to detect if there's a burst access going on
   assign wb_b3_trans_start = ((wb_cti_i == 3'b001)|(wb_cti_i == 3'b010)) & 
			      wb_stb_i & !wb_b3_trans;
   
   assign  wb_b3_trans_stop = ((wb_cti_i == 3'b111) & 
			      wb_stb_i & wb_b3_trans & wb_ack_o) | wb_err_o;
   
   always @(posedge wb_clk_i)
     if (wb_rst_i)
       wb_b3_trans <= 0;
     else if (wb_b3_trans_start)
       wb_b3_trans <= 1;
     else if (wb_b3_trans_stop)
       wb_b3_trans <= 0;

   // Burst address generation logic
   always @(/*AUTOSENSE*/wb_ack_o or wb_b3_trans or wb_b3_trans_start
	    or wb_bte_i_r or wb_cti_i_r or wb_adr_i or adr)
     if (wb_b3_trans_start)
       // Kick off burst_adr_counter, this assumes 4-byte words when getting
       // address off incoming Wishbone bus address! 
       // So if dw is no longer 4 bytes, change this!
       burst_adr_counter = wb_adr_i[mem_adr_width-1:2];
     else if ((wb_cti_i_r == 3'b010) & wb_ack_o & wb_b3_trans)
       // Incrementing burst
       begin
	  if (wb_bte_i_r == 2'b00) // Linear burst
	    burst_adr_counter = adr + 1;
	  if (wb_bte_i_r == 2'b01) // 4-beat wrap burst
	    burst_adr_counter[1:0] = adr[1:0] + 1;
	  if (wb_bte_i_r == 2'b10) // 8-beat wrap burst
	    burst_adr_counter[2:0] = adr[2:0] + 1;
	  if (wb_bte_i_r == 2'b11) // 16-beat wrap burst
	    burst_adr_counter[3:0] = adr[3:0] + 1;
       end // if ((wb_cti_i_r == 3'b010) & wb_ack_o_r)

   always @(posedge wb_clk_i)
     wb_bte_i_r <= wb_bte_i;

   // Register it locally
   always @(posedge wb_clk_i)
     wb_cti_i_r <= wb_cti_i;

   assign using_burst_adr = wb_b3_trans;
   
   assign burst_access_wrong_wb_adr = (using_burst_adr & 
				       (adr != wb_adr_i[mem_adr_width-1:2]));

   // Address registering logic
   always@(posedge wb_clk_i)
     if(wb_rst_i)
       adr <= 0;
     else if (using_burst_adr)
       adr <= burst_adr_counter;
     else if (wb_cyc_i & wb_stb_i)
       adr <= wb_adr_i[mem_adr_width-1:2];

   reg [(mem_adr_width-adr_width_for_num_word_bytes)-1:0] adr_comb;
   // Address registering logic, combinational version, to enable block ram infer
   always@(*)
     if(wb_rst_i)
       adr_comb = 0;
     else if (using_burst_adr)
       adr_comb = burst_adr_counter;
     else if (wb_cyc_i & wb_stb_i)
       adr_comb = wb_adr_i[mem_adr_width-1:2];

   // Memory initialisation.

   parameter memory_file = "sram.vmem";

// Must add -use_new_parser yes to XST command line for this to work
//otherwise XST requires an odd format
   integer k;
   initial begin
// synthesis translate_off
 	for(k = 0; k < (1 << mem_words); k = k + 1)
	  begin
             mem[k] = 0;
	  end
// synthesis translate_on
	$readmemh(memory_file, mem);
   end
   
   assign wb_rty_o = 0;

   // mux for data to ram, RMW on part sel != 4'hf
   assign wr_data[31:24] = wb_sel_i[3] ? wb_dat_i[31:24] : wb_dat_o[31:24];
   assign wr_data[23:16] = wb_sel_i[2] ? wb_dat_i[23:16] : wb_dat_o[23:16];
   assign wr_data[15: 8] = wb_sel_i[1] ? wb_dat_i[15: 8] : wb_dat_o[15: 8];
   assign wr_data[ 7: 0] = wb_sel_i[0] ? wb_dat_i[ 7: 0] : wb_dat_o[ 7: 0];
   
   wire ram_we;
   assign ram_we = wb_we_i & wb_ack_o;

   reg [dw-1:0] data_from_mem;
  always @(posedge wb_clk_i)begin
    if(wb_cyc_i)
      if(~ram_we)
        data_from_mem <= mem[adr_comb]; // comb
      else
        mem[adr_comb] <= wr_data;
  end

   assign wb_dat_o = data_from_mem;
   
   // Ack Logic
   reg wb_ack_o_r;

   assign wb_ack_o = wb_ack_o_r & wb_stb_i & 
		     !(burst_access_wrong_wb_adr | addr_err);
   
   always @ (posedge wb_clk_i)
     if (wb_rst_i)
       wb_ack_o_r <= 1'b0;
     else if (wb_cyc_i) // We have bus
       begin
	  if (addr_err)
	    begin
	       wb_ack_o_r <= 1;
	    end
	  else if (wb_cti_i == 3'b000)
	    begin
	       // Classic cycle acks
	       if (wb_stb_i)
		 begin
		    if (!wb_ack_o_r)
		      wb_ack_o_r <= 1;
		    else
		      wb_ack_o_r <= 0;
		 end
	    end // if (wb_cti_i == 3'b000)
	  else if ((wb_cti_i == 3'b001) | (wb_cti_i == 3'b010))
	    begin
	       // Increment/constant address bursts
	       if (wb_stb_i)
		 wb_ack_o_r <= 1;
	       else
		 wb_ack_o_r <= 0;
	    end
	  else if (wb_cti_i == 3'b111)
	    begin
	       // End of cycle
	       if (!wb_ack_o_r)
		 wb_ack_o_r <= wb_stb_i;
	       else
		 wb_ack_o_r <= 0;
	    end
       end // if (wb_cyc_i)
     else
       wb_ack_o_r <= 0;


   //
   // Error signal generation
   //
   
   // Error when out of bounds of memory - skip top byte of address in case
   // this is mapped somewhere other than 0x00.
   assign addr_err  = wb_cyc_i & wb_stb_i & (|wb_adr_i[aw-1:mem_adr_width]);
 
   
   // OR in other errors here...
   assign wb_err_o =  wb_ack_o_r & wb_stb_i & 
		      (burst_access_wrong_wb_adr | addr_err);
endmodule // ram_wb_b3

