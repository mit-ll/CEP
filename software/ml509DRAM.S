#include "or1k-asm.h"

/*
 * Define symbols to be used during startup - file is linked at compile time
 *
 */
.weak _board_mem_base
.weak _board_mem_size
.weak _board_clk_freq

_board_mem_base:	.long	0x0
_board_mem_size:	.long	0x08000000

_board_clk_freq:	.long	50000000
	
/* Peripheral information - Set base to 0 if not present*/
.weak _board_uart_base
.weak _board_uart_baud
.weak _board_uart_IRQ
	
_board_uart_base:	.long	0x90000000
_board_uart_baud:	.long	115200
_board_uart_IRQ:	.long	2

.weak _board_exit
_board_exit:
        l.j _board_exit
        l.nop 0x1

.global _board_init_early
_board_init_early:
    OR1K_DELAYED_NOP(l.jr r9)

.weak _board_init
_board_init:
    OR1K_DELAYED_NOP(l.jr r9)
