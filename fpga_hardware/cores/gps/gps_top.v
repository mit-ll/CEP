module gps_top (
   wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i,
   wb_ack_o, wb_err_o, wb_dat_o,
   wb_clk_i, wb_rst_i, int_o
);

   parameter dw = 32;
   parameter aw = 32;

   input [aw-1:0] wb_adr_i;
   input	  wb_cyc_i;
   input [dw-1:0] wb_dat_i;
   input [3:0]	  wb_sel_i;
   input	  wb_stb_i;
   input	  wb_we_i;
   
   output	  wb_ack_o;
   output	  wb_err_o;
   output reg [dw-1:0] 	wb_dat_o;
   output         int_o;
   
   input	  wb_clk_i;
   input	  wb_rst_i;


   assign wb_ack_o = 1'b1;
   assign wb_err_o = 1'b0;
   assign int_o = 1'b0;

   // Internal registers
   reg genNext;
   wire [12:0] ca_code;
   wire [127:0] p_code;
   wire [127:0] l_code;
   wire codes_valid;
   
   // Implement GPS I/O memory map interface
   // Write side
   always @(posedge wb_clk_i) begin
     if(wb_rst_i) begin
       genNext <= 0;
     end
     else if(wb_stb_i & wb_we_i)
       case(wb_adr_i[5:2])
         0: genNext <= wb_dat_i[0];
         default: ;
       endcase
   end // always @ (posedge wb_clk_i)

   // Implement GPS I/O memory map interface
   // Read side
   always @(*) begin
      case(wb_adr_i[5:2])
        0: wb_dat_o = {31'b0, codes_valid};
        1: wb_dat_o = {19'b0, ca_code};
        2: wb_dat_o = p_code[31:0];
        3: wb_dat_o = p_code[63:32];
        4: wb_dat_o = p_code[95:64];
        5: wb_dat_o = p_code[127:96];
        6: wb_dat_o = l_code[31:0];
        7: wb_dat_o = l_code[63:32];
        8: wb_dat_o = l_code[95:64];
        9: wb_dat_o = l_code[127:96];
        default: wb_dat_o = 32'b0;
      endcase
   end // always @ (*)

gps gps(
   wb_clk_i,
   wb_rst_i,
   6'd12,
   genNext,
   ca_code,
   p_code,
   l_code,
   codes_valid
);

endmodule
