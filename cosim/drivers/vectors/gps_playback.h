//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// This file is auto-generated for test: gps. Do not modify!!!
//
// Generated on: Feb 16 2021 18:53:57
//************************************************************************
#ifndef gps_playback_H
#define gps_playback_H

#ifndef PLAYBACK_CMD_H
#define PLAYBACK_CMD_H
// Write to : <physicalAdr> <writeData>
#define WRITE__CMD  1
// Read and compare: <physicalAdr> <Data2Compare>
#define RDnCMP_CMD  2
// Read and spin until match : <physicalAdr> <Data2Match> <mask> <timeout>
#define RDSPIN_CMD  3

#define WRITE__CMD_SIZE  3
#define RDnCMP_CMD_SIZE  3
#define RDSPIN_CMD_SIZE  5
#endif

// gps command sequences to playback
uint64_t gps_playback[] = { 
	  WRITE__CMD, 0x70090030, 0x0000000000000000 // 1
	, WRITE__CMD, 0x70090030, 0x0000000000000001 // 2
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 3
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 4
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 5
	, RDnCMP_CMD, 0x70090008, 0x0000000000001907 // 6
	, RDnCMP_CMD, 0x70090010, 0x2006ce8f5af7c2e5 // 7
	, RDnCMP_CMD, 0x70090018, 0x3d9294864adf8e49 // 8
	, RDnCMP_CMD, 0x70090020, 0x565576e441e60edf // 9
	, RDnCMP_CMD, 0x70090028, 0x5dfcdf1c41c6436b // 10
	, WRITE__CMD, 0x70090030, 0x0000000000000002 // 11
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 12
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 13
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 14
	, RDnCMP_CMD, 0x70090008, 0x0000000000001c87 // 15
	, RDnCMP_CMD, 0x70090010, 0x3495edddbd79f008 // 16
	, RDnCMP_CMD, 0x70090018, 0x52d096b63050a593 // 17
	, RDnCMP_CMD, 0x70090020, 0x145833c3c7004f68 // 18
	, RDnCMP_CMD, 0x70090028, 0x135a5f719f1ecbc5 // 19
	, WRITE__CMD, 0x70090030, 0x0000000000000003 // 20
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 21
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 22
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 23
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e47 // 24
	, RDnCMP_CMD, 0x70090010, 0x3edc7c74cebee97e // 25
	, RDnCMP_CMD, 0x70090018, 0xe57197ae0d17307e // 26
	, RDnCMP_CMD, 0x70090020, 0xe01663e6dfc9367b // 27
	, RDnCMP_CMD, 0x70090028, 0x48aeb2c186b9ba48 // 28
	, WRITE__CMD, 0x70090030, 0x0000000000000004 // 29
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 30
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 31
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 32
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f27 // 33
	, RDnCMP_CMD, 0x70090010, 0x3bf8b4a0775d65c5 // 34
	, RDnCMP_CMD, 0x70090018, 0xbea1172213b4fa88 // 35
	, RDnCMP_CMD, 0x70090020, 0x939a8e37fda11b18 // 36
	, RDnCMP_CMD, 0x70090028, 0x4598b12ca5508e46 // 37
	, WRITE__CMD, 0x70090030, 0x0000000000000005 // 38
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 39
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 40
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 41
	, RDnCMP_CMD, 0x70090008, 0x00000000000012d8 // 42
	, RDnCMP_CMD, 0x70090010, 0x396ad0ca2baca398 // 43
	, RDnCMP_CMD, 0x70090018, 0x134957641ce51ff3 // 44
	, RDnCMP_CMD, 0x70090020, 0xa6806b2dbc44d608 // 45
	, RDnCMP_CMD, 0x70090028, 0x11638e7d6539b975 // 46
	, WRITE__CMD, 0x70090030, 0x0000000000000006 // 47
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 48
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 49
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 50
	, RDnCMP_CMD, 0x70090008, 0x0000000000001968 // 51
	, RDnCMP_CMD, 0x70090010, 0x3823e2ff05d440b6 // 52
	, RDnCMP_CMD, 0x70090018, 0xc5bd77471b4ded4e // 53
	, RDnCMP_CMD, 0x70090020, 0x571223360fb8b6da // 54
	, RDnCMP_CMD, 0x70090028, 0xb361ee3d0a6f0b44 // 55
	, WRITE__CMD, 0x70090030, 0x0000000000000007 // 56
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 57
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 58
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 59
	, RDnCMP_CMD, 0x70090008, 0x00000000000012cf // 60
	, RDnCMP_CMD, 0x70090010, 0x38877be592e83121 // 61
	, RDnCMP_CMD, 0x70090018, 0xaec7675698999410 // 62
	, RDnCMP_CMD, 0x70090020, 0xff6da72af6d826f7 // 63
	, RDnCMP_CMD, 0x70090028, 0x86282a26ac78dfa7 // 64
	, WRITE__CMD, 0x70090030, 0x0000000000000008 // 65
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 66
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 67
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 68
	, RDnCMP_CMD, 0x70090008, 0x0000000000001963 // 69
	, RDnCMP_CMD, 0x70090010, 0x38d53768d97609ea // 70
	, RDnCMP_CMD, 0x70090018, 0x1b7a6f5e5973a8bf // 71
	, RDnCMP_CMD, 0x70090020, 0x69694e5c1052f079 // 72
	, RDnCMP_CMD, 0x70090028, 0x616c955c38941846 // 73
	, WRITE__CMD, 0x70090030, 0x0000000000000009 // 74
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 75
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 76
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 77
	, RDnCMP_CMD, 0x70090008, 0x0000000000001cb5 // 78
	, RDnCMP_CMD, 0x70090010, 0x38fc112e7cb9158f // 79
	, RDnCMP_CMD, 0x70090018, 0xc1a4eb5a3986b6e8 // 80
	, RDnCMP_CMD, 0x70090020, 0x4c684aa24832147a // 81
	, RDnCMP_CMD, 0x70090028, 0x6bd67e8b9a91a106 // 82
	, WRITE__CMD, 0x70090030, 0x000000000000000a // 83
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 84
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 85
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 86
	, RDnCMP_CMD, 0x70090008, 0x0000000000001a25 // 87
	, RDnCMP_CMD, 0x70090010, 0x38e8820d2e5e9bbd // 88
	, RDnCMP_CMD, 0x70090018, 0x2ccba95809fc39c3 // 89
	, RDnCMP_CMD, 0x70090020, 0x66df782009ebee4a // 90
	, RDnCMP_CMD, 0x70090028, 0x27d09a4cc1a2b36c // 91
	, WRITE__CMD, 0x70090030, 0x000000000000000b // 92
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 93
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 94
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 95
	, RDnCMP_CMD, 0x70090008, 0x0000000000001d16 // 96
	, RDnCMP_CMD, 0x70090010, 0x38e2cb9c872d5ca4 // 97
	, RDnCMP_CMD, 0x70090018, 0x5a7c085911c17e56 // 98
	, RDnCMP_CMD, 0x70090020, 0x79ba87eac2c745cd // 99
	, RDnCMP_CMD, 0x70090028, 0x5cf920524ede10cd // 100
	, WRITE__CMD, 0x70090030, 0x000000000000000c // 101
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 102
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 103
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 104
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f43 // 105
	, RDnCMP_CMD, 0x70090010, 0x38e7ef545394bf28 // 106
	, RDnCMP_CMD, 0x70090018, 0xe127d8d99ddfdd9c // 107
	, RDnCMP_CMD, 0x70090020, 0xc643bc27e284b05a // 108
	, RDnCMP_CMD, 0x70090028, 0x4155a750fd86c6d3 // 109
	, WRITE__CMD, 0x70090030, 0x000000000000000d // 110
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 111
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 112
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 113
	, RDnCMP_CMD, 0x70090008, 0x0000000000001fa5 // 114
	, RDnCMP_CMD, 0x70090010, 0x38e57d3039c84eee // 115
	, RDnCMP_CMD, 0x70090018, 0xbc8a3099dbd08c79 // 116
	, RDnCMP_CMD, 0x70090020, 0xbe46869eff9ab3d2 // 117
	, RDnCMP_CMD, 0x70090028, 0xfd0898cda314b2d4 // 118
	, WRITE__CMD, 0x70090030, 0x000000000000000e // 119
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 120
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 121
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 122
	, RDnCMP_CMD, 0x70090008, 0x0000000000001fd6 // 123
	, RDnCMP_CMD, 0x70090010, 0x38e434020ce6360d // 124
	, RDnCMP_CMD, 0x70090018, 0x925cc4b9f8d7248b // 125
	, RDnCMP_CMD, 0x70090020, 0xe560c454bf82b948 // 126
	, RDnCMP_CMD, 0x70090028, 0x7621c2e043639a06 // 127
	, WRITE__CMD, 0x70090030, 0x000000000000000f // 128
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 129
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 130
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 131
	, RDnCMP_CMD, 0x70090008, 0x0000000000001fef // 132
	, RDnCMP_CMD, 0x70090010, 0x38e4909b16710a7c // 133
	, RDnCMP_CMD, 0x70090018, 0x0537bea9e954f0f2 // 134
	, RDnCMP_CMD, 0x70090020, 0x4dcd60e3a7d2469c // 135
	, RDnCMP_CMD, 0x70090028, 0x4070fc3301a9d946 // 136
	, WRITE__CMD, 0x70090030, 0x0000000000000010 // 137
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 138
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 139
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 140
	, RDnCMP_CMD, 0x70090008, 0x0000000000001ff3 // 141
	, RDnCMP_CMD, 0x70090010, 0x38e4c2d79b3a9444 // 142
	, RDnCMP_CMD, 0x70090018, 0xce8203a1e1951ace // 143
	, RDnCMP_CMD, 0x70090020, 0x6da0cef37e01089e // 144
	, RDnCMP_CMD, 0x70090028, 0x2625e8056501aa4b // 145
	, WRITE__CMD, 0x70090030, 0x0000000000000011 // 146
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 147
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 148
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 149
	, RDnCMP_CMD, 0x70090008, 0x0000000000001370 // 150
	, RDnCMP_CMD, 0x70090010, 0x38e4ebf1dd9f5b58 // 151
	, RDnCMP_CMD, 0x70090018, 0xab58dd25e5f5efd0 // 152
	, RDnCMP_CMD, 0x70090020, 0x6e43b1966f3612ea // 153
	, RDnCMP_CMD, 0x70090028, 0x62db4dc5d135ebdb // 154
	, WRITE__CMD, 0x70090030, 0x0000000000000012 // 155
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 156
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 157
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 158
	, RDnCMP_CMD, 0x70090008, 0x00000000000019bc // 159
	, RDnCMP_CMD, 0x70090010, 0x38e4ff62fecdbcd6 // 160
	, RDnCMP_CMD, 0x70090018, 0x99b5b267e7c5955f // 161
	, RDnCMP_CMD, 0x70090020, 0xffec452494e4fe4e // 162
	, RDnCMP_CMD, 0x70090028, 0x449e69e78d96c9b4 // 163
	, WRITE__CMD, 0x70090030, 0x0000000000000013 // 164
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 165
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 166
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 167
	, RDnCMP_CMD, 0x70090008, 0x0000000000001cda // 168
	, RDnCMP_CMD, 0x70090010, 0x38e4f52b6f64cf11 // 169
	, RDnCMP_CMD, 0x70090018, 0x80c305c6e6dda818 // 170
	, RDnCMP_CMD, 0x70090020, 0x162ab58693395f7b // 171
	, RDnCMP_CMD, 0x70090028, 0x114f24a89aee5ba8 // 172
	, WRITE__CMD, 0x70090030, 0x0000000000000014 // 173
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 174
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 175
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 176
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e69 // 177
	, RDnCMP_CMD, 0x70090010, 0x38e4f00fa7b076f2 // 178
	, RDnCMP_CMD, 0x70090018, 0x0c785e166651b6bb // 179
	, RDnCMP_CMD, 0x70090020, 0x18bd6044e9cf1968 // 180
	, RDnCMP_CMD, 0x70090028, 0x68530aadfe9afa87 // 181
	, WRITE__CMD, 0x70090030, 0x0000000000000015 // 182
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 183
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 184
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 185
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f30 // 186
	, RDnCMP_CMD, 0x70090010, 0x38e4f29dc3da2a03 // 187
	, RDnCMP_CMD, 0x70090018, 0xca25f3fe2617b9ea // 188
	, RDnCMP_CMD, 0x70090020, 0x707fd8c0437e75aa // 189
	, RDnCMP_CMD, 0x70090028, 0x4a24307d9c973c50 // 190
	, WRITE__CMD, 0x70090030, 0x0000000000000016 // 191
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 192
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 193
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 194
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f9c // 195
	, RDnCMP_CMD, 0x70090010, 0x38e4f3d4f1ef047b // 196
	, RDnCMP_CMD, 0x70090018, 0x290b250a0634be42 // 197
	, RDnCMP_CMD, 0x70090020, 0x9302e1ca80749923 // 198
	, RDnCMP_CMD, 0x70090028, 0x4cee302aaa4ffa13 // 199
	, WRITE__CMD, 0x70090030, 0x0000000000000017 // 200
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 201
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 202
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 203
	, RDnCMP_CMD, 0x70090008, 0x000000000000119e // 204
	, RDnCMP_CMD, 0x70090010, 0x38e4f37068f59347 // 205
	, RDnCMP_CMD, 0x70090018, 0x589c4e7016253d96 // 206
	, RDnCMP_CMD, 0x70090020, 0xd307dbf803b2d295 // 207
	, RDnCMP_CMD, 0x70090028, 0xd2154a5ff6a284d9 // 208
	, WRITE__CMD, 0x70090030, 0x0000000000000018 // 209
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 210
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 211
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 212
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e34 // 213
	, RDnCMP_CMD, 0x70090010, 0x38e4f3222478d8d9 // 214
	, RDnCMP_CMD, 0x70090018, 0x6057fbcd1e2dfc7c // 215
	, RDnCMP_CMD, 0x70090020, 0x7a48c7e197d02a99 // 216
	, RDnCMP_CMD, 0x70090028, 0x0074244e99210e8f // 217
	, WRITE__CMD, 0x70090030, 0x0000000000000019 // 218
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 219
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 220
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 221
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f1e // 222
	, RDnCMP_CMD, 0x70090010, 0x38e4f30b023e7d16 // 223
	, RDnCMP_CMD, 0x70090018, 0x7c3221139a299c89 // 224
	, RDnCMP_CMD, 0x70090020, 0xcd5822e30884b147 // 225
	, RDnCMP_CMD, 0x70090028, 0x33b98317fdc01b68 // 226
	, WRITE__CMD, 0x70090030, 0x000000000000001a // 227
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 228
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 229
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 230
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f8b // 231
	, RDnCMP_CMD, 0x70090010, 0x38e4f31f911d2ff1 // 232
	, RDnCMP_CMD, 0x70090018, 0xf200cc7cd82bacf3 // 233
	, RDnCMP_CMD, 0x70090020, 0xe217d0b4d1ed53fc // 234
	, RDnCMP_CMD, 0x70090028, 0xf0c10ac4a71a4e05 // 235
	, WRITE__CMD, 0x70090030, 0x000000000000001b // 236
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 237
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 238
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 239
	, RDnCMP_CMD, 0x70090008, 0x0000000000001fc1 // 240
	, RDnCMP_CMD, 0x70090010, 0x38e4f315d88c8682 // 241
	, RDnCMP_CMD, 0x70090018, 0x3519bacb792ab4ce // 242
	, RDnCMP_CMD, 0x70090020, 0x32952ef7bca15a03 // 243
	, RDnCMP_CMD, 0x70090028, 0x7cff8786f30e5601 // 244
	, WRITE__CMD, 0x70090030, 0x000000000000001c // 245
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 246
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 247
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 248
	, RDnCMP_CMD, 0x70090008, 0x0000000000001fe4 // 249
	, RDnCMP_CMD, 0x70090010, 0x38e4f310fc44523b // 250
	, RDnCMP_CMD, 0x70090018, 0xd6950190a9aa38d0 // 251
	, RDnCMP_CMD, 0x70090020, 0x3d91ba3d23057c85 // 252
	, RDnCMP_CMD, 0x70090028, 0x36fc9b9120145d8e // 253
	, WRITE__CMD, 0x70090030, 0x000000000000001d // 254
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 255
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 256
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 257
	, RDnCMP_CMD, 0x70090008, 0x00000000000012bc // 258
	, RDnCMP_CMD, 0x70090010, 0x38e4f3126e203867 // 259
	, RDnCMP_CMD, 0x70090018, 0x27535c3d41ea7edf // 260
	, RDnCMP_CMD, 0x70090020, 0xa9fed46eb5165c23 // 261
	, RDnCMP_CMD, 0x70090028, 0x47ef77ed6c2105ea // 262
	, WRITE__CMD, 0x70090030, 0x000000000000001e // 263
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 264
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 265
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 266
	, RDnCMP_CMD, 0x70090008, 0x000000000000195a // 267
	, RDnCMP_CMD, 0x70090010, 0x38e4f31327120d49 // 268
	, RDnCMP_CMD, 0x70090018, 0x5fb072ebb5ca5dd8 // 269
	, RDnCMP_CMD, 0x70090020, 0x7e31c3bf11044a3d // 270
	, RDnCMP_CMD, 0x70090028, 0xe6a1f9d719b1758f // 271
	, WRITE__CMD, 0x70090030, 0x000000000000001f // 272
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 273
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 274
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 275
	, RDnCMP_CMD, 0x70090008, 0x0000000000001ca9 // 276
	, RDnCMP_CMD, 0x70090010, 0x38e4f313838b17de // 277
	, RDnCMP_CMD, 0x70090018, 0x63c1e580cfda4c5b // 278
	, RDnCMP_CMD, 0x70090020, 0x21478ed2a2d03281 // 279
	, RDnCMP_CMD, 0x70090028, 0xea1b724d420e9f97 // 280
	, WRITE__CMD, 0x70090030, 0x0000000000000020 // 281
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 282
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 283
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 284
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e50 // 285
	, RDnCMP_CMD, 0x70090010, 0x38e4f313d1c79a95 // 286
	, RDnCMP_CMD, 0x70090018, 0xfdf92e3572d2449a // 287
	, RDnCMP_CMD, 0x70090020, 0x90c649c70da56b4e // 288
	, RDnCMP_CMD, 0x70090028, 0x525fa68a6c843654 // 289
	, WRITE__CMD, 0x70090030, 0x0000000000000021 // 290
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 291
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 292
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 293
	, RDnCMP_CMD, 0x70090008, 0x0000000000001f2c // 294
	, RDnCMP_CMD, 0x70090010, 0x38e4f313f8e1dc30 // 295
	, RDnCMP_CMD, 0x70090018, 0x32e54befac5640fa // 296
	, RDnCMP_CMD, 0x70090020, 0x6edfa0e0289a302d // 297
	, RDnCMP_CMD, 0x70090028, 0x4ad2b5017b817ceb // 298
	, WRITE__CMD, 0x70090030, 0x0000000000000022 // 299
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 300
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 301
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 302
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e5b // 303
	, RDnCMP_CMD, 0x70090010, 0x38e4f313ec72ff62 // 304
	, RDnCMP_CMD, 0x70090018, 0xd56b7902c31442ca // 305
	, RDnCMP_CMD, 0x70090020, 0xae3356b0b4970e44 // 306
	, RDnCMP_CMD, 0x70090028, 0x88de2487e19724b0 // 307
	, WRITE__CMD, 0x70090030, 0x0000000000000023 // 308
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 309
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 310
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 311
	, RDnCMP_CMD, 0x70090008, 0x00000000000012e1 // 312
	, RDnCMP_CMD, 0x70090010, 0x38e4f313e63b6ecb // 313
	, RDnCMP_CMD, 0x70090018, 0xa6ac607474b543d2 // 314
	, RDnCMP_CMD, 0x70090020, 0x8750d90e2d97d20c // 315
	, RDnCMP_CMD, 0x70090028, 0x935f662503862261 // 316
	, WRITE__CMD, 0x70090030, 0x0000000000000024 // 317
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 318
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 319
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 320
	, RDnCMP_CMD, 0x70090008, 0x0000000000001974 // 321
	, RDnCMP_CMD, 0x70090010, 0x38e4f313e31fa61f // 322
	, RDnCMP_CMD, 0x70090018, 0x1f4feccf2f65c35e // 323
	, RDnCMP_CMD, 0x70090020, 0xdcfe3b9777794f2e // 324
	, RDnCMP_CMD, 0x70090028, 0x58a27aeed5dae14e // 325
	, WRITE__CMD, 0x70090030, 0x0000000000000025 // 326
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 327
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 328
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 329
	, RDnCMP_CMD, 0x70090008, 0x0000000000001e5b // 330
	, RDnCMP_CMD, 0x70090010, 0x38e4f313e18dc275 // 331
	, RDnCMP_CMD, 0x70090018, 0x43be2a92828d8318 // 332
	, RDnCMP_CMD, 0x70090020, 0xb6c2bd8866f78385 // 333
	, RDnCMP_CMD, 0x70090028, 0x6c359b84c73689c9 // 334
	, WRITE__CMD, 0x70090030, 0x0000000000000001 // 335
	, WRITE__CMD, 0x70090000, 0x0000000000000001 // 336
	, WRITE__CMD, 0x70090000, 0x0000000000000000 // 337
	, RDSPIN_CMD, 0x70090000, 0x0000000000000002, 0x2, 0x1f4 // 338
	, RDnCMP_CMD, 0x70090008, 0x0000000000001907 // 339
	, RDnCMP_CMD, 0x70090010, 0x2006ce8f5af7c2e5 // 340
	, RDnCMP_CMD, 0x70090018, 0x3d9294864adf8e49 // 341
	, RDnCMP_CMD, 0x70090020, 0x565576e441e60edf // 342
	, RDnCMP_CMD, 0x70090028, 0x5dfcdf1c41c6436b // 343
};

#define gps_adrBase 0x0070090000
#define gps_adrSize 0x10000
#define gps_size 1105
#define gps_cmdCnt4Single 28
#define gps_totalCommands 343
#endif
