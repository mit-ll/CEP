/*
 * Copyright 2012, Homer Hsing <homer.hsing@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// This is a fully unrolled implementation
module aes_192 (clk, rst, start, state, key, out, out_valid);
    input           clk; 
    input           rst; 
    input           start;
    input  [127:0]  state;
    input  [191:0]  key;
    output [127:0]  out;
    output          out_valid;

    // Internals signals and such
    reg    [127:0]  s0;
    reg    [191:0]  k0;
    wire   [127:0]  s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    wire   [191:0]  k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11;
    wire   [127:0]  k0b, k1b, k2b, k3b, k4b, k5b, k6b, k7b, k8b, k9b, k10b, k11b;
    reg             start_r;
    wire            start_posedge = start & ~start_r;
    reg    [4:0]    validCounter;
    
    always @(posedge clk or posedge rst)
    begin
        if (rst)
            start_r <= 1'b0;
        else
            start_r <= start;
    end // end always

    always @ (posedge clk or posedge rst)
    begin
        if (rst) begin
            s0              <= 0;
            k0              <= 0;
            validCounter    <= 0;
        end else
            if(start_posedge)
                begin
                    s0 <= state ^ key[191:64];
                    k0 <= key;
                    validCounter <= 26;
                end
            else if(validCounter > 1)
                begin
                    validCounter <= validCounter - 1;
                end
    end // end always

    assign out_valid = (validCounter == 1);
    
    expand_key_type_D_192  a0 (clk, rst, k0, 8'h1,   k1,  k0b);
    expand_key_type_B_192  a1 (clk, rst, k1,         k2,  k1b);
    expand_key_type_A_192  a2 (clk, rst, k2, 8'h2,   k3,  k2b);
    expand_key_type_C_192  a3 (clk, rst, k3, 8'h4,   k4,  k3b);
    expand_key_type_B_192  a4 (clk, rst, k4,         k5,  k4b);
    expand_key_type_A_192  a5 (clk, rst, k5, 8'h8,   k6,  k5b);
    expand_key_type_C_192  a6 (clk, rst, k6, 8'h10,  k7,  k6b);
    expand_key_type_B_192  a7 (clk, rst, k7,         k8,  k7b);
    expand_key_type_A_192  a8 (clk, rst, k8, 8'h20,  k9,  k8b);
    expand_key_type_C_192  a9 (clk, rst, k9, 8'h40,  k10, k9b);
    expand_key_type_B_192 a10 (clk, rst, k10,        k11, k10b);
    expand_key_type_A_192 a11 (clk, rst, k11, 8'h80,    , k11b);

    one_round
        r1 (clk, rst, s0, k0b, s1),
        r2 (clk, rst, s1, k1b, s2),
        r3 (clk, rst, s2, k2b, s3),
        r4 (clk, rst, s3, k3b, s4),
        r5 (clk, rst, s4, k4b, s5),
        r6 (clk, rst, s5, k5b, s6),
        r7 (clk, rst, s6, k6b, s7),
        r8 (clk, rst, s7, k7b, s8),
        r9 (clk, rst, s8, k8b, s9),
        r10 (clk, rst, s9, k9b, s10),
        r11 (clk, rst, s10, k10b, s11);

    final_round
        rf (clk, rst, s11, k11b, out);

endmodule

/* expand k0,k1,k2,k3 for every two clock cycles */
module expand_key_type_A_192 (clk, rst, in, rcon, out_1, out_2);
    input              clk;
    input              rst;
    input      [191:0] in;
    input      [7:0]   rcon;
    output reg [191:0] out_1;
    output     [127:0] out_2;

    // Internal signals
    wire       [31:0]  k0, k1, k2, k3, k4, k5, v0, v1, v2, v3;
    reg        [31:0]  k0a, k1a, k2a, k3a, k4a, k5a;
    wire       [31:0]  k0b, k1b, k2b, k3b, k4b, k5b, k6a;

    assign {k0, k1, k2, k3, k4, k5} = in;

    assign v0 = {k0[31:24] ^ rcon, k0[23:0]};
    assign v1 = v0 ^ k1;
    assign v2 = v1 ^ k2;
    assign v3 = v2 ^ k3;

    always @ (posedge clk or posedge rst) 
    begin
        if (rst)
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {32'd0, 32'd0, 32'd0, 32'd0, 32'd0, 32'd0};
        else
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {v0, v1, v2, v3, k4, k5};
    end // end always

    S4 S4_0 (clk, rst, {k5[23:0], k5[31:24]}, k6a);

    assign k0b = k0a ^ k6a;
    assign k1b = k1a ^ k6a;
    assign k2b = k2a ^ k6a;
    assign k3b = k3a ^ k6a;
    assign {k4b, k5b} = {k4a, k5a};

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            out_1 <= 0;
        else
            out_1 <= {k0b, k1b, k2b, k3b, k4b, k5b};
    end // end always

    assign out_2 = {k0b, k1b, k2b, k3b};

endmodule   // end module expand_key_type_A_192


/* expand k2,k3,k4,k5 for every two clock cycles */
module expand_key_type_B_192 (clk, rst, in, out_1, out_2);
    input              clk;
    input              rst;
    input      [191:0] in;
    output reg [191:0] out_1;
    output     [127:0] out_2;
    wire       [31:0]  k0, k1, k2, k3, k4, k5, v2, v3, v4, v5;
    reg        [31:0]  k0a, k1a, k2a, k3a, k4a, k5a;

    assign {k0, k1, k2, k3, k4, k5} = in;

    assign v2 = k1 ^ k2;
    assign v3 = v2 ^ k3;
    assign v4 = v3 ^ k4;
    assign v5 = v4 ^ k5;

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {32'd0, 32'd0, 32'd0, 32'd0, 32'd0, 32'd0};
        else
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {k0, k1, v2, v3, v4, v5};
    end // end always

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            out_1   <= 0;
        else
            out_1   <= {k0a, k1a, k2a, k3a, k4a, k5a};
    end

    assign out_2 = {k2a, k3a, k4a, k5a};

endmodule   // end expand_key_type_B_192



/* expand k0,k1,k4,k5 for every two clock cycles */
module expand_key_type_C_192 (clk, rst, in, rcon, out_1, out_2);
    input              clk;
    input              rst;
    input      [191:0] in;
    input      [7:0]   rcon;
    output reg [191:0] out_1;
    output     [127:0] out_2;

    wire       [31:0]  k0, k1, k2, k3, k4, k5, v4, v5, v0, v1;
    reg        [31:0]  k0a, k1a, k2a, k3a, k4a, k5a;
    wire       [31:0]  k0b, k1b, k2b, k3b, k4b, k5b, k6a;

    assign {k0, k1, k2, k3, k4, k5} = in;
    
    assign v4 = k3 ^ k4;
    assign v5 = v4 ^ k5;
    assign v0 = {k0[31:24] ^ rcon, k0[23:0]};
    assign v1 = v0 ^ k1;

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {32'd0, 32'd0, 32'd0, 32'd0, 32'd0, 32'd0};
        else
            {k0a, k1a, k2a, k3a, k4a, k5a} <= {v0, v1, k2, k3, v4, v5};
    end

    S4 S4_0 (clk, rst, {v5[23:0], v5[31:24]}, k6a);

    assign k0b = k0a ^ k6a;
    assign k1b = k1a ^ k6a;
    assign {k2b, k3b, k4b, k5b} = {k2a, k3a, k4a, k5a};

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            out_1   <= 0;
        else
            out_1   <= {k0b, k1b, k2b, k3b, k4b, k5b};
    end

    assign out_2 = {k4b, k5b, k0b, k1b};

endmodule   // end expand_key_type_C_192

/* expand k0,k1 for every two clock cycles */
module expand_key_type_D_192 (clk, rst, in, rcon, out_1, out_2);
    input              clk;
    input              rst;
    input      [191:0] in;
    input      [7:0]   rcon;
    output reg [191:0] out_1;
    output     [127:0] out_2;
    wire       [31:0]  k0, k1, k2, k3, k4, k5, v0, v1;
    reg        [31:0]  k0a, k1a, k2a, k3a, k4a, k5a;
    wire       [31:0]  k0b, k1b, k2b, k3b, k4b, k5b, k6a;

    assign {k0, k1, k2, k3, k4, k5} = in;

    assign v0 = {k0[31:24] ^ rcon, k0[23:0]};
    assign v1 = v0 ^ k1;

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            {k0a, k1a, k2a, k3a, k4a, k5a}  <= {32'd0, 32'd0, 32'd0, 32'd0, 32'd0, 32'd0};
        else
            {k0a, k1a, k2a, k3a, k4a, k5a}  <= {v0, v1, k2, k3, k4, k5};
    end // end always

    S4 S4_0 (clk, rst, {k5[23:0], k5[31:24]}, k6a);

    assign k0b = k0a ^ k6a;
    assign k1b = k1a ^ k6a;
    assign {k2b, k3b, k4b, k5b} = {k2a, k3a, k4a, k5a};

    always @ (posedge clk or posedge rst)
    begin
        if (rst)
            out_1   <= 0;
        else
            out_1   <= {k0b, k1b, k2b, k3b, k4b, k5b};
    end // end always

    assign out_2 = {k4b, k5b, k0b, k1b};

endmodule   // end expand_key_type_D_192   
