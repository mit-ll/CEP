//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      srot_playback.h
// Program:        Common Evaluation Platform (CEP)
// Description:    SROT Core unit testbench stimulus
// Notes:
//
//************************************************************************
`ifndef SROT_playback_H
`define SROT_playback_H

`ifndef PLAYBACK_CMD_H
`define PLAYBACK_CMD_H
`define WRITE__CMD  1
`define RDnCMP_CMD  2
`define RDSPIN_CMD  3
`define WRITE__CMD_SIZE  3
`define RDnCMP_CMD_SIZE  3
`define RDSPIN_CMD_SIZE  5
`endif

longint srot_playback[] = {
	   `WRITE__CMD,'h70008008, 'h0000000000030007 //           1
	,  `RDSPIN_CMD,'h70008000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //           2
	,  `WRITE__CMD,'h70008008, 'h0123456789abcdef //           3
	,  `RDSPIN_CMD,'h70008000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //           4
	,  `RDSPIN_CMD,'h70008000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //           5
	,  `WRITE__CMD,'h70008008, 'hfedcba9876543210 //           6
	,  `RDSPIN_CMD,'h70008000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //           7
	,  `RDSPIN_CMD,'h70008008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //           8
	,  `WRITE__CMD,'h70048008, 'h0000000000020007 //           9
	,  `RDSPIN_CMD,'h70048000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          10
	,  `WRITE__CMD,'h70048008, 'h0123456789abcdef //          11
	,  `RDSPIN_CMD,'h70048000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          12
	,  `RDSPIN_CMD,'h70048008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          13
	,  `WRITE__CMD,'h70058008, 'h0000000000020007 //          14
	,  `RDSPIN_CMD,'h70058000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          15
	,  `WRITE__CMD,'h70058008, 'h0123456789abcdef //          16
	,  `RDSPIN_CMD,'h70058000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          17
	,  `RDSPIN_CMD,'h70058008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          18
	,  `WRITE__CMD,'h70078008, 'h0000000000020007 //          19
	,  `RDSPIN_CMD,'h70078000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          20
	,  `WRITE__CMD,'h70078008, 'h0123456789abcdef //          21
	,  `RDSPIN_CMD,'h70078000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          22
	,  `RDSPIN_CMD,'h70078008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          23
	,  `WRITE__CMD,'h70098008, 'h0000000000060007 //          24
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          25
	,  `WRITE__CMD,'h70098008, 'h0123456789abcdef //          26
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          27
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          28
	,  `WRITE__CMD,'h70098008, 'hfedcba9876543210 //          29
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          30
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          31
	,  `WRITE__CMD,'h70098008, 'h0123456789abcdef //          32
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          33
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          34
	,  `WRITE__CMD,'h70098008, 'hfedcba9876543210 //          35
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          36
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          37
	,  `WRITE__CMD,'h70098008, 'h0123456789abcdef //          38
	,  `RDSPIN_CMD,'h70098000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          39
	,  `RDSPIN_CMD,'h70098008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          40
	,  `WRITE__CMD,'h70068008, 'h0000000000020007 //          41
	,  `RDSPIN_CMD,'h70068000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          42
	,  `WRITE__CMD,'h70068008, 'h0123456789abcdef //          43
	,  `RDSPIN_CMD,'h70068000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          44
	,  `RDSPIN_CMD,'h70068008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          45
	,  `WRITE__CMD,'h70088008, 'h0000000000020007 //          46
	,  `RDSPIN_CMD,'h70088000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          47
	,  `WRITE__CMD,'h70088008, 'h0123456789abcdef //          48
	,  `RDSPIN_CMD,'h70088000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          49
	,  `RDSPIN_CMD,'h70088008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          50
	,  `WRITE__CMD,'h70018008, 'h0000000000090007 //          51
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          52
	,  `WRITE__CMD,'h70018008, 'h0123456789abcdef //          53
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          54
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          55
	,  `WRITE__CMD,'h70018008, 'hfedcba9876543210 //          56
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          57
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          58
	,  `WRITE__CMD,'h70018008, 'h0123456789abcdef //          59
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          60
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          61
	,  `WRITE__CMD,'h70018008, 'hfedcba9876543210 //          62
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          63
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          64
	,  `WRITE__CMD,'h70018008, 'h0123456789abcdef //          65
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          66
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          67
	,  `WRITE__CMD,'h70018008, 'hfedcba9876543210 //          68
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          69
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          70
	,  `WRITE__CMD,'h70018008, 'h0123456789abcdef //          71
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          72
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          73
	,  `WRITE__CMD,'h70018008, 'hfedcba9876543210 //          74
	,  `RDSPIN_CMD,'h70018000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          75
	,  `RDSPIN_CMD,'h70018008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          76
	,  `WRITE__CMD,'h70038008, 'h0000000000020007 //          77
	,  `RDSPIN_CMD,'h70038000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          78
	,  `WRITE__CMD,'h70038008, 'h0123456789abcdef //          79
	,  `RDSPIN_CMD,'h70038000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //          80
	,  `RDSPIN_CMD,'h70038008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //          81
	,  `WRITE__CMD,'h70028008, 'h0000000000090007 //          82
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          83
	,  `WRITE__CMD,'h70028008, 'h0123456789abcdef //          84
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          85
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          86
	,  `WRITE__CMD,'h70028008, 'hfedcba9876543210 //          87
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          88
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          89
	,  `WRITE__CMD,'h70028008, 'h0123456789abcdef //          90
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          91
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          92
	,  `WRITE__CMD,'h70028008, 'hfedcba9876543210 //          93
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          94
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          95
	,  `WRITE__CMD,'h70028008, 'h0123456789abcdef //          96
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //          97
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //          98
	,  `WRITE__CMD,'h70028008, 'hfedcba9876543210 //          99
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //         100
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //         101
	,  `WRITE__CMD,'h70028008, 'h0123456789abcdef //         102
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000000, 'hffffffffffffffff, 'h32 //         103
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000002, 'hffffffffffffffff, 'h32 //         104
	,  `WRITE__CMD,'h70028008, 'hfedcba9876543210 //         105
	,  `RDSPIN_CMD,'h70028000, 'h0000000000000001, 'hffffffffffffffff, 'h32 //         106
	,  `RDSPIN_CMD,'h70028008, 'h000000000001000a, 'hffffffffffffffff, 'h32 //         107
};

`define srot_llki_offsetBase 'h8000
`define srot_totalCommands         107
`endif
