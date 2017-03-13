/* test case mostly by Andres Molina-Markham amolina AT cs umass edu */

/*
 * Copyright 2010 UMass Amherst. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UMass Amherst ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <math.h>
// #include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "rsa.h"

/*
 * generate a public key and exponent suitable for RSA encryption like this:
 * % openssl genrsa -out foo.rsa <numbits>
 * % openssl rsa -in foo.rsa -pubout | openssl rsa -pubin -noout -text 
 *   <will output n and e>
*/   
#define KEYLEN 4
uint16_t n[KEYLEN];
uint16_t e[2];
uint16_t plaintext[KEYLEN];
uint16_t ciphertext[KEYLEN];

/* would be nice if we could use malloc instead of globals here */
uint16_t _tmpglobal_ab[2*KEYLEN];
uint16_t _tmpglobal_q[KEYLEN];
uint16_t _tmpglobal_x[2*KEYLEN];
uint16_t _tmpglobal_ybnt[2*KEYLEN];
uint16_t _tmpglobal_ybit[2*KEYLEN];
uint16_t _tmpglobal_qitybit[2*KEYLEN];
uint16_t _tmpglobal_temp[KEYLEN];

/**
 * One word addition with a carry bit
 * c_i = (a_1 + b_i + epsilon_prime) mod 0xFFFF
 */
uint16_t add_word(uint16_t * c_i, uint16_t a_i, uint16_t b_i, uint16_t epsilon_prime) {
    uint16_t epsilon; //The carry bit
    *c_i = (a_i + b_i + epsilon_prime);
	
    if ((*c_i >= (a_i + epsilon_prime)) && (*c_i >= b_i)) {
        epsilon = 0;
    } else {
        epsilon = 1;
    }
    return epsilon;
}

/**
 * One word subtraction with a borrow bit
 * c_i = (a_1 - b_i - epsilon_prime) mod 0xFFFF
 */
uint8_t subtract_word(uint16_t * c_i, uint16_t a_i, uint16_t b_i, uint8_t epsilon_prime) {
    uint8_t epsilon; //The carry bit
    *c_i = (a_i - b_i - epsilon_prime);
    if (0xFFFF == b_i) {
        epsilon = 1;
    } else {
        if (a_i < b_i) {
            epsilon = 1;
        } else {
            epsilon = 0;
        }
    }
	
    return epsilon;
}

/**
 * Multiprecision addition c.f. Alg. 2.5
 * Input: a, b \in [0,2^{Wt})
 * Output: (epsilon, c) where c = a + b mod 2^{Wt} and epsilon is the carry bit
 */
uint16_t add_mp_elements(uint16_t * pfe_c, uint16_t * pfe_a, uint16_t * pfe_b, uint8_t wordlength) {
    uint16_t epsilon; //The carry bit
    int i; // index for loop
	
    //1.
    epsilon = add_word(&pfe_c[0], pfe_a[0], pfe_b[0], 0);
    //2.
	
    for (i = 1; i < wordlength; i++) {
        epsilon = add_word(&pfe_c[i], pfe_a[i], pfe_b[i], epsilon);
    }
	
    //3.
    return epsilon;
}

/**
 * Multiprecision subtraction c.f. Alg. 2.6
 * Input: a, b \in [0,2^{Wt})
 * Output: (epsilon, c) where c = a - b mod 2^{Wt} and epsilon is the borrow bit
 */
uint16_t subtract_mp_elements(uint16_t * pfe_c, uint16_t * pfe_a, uint16_t * pfe_b, uint8_t wordlength) {
    uint16_t epsilon; //The carry bit
    int i; // index for loop
    
    //1.
    epsilon = subtract_word(&pfe_c[0], pfe_a[0], pfe_b[0], 0);
    //2.
	
    for (i = 1; i < wordlength; i++) {
		
        epsilon = subtract_word(&pfe_c[i], pfe_a[i], pfe_b[i], epsilon);
    }
	
    //3.
    return epsilon;
}

/**
 * Addition in F_p c.f. Alg. 2.7
 * Input: a, b \in [0,p-1)
 * Output: c = a + b mod p
 */
void add_mod_p(uint16_t * c, uint16_t * a, uint16_t * b, uint16_t * p, uint8_t wordlength) {
    uint8_t epsilon;
    
	
    //1.
    epsilon = add_mp_elements(c, a, b, wordlength);
    //2.
    if (1 == epsilon) {
        subtract_mp_elements(c, c, p, wordlength);
    } else {
        if (1 == compare_mp_elements(c, p, wordlength)) {
            subtract_mp_elements(c, c, p, wordlength);
        }
    }
}

/**
 * subtraction in F_p c.f. Alg. 2.8
 * Input: a, b \in [0,p-1)
 * Output: c = a - b mod p
 */
void subtract_mod_p(uint16_t * c, uint16_t * a, uint16_t * b, uint16_t * p, uint8_t wordlength) {
    uint8_t epsilon; //The carry bit
    //1.
    epsilon = subtract_mp_elements(c, a, b, wordlength);
    if (1 == epsilon) {
        add_mp_elements(c, c, p, wordlength);
    }
}

/**
 * Sets a bn to zero
 */
void set_to_zero(uint16_t * c, uint8_t wordlength) {
    int i;
    for (i = 0; i < wordlength; i++) {
        c[i] = 0x0000;
    }
}

/**
 * Multiply two single words into a double word
 * Input: a,b words
 * Output: uv one 2-word
 */
void multiply_words(uint16_t a, uint16_t b, uint16_t * uv) {
    uint32_t uv_32;
    uint16_t u,v;
	
	
    uv_32 = ((uint32_t) a) * ((uint32_t) b);
    u = (uint16_t) (uv_32 >> 16);
    v = (uint16_t) uv_32;
	
    uv[1] = u;
    uv[0] = v;
	
}

/**
 * Multiply two single words into a double word
 * Input: a,b words
 * Output: uv one 2-word
 */
void multiply_words_2(uint16_t a, uint16_t b, uint16_t * uv) {
    uint16_t a0, a1, b0, b1;
    uint16_t t[2];
    uint16_t s[2];
    uint16_t m;
    uint16_t borrow;
	
    a0 = (a & 0xFF00) >> 8;
    a1 = a & 0x00FF;
    b0 = (b & 0xFF00) >> 8;
    b1 = b & 0x00FF;
	
    //1.
    m = a1 * b1;
    t[0] = m & 0x00FF;
    borrow = (m & 0xFF00) >> 8;
	
    //2.
    m = a0 * b1 + borrow;
    t[0] ^= ((m & 0x00FF) << 8);
    t[1] = ((m & 0xFF00) >> 8);
	
    //3.
    m = a1*b0;
    s[0] = (m & 0x00FF) << 8;
    borrow = (m & 0xFF00) >> 8;
	
    //4.
    m = a0 * b0 + borrow;
    s[1] = m;
	
	
    //5.
    //Add two rows s ,t
    add_mp_elements(uv, s, t, 2);
	
}

/**
 * Multiprecision Multiplication c.f. Alg. 2.9
 * Input: a, b \in [0,p-1]
 * Output: c = a*b
 */
void multiply_mp_elements(uint16_t * c, uint16_t * a, uint16_t * b, uint8_t
        wordlength) {
    uint16_t UV[2];
    uint16_t temp1[2];
    uint16_t temp2[2];
	
    int i, j;
    //1. Set c[i] = 0 for 0 \leq i \leq wordlength-1
    set_to_zero(c, 2 * wordlength);
    //2.
    set_to_zero(UV, 2);
    for (i = 0; i < wordlength; i++) {
        UV[1] = 0;
        for (j = 0; j < wordlength; j++) {
            //UV = c[i+j] + a[i]*b[j] + UV[0];
            temp2[0] = UV[1];
            temp2[1] = 0x0000;
            multiply_words(a[i], b[j], UV);
            temp1[0] = c[i + j];
            temp1[1] = 0x0000;
            add_mp_elements(UV, UV, temp1, 2);
            add_mp_elements(UV, UV, temp2, 2);
            c[i + j] = UV[0];
        }
        c[i + wordlength] = UV[1];
    }
}

/**
 * Multiprecision Multiplication c.f. Alg. 2.9
 * Input: a, b \in [0,p-1]
 * Output: c = a*b
 */
void multiply_mp_elements2(uint16_t * c, uint16_t * a, uint8_t wordlength_a, uint16_t * b, uint8_t wordlength_b) {
    uint16_t UV[2];
    uint16_t temp1[2];
    uint16_t temp2[2];
	
    int i, j;
    //1. Set c[i] = 0 for 0 \leq i \leq wordlength-1
    set_to_zero(c, wordlength_a + wordlength_b);
    //2.
    set_to_zero(UV, 2);
    for (i = 0; i < wordlength_b; i++) {
        UV[1] = 0;
        for (j = 0; j < wordlength_a; j++) {
            //UV = c[i+j] + a[i]*b[j] + UV[0];
            temp2[0] = UV[1];
            temp2[1] = 0x0000;
            multiply_words(a[i], b[j], UV);
            temp1[0] = c[i + j];
            temp1[1] = 0x0000;
            add_mp_elements(UV, UV, temp1, 2);
            add_mp_elements(UV, UV, temp2, 2);
            c[i + j] = UV[0];
        }
        c[i + wordlength_b] = UV[1];
    }
}



/**
 * Compares two big nums
 * Returns 1 if a >= b 0 otherwise
 */
uint8_t compare_mp_elements(uint16_t * a, uint16_t * b, uint8_t wordlength) {
    int i;
	
    for (i = wordlength-1; i > -1; i--) {
        //if (a[i] > b[i]) {
        //    return 1;
        //}
        if (a[i] < b[i]) {
            return 0;
        }
        if (a[i] > b[i]) {
            return 1;
        }
    }
    //The elements are equal
    return 1;
}




/**
 Multiply by a power of b
 * out = a*b^k
 */
void mult_by_power_of_b(uint16_t * out, uint16_t wordlength_out, uint16_t * a,
						uint16_t wordlength_a, uint16_t k) {
    int i = 0;
    //initialize out
    set_to_zero(out, wordlength_out);
	
    while(i + k < wordlength_out) {
        if(i<wordlength_a) {
            out[i+k] = a[i];
        }
        i++;
    }
}

void mod_pow_of_b(uint16_t * out, uint16_t wordlength_out, uint16_t * a,
				  uint16_t wordlength_a, uint16_t k){
    int i;
	
    while(i < wordlength_out) {
        if(i < wordlength_a) {
            out[i] = a[i];
        }
        else {
            out[i] = 0;
        }
        i++;
    }
}

/*
 Divide by a power of b
 */
void div_by_power_of_b(uint16_t * out, uint16_t out_len, uint16_t * a, uint16_t a_len, uint16_t k) {
    int i;
    //initialize z_div
    set_to_zero(out, out_len);
	
	
    if (out_len + 1 > a_len - k) {
        for (i = 0; i < out_len; i++) {
			if(k+i<a_len) {
				out[i] = a[k+i];
			}
        }
    }
	
}

/**
 * @param c An output BigNum such that c = a * b
 * @param a A 16-bit unsigned integer
 * @param b A BigNum of size wordlength_b in 16-bit words.
 * @param wordlength_b
 */
void multiply_sp_by_mp_element(uint16_t * c, uint16_t a, uint16_t * b,
							   uint16_t wordlength_b) {
    uint32_t uv;
    uint16_t u;
    uint16_t v;
    uint16_t carry;
	
    int j;
    //1. Set c[i] = 0 for 0 \leq i \leq wordlength-1
    set_to_zero(c, wordlength_b + 1);
    //2. Perform paper and pencil multiplication
    uv = 0;
    carry = 0;
    for (j = 0; j < wordlength_b; j++) {
        uv = ((uint32_t) a) * ((uint32_t) b[j]) + ((uint32_t) carry);
        u = (uint16_t) (uv >> 16);
        v = (uint16_t) uv;
        c[j] = v;
        carry = u;
    }
    c[wordlength_b] = carry;
}

int are_mp_equal(uint16_t * a, uint16_t * b, uint8_t wordlength){
    int i =0;
    int answ = 1;
	
    while((1 == answ) && (i<wordlength)){
        if(a[i] != b[i]) {
            answ = 0;
        }
        i++;
    }
    return answ;
}

void copy_mp(uint16_t * out, uint16_t * in, int wordlength){
    int i;
    for(i = 0; i< wordlength; i++) {
        out[i] = in[i];
    }
}

int ith_bit(uint16_t e, int i){
    uint16_t mask;
    mask = 0x0001 << i;
    mask = e & mask;
    if(0x0000 == mask){
        return 0;
    } else {
        return 1;
    }
}

int bit_length(uint16_t e){
    int i = 15;
    int found_one = 0;
    while(0 == found_one){
        if(1 == ith_bit(e, i)) {
            found_one = 1;
        } else {
            i--;
        }
    }
    return i;
}

int mp_bit_length(uint16_t * e, uint16_t wordlength){
    int i = wordlength - 1;
    int length;
    int last_non_zero_word = -1;
    while((i>-1)&&(last_non_zero_word < 0)){
        if(0!= e[i]){
            last_non_zero_word = i;
        }
        i--;
    }
    length = 16*last_non_zero_word + bit_length(e[last_non_zero_word]);
    return length;
}

int mp_ith_bit(uint16_t * e, int i){
    uint16_t word;
    uint16_t word_bit;
	
    word = (int) i/16;
    word_bit = ith_bit(e[word],i - word *16);
    return word_bit;
}

int mp_non_zero_words(uint16_t * e, uint16_t wordlength){
    int i = wordlength - 1;
    int last_non_zero_word = -1;
    while((i>-1)&&(last_non_zero_word < 0)){
        if(0!= e[i]){
            last_non_zero_word = i;
        }
        i--;
    }
    return last_non_zero_word;
}

/**
 * Multiplication in F_p
 * Input: a, b \in [0,p-1)
 * Output: c = a * b mod p
 */
void multiply_mod_p(uint16_t * c, uint16_t * a, uint16_t * b, uint16_t * p,
        uint8_t wordlength){
    multiply_mp_elements(_tmpglobal_ab, a, b, wordlength);
    divide_mp_elements(_tmpglobal_q, c, _tmpglobal_ab, 2*wordlength, p, wordlength);
}

/*
 Returns floor((x1*(0xFFFF) + x0)/y)
 */
uint16_t divide_2_word_by_1_word(uint16_t x1, uint16_t x0, uint16_t y) {
    uint32_t result;
    if (0 != y) {
        //result = (uint32_t ) ((uint32_t ) x1) * (0x00010000);
        result = (((uint32_t) x1) << 16);
        //result = result | x0;
        result = result + (uint32_t) x0;
        result = (result) / ((uint32_t) y);
        return result;
    } else {
        return 0;
    }
	
}

/**
 * Multiprecision Division c.f. Alg. 14.20 Handbook of Applied Crypto (Menezes, et.al.)
 * Input: x = (x_n . . . x_1 x_0)_b, y = (y_t . . . y_1 y_0)_b with n >= t > 1, y_t != 0
 * Output: q = (q_{n-t} . . . q_1 q_0)_b, r = (r_t . . . r_1 r_0)_b
 * such that x = qy +r, 0 \leq r < y.
 */
void divide_mp_elements(uint16_t * q, uint16_t * r, uint16_t * x_in, int n,
        uint16_t * y, int t) {
    int i, j, k;
    //For step 3.2
    //uint64_t ls, rs;
    uint16_t temp_ls[2];
    //uint16_t temp_rs[2];
    uint16_t ls[3];
    uint16_t rs[3];
	
    //Just for testing
    uint16_t temp;

    //0) copy x_in to x
    for (i = 0; i < n; i++)
        _tmpglobal_x[i] = x_in[i];
	
    //1)
    for (j = 0; j <= n - t; j++) {
        q[j] = 0x0000;
    }
    //2)
    mult_by_power_of_b(_tmpglobal_ybnt, n, y, t, n - t);
	
    while (1 == compare_mp_elements(_tmpglobal_x, _tmpglobal_ybnt, (int) n)) {
        q[n - t] = q[n - t] + 1;
        subtract_mp_elements(_tmpglobal_x, _tmpglobal_x, _tmpglobal_ybnt, n);
    }
    //3)
    for (i = n - 1; i > t - 1; i--) { //<--- check index here
        temp = _tmpglobal_x[i] - y[t - 1];
        //3.1)
        if (0 == temp) {
            q[i - t] = 0xFFFF; //<--- check index here
        } else {
            q[i - t] = divide_2_word_by_1_word(_tmpglobal_x[i], _tmpglobal_x[i - 1], y[t - 1]);
        }
        //3.2)
        //ls = ((uint64_t) q[i - t])*((((uint64_t) y[t - 1]) << 16) + ((uint64_t) y[t - 2]));
        temp_ls[0] = y[t - 2];
        temp_ls[1] = y[t - 1];
        multiply_sp_by_mp_element(ls, q[i - t], temp_ls, 2);
		
        //rs = (((uint64_t) x[i]) << 32) + (((uint64_t) x[i - 1]) << 16) + ((uint64_t) x[i - 2]);
        rs[0] = _tmpglobal_x[i - 2];
        rs[1] = _tmpglobal_x[i - 1];
        rs[2] = _tmpglobal_x[i];
		
        //if((0xBCD3 != rs[2]) && (0x78FC != rs[1]) && (0x1917 != rs[0])) {
		
        //while (ls > rs) {
        while (0 == compare_mp_elements(rs, ls, 3)) {
            q[i - t] = q[i - t] - 1;
            //ls = ((uint64_t) q[i - t])*((((uint64_t) y[t - 1]) << 16)+ ((uint64_t) y[t - 2]));
            multiply_sp_by_mp_element(ls, q[i - t], temp_ls, 2);
        }
        //3.3)
        mult_by_power_of_b(_tmpglobal_ybit, n, y, t, i - t);
        multiply_sp_by_mp_element(_tmpglobal_qitybit, q[i - t], _tmpglobal_ybit, n);
        //3.3 + 3.4) Last part of 3.3 merged with 3.4 to avoid negative comparisons
        if (0 == compare_mp_elements(_tmpglobal_x, _tmpglobal_qitybit, n)) {
            add_mp_elements(_tmpglobal_x, _tmpglobal_x, _tmpglobal_ybit, n);
            subtract_mp_elements(_tmpglobal_x, _tmpglobal_x, _tmpglobal_qitybit, n);
            q[i - t] = q[i - t] - 1;
        } else {
            subtract_mp_elements(_tmpglobal_x, _tmpglobal_x, _tmpglobal_qitybit, n);
        }
        //4
        for (k = 0; k < t; k++) {
            r[k] = _tmpglobal_x[k];
        }
	}
    //}
	
}

/**
 * Implementation of the left to right modular exponentiation algorithm
 * as described in the HAC book by Menezes et.al.
 *
 * @param A The result of raising g to the power of e
 * @param g an element of Z*_p
 * @param e a multi-precission exponent
 * @param e_legth the wordlength of the multi-precission exponent
 */

void mod_exp(uint16_t * A, uint16_t * g, uint16_t * e, uint16_t e_length,
        uint16_t * p, uint16_t p_length) {
    int i;
    int t = mp_bit_length(e,e_length);
	
    set_to_zero(A, p_length);
    A[0] = 1;
	
    for (i = t; i >= 0; i--) { // Note, first decrease, then work
        //2.1 A = A*A mod p
        multiply_mod_p(_tmpglobal_temp, A, A, p, p_length);
        copy_mp(A, _tmpglobal_temp, p_length);
        //2.2 If e_i = 1 then A = Mont(A,x_hat)
        if (1 == mp_ith_bit(e, i)) {
            multiply_mod_p(_tmpglobal_temp, A, g, p, p_length);
            copy_mp(A, _tmpglobal_temp, p_length);
        }
    }
	
    //3.
    //copy_mp(out,A,12); // no need to copy the result, alreade stored in A
}

void test_rsa_encrypt(){
    mod_exp(ciphertext, plaintext, e, 1, n, KEYLEN);
}

int main (void) {
    /* see above comment about generating these */
    n[0] = 0xab78; n[1] = 0xafba; n[2] = 0x88e7; n[3] = 0x496d;
    e[0] = 0x0001; e[1] = 0x0001; // e = 65537

    plaintext[0] = 0x4d65; // Me
    plaintext[1] = 0x6d65; // me
    plaintext[2] = 0x6e74; // nt
    plaintext[3] = 0x6f73; // os

    int cnt;
    printf("Plaintext:\n\r");
    for(cnt = 0; cnt < KEYLEN; ++cnt)
      printf("0x%08X\n\r", plaintext[cnt]);

    test_rsa_encrypt();

    printf("Cipertext:\n\r");
    for(cnt = 0; cnt < KEYLEN; ++cnt)
      printf("0x%08X\n\r", ciphertext[cnt]);

    return 11;
}
