module iir_top(
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
   reg next;
   reg [31:0] dataX [0:31];
   reg [31:0] dataY [0:31];
   reg [5:0] xSel=6'b0;
   reg [5:0] ySel=6'b0;
   reg [5:0] count;
   wire [31:0] dataIn, dataOut, dataR_Out;
   reg [31:0] dataW_In, dataW_addr, dataR_addr;

   reg data_valid;
   wire next_out;
   reg dataW, dataR;

   // Implement MD5 I/O memory map interface
   // Write side
   always @(posedge wb_clk_i) begin
     if(wb_rst_i) begin
         next           <= 0;
         dataW          <= 0;
         dataW_addr     <= 0;
         dataW_In       <= 0;
     end
     else if(wb_stb_i & wb_we_i)
       case(wb_adr_i[5:2])
         0: next           <= wb_dat_i[0];
         1: dataW          <= wb_dat_i[0];
         2: dataW_addr     <= wb_dat_i;
         3: dataW_In       <= wb_dat_i;
         4: dataR_addr     <= wb_dat_i;
         default: ;
       endcase
   end // always @ (posedge wb_clk_i)

   // Implement MD5 I/O memory map interface
   // Read side
   always @(*) begin
      case(wb_adr_i[5:2])
         0: wb_dat_o = {31'b0, next};
         1: wb_dat_o = {31'b0, dataW};
         2: wb_dat_o = dataW_addr;
         3: wb_dat_o = dataW_In;
         4: wb_dat_o = {31'b0, data_valid};
         5: wb_dat_o = {31'b0, dataR};
         6: wb_dat_o = dataR_addr;
         7: wb_dat_o = dataR_Out;
         default: wb_dat_o = 32'b0;
      endcase
   end // always @ (*)
  
   IIR_filter IIR_filter(
    .clk(wb_clk_i),
    .reset(~wb_rst_i),
    .inData(dataIn),
    .outData(dataOut));

    reg dataW_r;
    always @(posedge wb_clk_i) begin
        dataW_r <= dataW;
    end

    wire dataW_posedge = dataW & ~dataW_r;
        
   always @ (posedge wb_clk_i) begin
        if(dataW_posedge) begin
            dataX[dataW_addr] <= dataW_In;
        end
    	//dataR_Out=dataY[dataR_addr];
   end
   assign dataR_Out=dataY[dataR_addr];

    reg next_r;
    always @(posedge wb_clk_i) begin
        next_r <= next;
    end

    wire next_posedge = next & ~next_r;
        
   always @ (posedge wb_clk_i) begin
        if(next_posedge) begin
            xSel <= 6'h00;
        end
        else if(xSel<6'b100000) begin
            xSel <= xSel +1;
            //dataIn = dataX[xSel];
        end
   end
   assign dataIn = (xSel<6'b100000) ? dataX[xSel] : 32'b0;//dataX[xSel];
   
   always @ (posedge wb_clk_i) begin
        if(next_posedge) begin
            count <= 6'h00;
        end
        else if(xSel<4'b1010) begin
            count <= count +1;
        end
   end
   
   assign next_out = (count == 4'b1000);
   
    reg next_out_r;
    always @(posedge wb_clk_i) begin
        next_out_r <= next_out;
    end

    wire next_out_posedge = next_out & ~next_out_r;

   always @ (posedge wb_clk_i) begin
        if(next_out_posedge) begin
            ySel <= 6'h00;
            //data_valid <= 0;
        end
        else if(ySel<6'b100000) begin
            ySel <= ySel +1;
            dataY[ySel] = dataOut;
            //data_valid <= 1;
        end
   end

   always @ (posedge wb_clk_i) begin
        if(next_posedge) begin
            data_valid <= 0;
        end
        else if(next_out_posedge) begin
            data_valid <= 1;
        end
   end
   
   //assign data_valid = (ySel == 6'b100000);
endmodule
