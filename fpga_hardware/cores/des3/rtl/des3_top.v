module des3_top(
         wb_adr_i, wb_cyc_i, wb_dat_i, wb_sel_i,
         wb_stb_i, wb_we_i,
         wb_ack_o, wb_err_o, wb_dat_o,
         wb_clk_i, wb_rst_i, int_o
);

   parameter dw = 32;
   parameter aw = 32;

   input [aw-1:0]      wb_adr_i;
   input               wb_cyc_i;
   input [dw-1:0]      wb_dat_i;
   input [3:0]         wb_sel_i;
   input               wb_stb_i;
   input               wb_we_i;
   
   output              wb_ack_o;
   output              wb_err_o;
   output reg [dw-1:0] wb_dat_o;
   output              int_o;
   
   input               wb_clk_i;
   input               wb_rst_i;

   assign wb_ack_o = 1'b1;
   assign wb_err_o = 1'b0;
   assign int_o = 1'b0;

   // Internal registers
   reg start;
   reg decrypt;
   reg [31:0] desIn [0:1];
   reg [31:0] key   [0:5];

   wire [63:0] des_in={desIn[0], desIn[1]};
   wire [55:0] key_1 ={key[0][27:0], key[1][27:0]};
   wire [55:0] key_2 ={key[2][27:0], key[3][27:0]};
   wire [55:0] key_3 ={key[4][27:0], key[5][27:0]};
   wire [63:0] ct;
   wire ct_valid;

   // Implement MD5 I/O memory map interface
   // Write side
   always @(posedge wb_clk_i) begin
     if(wb_rst_i) begin
       start   <= 0;
       decrypt <= 0;
       desIn[1]<= 0;
       desIn[0]<= 0;
       key[5]  <= 0;
       key[4]  <= 0;
       key[3]  <= 0;
       key[2]  <= 0;
       key[1]  <= 0;
       key[0]  <= 0;
     end
     else if(wb_stb_i & wb_we_i)
       case(wb_adr_i[5:2])
         0: start   <= wb_dat_i[0];
         1: decrypt <= wb_dat_i[0];
         2: desIn[1]<= wb_dat_i;
         3: desIn[0]<= wb_dat_i;
         4: key[5]  <= wb_dat_i;
         5: key[4]  <= wb_dat_i;
         6: key[3]  <= wb_dat_i;
         7: key[2]  <= wb_dat_i;
         8: key[1]  <= wb_dat_i;
         9: key[0]  <= wb_dat_i;
         default: ;
       endcase
   end // always @ (posedge wb_clk_i)

   // Implement MD5 I/O memory map interface
   // Read side
   always @(*) begin
      case(wb_adr_i[5:2])
         0: wb_dat_o = {31'b0, start};
         1: wb_dat_o = {31'b0, decrypt};
         2: wb_dat_o = desIn[1];
         3: wb_dat_o = desIn[0];
         4: wb_dat_o = key[5];
         5: wb_dat_o = key[4];
         6: wb_dat_o = key[3];
         7: wb_dat_o = key[2];
         8: wb_dat_o = key[1];
         9: wb_dat_o = key[0];
        10: wb_dat_o = {31'b0, ct_valid};
        11: wb_dat_o = ct[63:32];
        12: wb_dat_o = ct[31:0];
        default: wb_dat_o = 32'b0;
      endcase
   end // always @ (*)
   
  des3 des3(
    .desOut(ct),
    .out_valid(ct_valid),
    .start(start),
    .desIn(des_in),
    .key1(key_1),
    .key2(key_2),
    .key3(key_3),
    .decrypt(decrypt),
    .clk(wb_clk_i));
endmodule
