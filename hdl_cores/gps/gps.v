//
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// From the GPS transmitter's perspective
module gps(
           sys_clk_50,
           sync_rst_in,
           sv_num,
           startRound,
           ca_code,
           p_code,
           l_code,
           l_code_valid
       );
input  sys_clk_50;
input  sync_rst_in;
input [5:0] sv_num;
input startRound;
output reg [12:0] ca_code;
output [127:0] p_code;
output [127:0] l_code;
output l_code_valid;

reg startRound_r;
wire gps_clk_fast;

always @(posedge gps_clk_fast)
    begin
        if(sync_rst_in)
            begin
                startRound_r <= 0;
            end
        else
            begin
                startRound_r <= startRound;
            end
    end
wire startRoundPosEdge = ~startRound_r & startRound;
   //
   // Control code generators
   //
reg code_gen_en;
reg [3:0] ca_bit_count;
reg [7:0] p_bit_count;
always @(posedge gps_clk_fast)
  begin
     if (sync_rst_in)
       code_gen_en <= 1'b0;
     // make reset and start seperate to remove reset dependency
     //if(sync_rst_in | startRoundPosEdge)
     else if(startRoundPosEdge)     
       begin
          code_gen_en <= startRoundPosEdge;
       end
     else if(p_bit_count == 8'd128 && ca_bit_count == 4'd13)
       begin
          code_gen_en <= 1'b0;
       end
  end

   //
   // Tony Duong: Added logic to detect a change in *sv_num* number and ca-code and p-code calculation automatically
   // to recompute CA-Code
   //
   reg [5:0] last_sv_num;
   wire      sv_changed = last_sv_num != sv_num;
   always @(posedge gps_clk_fast) begin
      if (sync_rst_in) last_sv_num <= 0;
      else last_sv_num <= sv_num;
   end
   //
   // 
   //

// Generate C/A code
wire ca_code_bit;
cacode ca(
          .clk         (gps_clk_slow),
          .rst         (sync_rst_in) ,
          .prn_num     (sv_num),
	  .prn_changed (sv_changed),
          .enb         (code_gen_en),
          .chip_out    (ca_code_bit)
       );

// Deal with clock differences by using code gen edge for slow clock
reg code_gen_en_r;
wire codeGenPosEdge = ~code_gen_en_r & code_gen_en;
always @(posedge gps_clk_slow)
    begin
        if(sync_rst_in)
            begin
                code_gen_en_r <= 0;
            end
        else
            begin
                code_gen_en_r <= code_gen_en;
            end
    end

// Save 13 ca-code bits
always @(posedge gps_clk_slow)
    begin
//        if(sync_rst_in | codeGenPosEdge)
        if(sync_rst_in) 
            begin
                ca_bit_count <= 1'b0;
                ca_code  <= 13'b0;
            end
	else if (codeGenPosEdge) begin // tony D.
           ca_bit_count <= 1'b0;
           ca_code  <= 13'b0;	   
	end
        else
            begin
                if(ca_bit_count < 4'd13)
                    begin
                        ca_bit_count <= ca_bit_count + 1;
                        ca_code <= {ca_code[11:0], ca_code_bit};
                    end
            end
    end

// Generate P-code
wire p_code_bit;
pcode p(
	.clk (gps_clk_fast),
        .reset (sync_rst_in ),
	.prn_changed (sv_changed),
        .en  (code_gen_en ),
        .sat (sv_num      ),  
        .preg(p_code_bit  )
      );

// Save 128 p-code bits, encrypt and send as l_code
reg [127:0] p_pt;
always @(posedge gps_clk_fast)
    begin
       //if(sync_rst_in | startRoundPosEdge)
       if (sync_rst_in) begin
          p_bit_count <= 0;
 	  p_pt <= 0; 
       end
       else if(startRoundPosEdge)       
         begin
            p_bit_count <= 0;
 	    p_pt <= 0; // Tony D. 04/02/20 Need to reset this hting else getting X in vector capture	       
         end
//       else
       else if ( code_gen_en) begin     // tony D.
         begin
            if(p_bit_count < 8'd128)
              begin
                 p_bit_count <= p_bit_count + 1;
                 p_pt <= {p_pt [126:0], p_code_bit};
              end
         end
       end
    end // always @ (posedge gps_clk_fast)
   

assign p_code = p_pt;

reg encrypt;
always @(posedge gps_clk_fast)
  begin
     if (sync_rst_in) encrypt <= 0; // Added TOny D.
     else begin
        encrypt <= 0;
        if(p_bit_count == 8'd127)
          encrypt <= 1'b1;
     end
  end
   
   //
   // Tony Duong 05/12/2020: Even though CA-Code is transmitting at 10x slower then P/L-code, there is no reason why we need to compute
   // the CA-Code with 10x slow clock as long as we can make timing...
   //
//`define USE_SLOW_CLOCK
`ifdef USE_SLOW_CLOCK
  gps_clkgen gps_clkgen_inst(
    .sys_clk_50(sys_clk_50),
    .sync_rst_in(sync_rst_in),

    .gps_clk_fast(gps_clk_fast),
    .gps_clk_slow(gps_clk_slow),
    .gps_rst());
`else
   assign gps_clk_fast = sys_clk_50;
   assign gps_clk_slow = sys_clk_50;
`endif

   //
   // NOTE: key should be changed to input so they can be programmed to anything to reflect true usage
   // TOny D. 05/12/20
   //
   // in order to remove the reset dependency, we need create a mask to mask out the out_valid from aes_192 module
   // while computing CA-code and P-code, the valid is only applied when encryt=1
   reg l_valid_mask;
   wire l_code_valid_i;
   assign l_code_valid = l_code_valid_i & l_valid_mask;
   //
   always @(posedge gps_clk_fast) begin
      if (sync_rst_in)         l_valid_mask <= 0;
      else if (codeGenPosEdge) l_valid_mask <= 0; // clear the mask
      else if (encrypt)        l_valid_mask <= 1; // this happens             
   end
   //
  aes_192 aes_192_inst (
    .clk        (gps_clk_fast),
//
// tony Duong: remove reset dependency
//  .rst        (sync_rst_in || startRoundPosEdge),
    .rst        (sync_rst_in),			
    .start      (encrypt),
    .state      (p_pt),
    .key        (192'hAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA),
    .out        (l_code),
    .out_valid  (l_code_valid_i)
  );

endmodule
