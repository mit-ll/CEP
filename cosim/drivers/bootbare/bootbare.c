//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

// See LICENSE for license details.
#include <stdint.h>

#include <platform.h>

#include "common.h"

#define DEBUG
//#include "kprintf.h"

#define MAX_CORES 8

#define PAYLOAD_SIZE	(26 << 11)

#ifndef TL_CLK
#error Must define TL_CLK
#endif

#define F_CLK TL_CLK


int main(void)
{
  /*
	REG32(uart, UART_REG_TXCTRL) = UART_TXEN;

	print_greeting();
	kputs("INIT");
	sd_poweron();
	if (sd_cmd0() ||
	    sd_cmd8() ||
	    sd_acmd41() ||
	    sd_cmd58() ||
	    sd_cmd16() ||
	    copy()) {
		kputs("ERROR");
		return 1;
	}

	kputs("BOOT");
  */
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}
