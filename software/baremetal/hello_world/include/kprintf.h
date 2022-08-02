// See LICENSE.Sifive for license details.
#ifndef _SDBOOT_KPRINTF_H
#define _SDBOOT_KPRINTF_H

#include <stdint.h>
#include <platform.h>

#define REG32(p, i)	((p)[(i) >> 2])

#define UART_RXEMPTY (1 << 31)

#ifndef UART_CTRL_ADDR
  #ifndef UART_NUM
    #define UART_NUM 0
  #endif

  #define _CONCAT3(A, B, C) A ## B ## C
  #define _UART_CTRL_ADDR(UART_NUM) _CONCAT3(UART, UART_NUM, _CTRL_ADDR)
  #define UART_CTRL_ADDR _UART_CTRL_ADDR(UART_NUM)
#endif
static volatile uint32_t * const uart = (void *)(UART_CTRL_ADDR);


void kputc(int c);
int kgetc(void);
void kputs(const char *);

#endif /* _SDBOOT_KPRINTF_H */
