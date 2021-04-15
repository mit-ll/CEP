//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      srot_playback.h
// Program:        Common Evaluation Platform (CEP)
// Description:    SROT Core unit testbench stimulus
// Notes:
//
//************************************************************************
#ifndef SROT_playback_H
#define SROT_playback_H

#ifndef PLAYBACK_CMD_H
#define PLAYBACK_CMD_H
#define WRITE__CMD  1
#define RDnCMP_CMD  2
#define RDSPIN_CMD  3
#define WRITE__CMD_SIZE  3
#define RDnCMP_CMD_SIZE  3
#define RDSPIN_CMD_SIZE  5
#endif

uint64_t srot_playback[] = {
	   WRITE__CMD,0x70008008, 0x0000000000010008 //           1
	,  RDSPIN_CMD,0x70008000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //           2
	,  RDSPIN_CMD,0x70008008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //           3
	,  WRITE__CMD,0x70008008, 0x0000000000030007 //           4
	,  RDSPIN_CMD,0x70008000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //           5
	,  WRITE__CMD,0x70008008, 0x0123456789abcdef //           6
	,  RDSPIN_CMD,0x70008000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //           7
	,  RDSPIN_CMD,0x70008000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //           8
	,  WRITE__CMD,0x70008008, 0xfedcba9876543210 //           9
	,  RDSPIN_CMD,0x70008000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          10
	,  RDSPIN_CMD,0x70008008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          11
	,  WRITE__CMD,0x70048008, 0x0000000000010008 //          12
	,  RDSPIN_CMD,0x70048000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          13
	,  RDSPIN_CMD,0x70048008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          14
	,  WRITE__CMD,0x70048008, 0x0000000000020007 //          15
	,  RDSPIN_CMD,0x70048000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          16
	,  WRITE__CMD,0x70048008, 0x0123456789abcdef //          17
	,  RDSPIN_CMD,0x70048000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          18
	,  RDSPIN_CMD,0x70048008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          19
	,  WRITE__CMD,0x70058008, 0x0000000000010008 //          20
	,  RDSPIN_CMD,0x70058000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          21
	,  RDSPIN_CMD,0x70058008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          22
	,  WRITE__CMD,0x70058008, 0x0000000000020007 //          23
	,  RDSPIN_CMD,0x70058000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          24
	,  WRITE__CMD,0x70058008, 0x0123456789abcdef //          25
	,  RDSPIN_CMD,0x70058000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          26
	,  RDSPIN_CMD,0x70058008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          27
	,  WRITE__CMD,0x70078008, 0x0000000000010008 //          28
	,  RDSPIN_CMD,0x70078000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          29
	,  RDSPIN_CMD,0x70078008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          30
	,  WRITE__CMD,0x70078008, 0x0000000000020007 //          31
	,  RDSPIN_CMD,0x70078000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          32
	,  WRITE__CMD,0x70078008, 0x0123456789abcdef //          33
	,  RDSPIN_CMD,0x70078000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          34
	,  RDSPIN_CMD,0x70078008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          35
	,  WRITE__CMD,0x70098008, 0x0000000000010008 //          36
	,  RDSPIN_CMD,0x70098000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          37
	,  RDSPIN_CMD,0x70098008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          38
	,  WRITE__CMD,0x70098008, 0x0000000000060007 //          39
	,  RDSPIN_CMD,0x70098000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          40
	,  WRITE__CMD,0x70098008, 0x0123456789abcdef //          41
	,  RDSPIN_CMD,0x70098000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          42
	,  RDSPIN_CMD,0x70098000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          43
	,  WRITE__CMD,0x70098008, 0xfedcba9876543210 //          44
	,  RDSPIN_CMD,0x70098000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          45
	,  RDSPIN_CMD,0x70098000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          46
	,  WRITE__CMD,0x70098008, 0x0123456789abcdef //          47
	,  RDSPIN_CMD,0x70098000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          48
	,  RDSPIN_CMD,0x70098000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          49
	,  WRITE__CMD,0x70098008, 0xfedcba9876543210 //          50
	,  RDSPIN_CMD,0x70098000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          51
	,  RDSPIN_CMD,0x70098000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          52
	,  WRITE__CMD,0x70098008, 0x0123456789abcdef //          53
	,  RDSPIN_CMD,0x70098000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          54
	,  RDSPIN_CMD,0x70098008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          55
	,  WRITE__CMD,0x70068008, 0x0000000000010008 //          56
	,  RDSPIN_CMD,0x70068000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          57
	,  RDSPIN_CMD,0x70068008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          58
	,  WRITE__CMD,0x70068008, 0x0000000000020007 //          59
	,  RDSPIN_CMD,0x70068000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          60
	,  WRITE__CMD,0x70068008, 0x0123456789abcdef //          61
	,  RDSPIN_CMD,0x70068000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          62
	,  RDSPIN_CMD,0x70068008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          63
	,  WRITE__CMD,0x70088008, 0x0000000000010008 //          64
	,  RDSPIN_CMD,0x70088000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          65
	,  RDSPIN_CMD,0x70088008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          66
	,  WRITE__CMD,0x70088008, 0x0000000000020007 //          67
	,  RDSPIN_CMD,0x70088000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          68
	,  WRITE__CMD,0x70088008, 0x0123456789abcdef //          69
	,  RDSPIN_CMD,0x70088000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          70
	,  RDSPIN_CMD,0x70088008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //          71
	,  WRITE__CMD,0x70018008, 0x0000000000010008 //          72
	,  RDSPIN_CMD,0x70018000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //          73
	,  RDSPIN_CMD,0x70018008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //          74
	,  WRITE__CMD,0x70018008, 0x0000000000090007 //          75
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          76
	,  WRITE__CMD,0x70018008, 0x0123456789abcdef //          77
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          78
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          79
	,  WRITE__CMD,0x70018008, 0xfedcba9876543210 //          80
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          81
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          82
	,  WRITE__CMD,0x70018008, 0x0123456789abcdef //          83
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          84
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          85
	,  WRITE__CMD,0x70018008, 0xfedcba9876543210 //          86
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          87
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          88
	,  WRITE__CMD,0x70018008, 0x0123456789abcdef //          89
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          90
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          91
	,  WRITE__CMD,0x70018008, 0xfedcba9876543210 //          92
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          93
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          94
	,  WRITE__CMD,0x70018008, 0x0123456789abcdef //          95
	,  RDSPIN_CMD,0x70018000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //          96
	,  RDSPIN_CMD,0x70018000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //          97
	,  WRITE__CMD,0x70018008, 0xfedcba9876543210 //          98
	,  RDSPIN_CMD,0x70018000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //          99
	,  RDSPIN_CMD,0x70018008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //         100
	,  WRITE__CMD,0x70038008, 0x0000000000010008 //         101
	,  RDSPIN_CMD,0x70038000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //         102
	,  RDSPIN_CMD,0x70038008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //         103
	,  WRITE__CMD,0x70038008, 0x0000000000020007 //         104
	,  RDSPIN_CMD,0x70038000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         105
	,  WRITE__CMD,0x70038008, 0x0123456789abcdef //         106
	,  RDSPIN_CMD,0x70038000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //         107
	,  RDSPIN_CMD,0x70038008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //         108
	,  WRITE__CMD,0x70028008, 0x0000000000010008 //         109
	,  RDSPIN_CMD,0x70028000, 0x0000000000000003, 'hffffffffffffffff, 'h32 //         110
	,  RDSPIN_CMD,0x70028008, 0x000000000001000b, 'hffffffffffffffff, 'h32 //         111
	,  WRITE__CMD,0x70028008, 0x0000000000090007 //         112
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         113
	,  WRITE__CMD,0x70028008, 0x0123456789abcdef //         114
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         115
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         116
	,  WRITE__CMD,0x70028008, 0xfedcba9876543210 //         117
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         118
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         119
	,  WRITE__CMD,0x70028008, 0x0123456789abcdef //         120
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         121
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         122
	,  WRITE__CMD,0x70028008, 0xfedcba9876543210 //         123
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         124
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         125
	,  WRITE__CMD,0x70028008, 0x0123456789abcdef //         126
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         127
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         128
	,  WRITE__CMD,0x70028008, 0xfedcba9876543210 //         129
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         130
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         131
	,  WRITE__CMD,0x70028008, 0x0123456789abcdef //         132
	,  RDSPIN_CMD,0x70028000, 0x0000000000000000, 'hffffffffffffffff, 'h32 //         133
	,  RDSPIN_CMD,0x70028000, 0x0000000000000002, 'hffffffffffffffff, 'h32 //         134
	,  WRITE__CMD,0x70028008, 0xfedcba9876543210 //         135
	,  RDSPIN_CMD,0x70028000, 0x0000000000000001, 'hffffffffffffffff, 'h32 //         136
	,  RDSPIN_CMD,0x70028008, 0x000000000001000a, 'hffffffffffffffff, 'h32 //         137
};

#define srot_llki_offsetBase 0x8000
#define srot_totalCommands         137
#endif
