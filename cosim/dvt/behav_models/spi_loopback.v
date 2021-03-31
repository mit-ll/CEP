module spi_loopback
  (
   // SPI Interface
   input      SCK,
   output reg MISO,
   input      MOSI,
   input      CS_n
   );

   initial MISO = 1'b1;
   always @(posedge  SCK) begin
      if (!CS_n) MISO <= MOSI;
   end
   
endmodule // spi_loopback
