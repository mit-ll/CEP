//
// Copyright (C) 2019 Massachusetts Institute of Technology
//

//////////////////////////////////////////////////////////////////////////////////
// Description:    Generate 1023 chip sequence for SV PRN number prn_num.
//                 First chip on chip_out after reset.
//                 Asserting enb will advance to next chip.
//
//////////////////////////////////////////////////////////////////////////////////


module cacode(
           input clk,
           input rst,
           input [5:0] prn_num,
           input enb,
           output reg chip_out
       );

reg [10:1] g1, g2;
reg chip;


always @ (posedge clk)
    if (rst)
        begin
            g1 <= 10'b1111111111;
            g2 <= 10'b1111111111;
        end
    
    else if (enb)
            begin
                g1[10:1] <= {g1[9:1], g1[3] ^ g1[10]};
                g2[10:1] <= {g2[9:1], g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10]};
            end


always @(*)
    begin
        if(rst)
           chip <= 1'b0; 
       else
    begin
        case (prn_num)
            6'd1 :
                chip <= g1[10] ^ g2[2] ^ g2[6];
            6'd2 :
                chip <= g1[10] ^ g2[3] ^ g2[7];
            6'd3 :
                chip <= g1[10] ^ g2[4] ^ g2[8];
            6'd4 :
                chip <= g1[10] ^ g2[5] ^ g2[9];
            6'd5 :
                chip <= g1[10] ^ g2[1] ^ g2[9];
            6'd6 :
                chip <= g1[10] ^ g2[2] ^ g2[10];
            6'd7 :
                chip <= g1[10] ^ g2[1] ^ g2[8];
            6'd8 :
                chip <= g1[10] ^ g2[2] ^ g2[9];
            6'd9 :
                chip <= g1[10] ^ g2[3] ^ g2[10];
            6'd10:
                chip <= g1[10] ^ g2[2] ^ g2[3];
            6'd11:
                chip <= g1[10] ^ g2[3] ^ g2[4];
            6'd12:
                chip <= g1[10] ^ g2[5] ^ g2[6];
            6'd13:
                chip <= g1[10] ^ g2[6] ^ g2[7];
            6'd14:
                chip <= g1[10] ^ g2[7] ^ g2[8];
            6'd15:
                chip <= g1[10] ^ g2[8] ^ g2[9];
            6'd16:
                chip <= g1[10] ^ g2[9] ^ g2[10];
            6'd17:
                chip <= g1[10] ^ g2[1] ^ g2[4];
            6'd18:
                chip <= g1[10] ^ g2[2] ^ g2[5];
            6'd19:
                chip <= g1[10] ^ g2[3] ^ g2[6];
            6'd20:
                chip <= g1[10] ^ g2[4] ^ g2[7];
            6'd21:
                chip <= g1[10] ^ g2[5] ^ g2[8];
            6'd22:
                chip <= g1[10] ^ g2[6] ^ g2[9];
            6'd23:
                chip <= g1[10] ^ g2[1] ^ g2[3];
            6'd24:
                chip <= g1[10] ^ g2[4] ^ g2[6];
            6'd25:
                chip <= g1[10] ^ g2[5] ^ g2[7];
            6'd26:
                chip <= g1[10] ^ g2[6] ^ g2[8];
            6'd27:
                chip <= g1[10] ^ g2[7] ^ g2[9];
            6'd28:
                chip <= g1[10] ^ g2[8] ^ g2[10];
            6'd29:
                chip <= g1[10] ^ g2[1] ^ g2[6];
            6'd30:
                chip <= g1[10] ^ g2[2] ^ g2[7];
            6'd31:
                chip <= g1[10] ^ g2[3] ^ g2[8];
            6'd32:
                chip <= g1[10] ^ g2[4] ^ g2[9];
            6'd33:
                chip <= g1[10] ^ g2[5] ^ g2[10];
            6'd34:
                chip <= g1[10] ^ g2[4] ^ g2[10];
            6'd35:
                chip <= g1[10] ^ g2[1] ^ g2[7];
            6'd36:
                chip <= g1[10] ^ g2[2] ^ g2[8];
            6'd37:
                chip <= g1[10] ^ g2[4] ^ g2[10];
            //
            default:
                chip <= 1'b0;  //invalid prn_num
        endcase
    end
end


// reclock for timing
always @ (posedge clk)
    begin
        if(rst)
            chip_out<= 1'b0;
        else
            chip_out <= chip;
    end


endmodule




