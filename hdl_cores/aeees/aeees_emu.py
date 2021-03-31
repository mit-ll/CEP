#!/usr/bin/python3
#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      aeees_emu.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Helper script for aeees.py
#//
#// Notes:          
#//
#//************************************************************************

import copy

## A(EE)ES software emulator, modified to extract customized per-round
## S-Boxes that incorporate the round keys for the provided AES-128
## key.
#
## The code is derived from
##   https://github.com/bozhu/AES-Python/blob/master/aes.py
## under terms of the following license:
#
# Copyright (C) 2012 Bo Zhu http://about.bozhu.me
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.


Sbox = (
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16,
)


# learnt from http://cs.ucsb.edu/~koc/cs178/projects/JT/aes.c
## used to generate round constants:  input is rcon(x-1), output is rcon(x), with rcon(0) = 0 and rcon(1) = 1
xtime = lambda a: (((a << 1) ^ 0x1B) & 0xFF) if (a & 0x80) else (a << 1)

def text2matrix(text):
    matrix = []
    for i in range(16):
        byte = (text >> (8 * (15 - i))) & 0xFF
        if i % 4 == 0:
            matrix.append([byte])
        else:
            matrix[int(i / 4)].append(byte)
    return matrix


def matrix2text(matrix):
    text = 0
    for i in range(4):
        for j in range(4):
            text |= (matrix[i][j] << (120 - 8 * (4 * i + j)))
    return text


class AEEES:
    def __init__(self, master_key, num_rounds):
        self.rcon = [0] * (num_rounds + 1)
        self.num_rounds = num_rounds
        self.round_keys = text2matrix(master_key)
        # print self.round_keys

        # populate rcon
        self.rcon[1] = 1
        for i in range(2, self.num_rounds + 1):
            self.rcon[i] = xtime( self.rcon[i-1])

        # produce round keys
        for i in range(4, 4 * (self.num_rounds + 1)):
            self.round_keys.append([])
            if i % 4 == 0:
                byte = self.round_keys[i - 4][0]        \
                     ^ Sbox[self.round_keys[i - 1][1]]  \
                     ^ self.rcon[int(i / 4)]
                self.round_keys[i].append(byte)

                for j in range(1, 4):
                    byte = self.round_keys[i - 4][j]    \
                         ^ Sbox[self.round_keys[i - 1][(j + 1) % 4]]
                    self.round_keys[i].append(byte)
            else:
                for j in range(4):
                    byte = self.round_keys[i - 4][j]    \
                         ^ self.round_keys[i - 1][j]
                    self.round_keys[i].append(byte)


    ## used for "Whitebox" mode
    def get_modded_sbox(self, round, offset):
        # combine the S-Box with the 8*offset .. 8*offset-7th bits of
        # the round-th round key.  note that addRoundKey comes
        # *first*, so we're actually permuting the *input* to the
        # S-Box.
        #
        # this function is used internally by wb_encrypt, and it's also used directly
        # to retrieve the data for each "custom" S-Box in the Verilog.

#        print("SBOX[%d,%d]:" % (round, offset))
        nubox = [0] * 256
        for x in range(0, 256):
            nubox[x] = Sbox[x ^ self.round_keys[4*(round-1) + int(offset/4)][offset%4]]

        if( round == self.num_rounds ): ## last round; fold in last round key as well
            ## the last round key is folded in on the *output*, after we InvShiftRows that round key
            lastroundkey = copy.deepcopy( self.round_keys[(4*round) : (4*round+4)])
            self.__inv_shift_rows( lastroundkey )

#            print( dumper.dump(lastroundkey[0]))
#            print( dumper.dump(lastroundkey[0][0]))
            for x in range(0, 256):
#                print("idx %d, %d" % ( int(offset/4), offset%4))
#                print("LRK:  %2.2x going to %2.2x" % (nubox[x], nubox[x] ^ lastroundkey[int(offset/4)][offset%4]))
                nubox[x] = nubox[x] ^ lastroundkey[int(offset/4)][offset%4]

        return nubox
        

    ## perform "whitebox" encryption, as a way of validating that
    ## we're building the boxes correctly.  the intermediate results
    ## of this won't match the basic encryption because ops are
    ## shifted slightly per-round, but the final results should match.
    #
    ## this is coded in a breathtakingly inefficient manner -- no
    ## custom S-boxes are ever memoized -- but you should only be
    ## using this a handful of times anyway, in a scenario where speed
    ## is not of the essence.

    def wb_encrypt(self, plaintext):
        self.plain_state = text2matrix(plaintext)
        answer = []
        answer.append( matrix2text( self.plain_state))

        for i in range(1, self.num_rounds):
            self.__wb_round_encrypt(i, self.plain_state)
            answer.append( matrix2text(self.plain_state))

        self.__wb_sub_bytes( self.num_rounds, self.plain_state)
        self.__shift_rows(self.plain_state)

        answer.append( matrix2text(self.plain_state))
        return answer

    ## this is "stock" encryption, using the single normal S-Box
    def encrypt(self, plaintext):
        self.plain_state = text2matrix(plaintext)
        answer = []

        self.__add_round_key(self.plain_state, self.round_keys[:4])
        answer.append( matrix2text(self.plain_state))

        for i in range(1, self.num_rounds):
            self.__round_encrypt(self.plain_state, self.round_keys[4 * i : 4 * (i + 1)])
            answer.append( matrix2text(self.plain_state))

        self.__sub_bytes(self.plain_state)
        self.__shift_rows(self.plain_state)
        self.__add_round_key(self.plain_state, self.round_keys[(4 * self.num_rounds):])

        answer.append( matrix2text(self.plain_state))
        return answer

    def __add_round_key(self, s, k):
        for i in range(4):
            for j in range(4):
                s[i][j] ^= k[i][j]

    ## identical to __round_encrypt, except that there is no addroundkey step
    def __wb_round_encrypt(self, round, state_matrix):
        self.__wb_sub_bytes(round, state_matrix)
        self.__shift_rows(state_matrix)
        self.__mix_columns(state_matrix)

    def __round_encrypt(self, state_matrix, key_matrix):
        self.__sub_bytes(state_matrix)
        self.__shift_rows(state_matrix)
        self.__mix_columns(state_matrix)
        self.__add_round_key(state_matrix, key_matrix)

    ## effective lunacy; recomputes the whole S-Box every time it's invoked.
    def __wb_sub_bytes(self, round, s):
        for i in range(4):
            for j in range(4):
                pre = s[i][j]
                s[i][j] = self.get_modded_sbox(round, i*4+j)[s[i][j]]
                print("ROUND %2d BYTE %2d [ %2.2x -> %2.2x ]" % (round, i*4+j, pre, s[i][j]))

    def __sub_bytes(self, s):
        for i in range(4):
            for j in range(4):
                s[i][j] = Sbox[s[i][j]]

    def __shift_rows(self, s):
        s[0][1], s[1][1], s[2][1], s[3][1] = s[1][1], s[2][1], s[3][1], s[0][1]
        s[0][2], s[1][2], s[2][2], s[3][2] = s[2][2], s[3][2], s[0][2], s[1][2]
        s[0][3], s[1][3], s[2][3], s[3][3] = s[3][3], s[0][3], s[1][3], s[2][3]

    def __inv_shift_rows(self, s):
        s[0][1], s[1][1], s[2][1], s[3][1] = s[3][1], s[0][1], s[1][1], s[2][1]
        s[0][2], s[1][2], s[2][2], s[3][2] = s[2][2], s[3][2], s[0][2], s[1][2]
        s[0][3], s[1][3], s[2][3], s[3][3] = s[1][3], s[2][3], s[3][3], s[0][3]

    def __mix_single_column(self, a):
        # please see Sec 4.1.2 in The Design of Rijndael
        t = a[0] ^ a[1] ^ a[2] ^ a[3]
        u = a[0]
        a[0] ^= t ^ xtime(a[0] ^ a[1])
        a[1] ^= t ^ xtime(a[1] ^ a[2])
        a[2] ^= t ^ xtime(a[2] ^ a[3])
        a[3] ^= t ^ xtime(a[3] ^ u)

    def __mix_columns(self, s):
        for i in range(4):
            self.__mix_single_column(s[i])


if __name__ == "__main__":
    import sys
    sys.exit("ERROR:  Invoke the tool as aeees.py instead.")
