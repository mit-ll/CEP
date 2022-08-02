// See LICENSE.Sifive for license details.
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "kprintf.h"

void kputc(int c)
{
#ifndef DISABLE_KPRINTF 
  volatile uint32_t *tx = &REG32(uart, UART_REG_TXFIFO);
  while ((int32_t)(*tx) < 0);
  *tx = (c & 0xFF);
#endif
}

int kgetc(void)
{
#ifndef DISABLE_KPRINTF
  uint32_t ch;
  volatile uint32_t *rx = &REG32(uart, UART_REG_RXFIFO);
  ch = *rx;

  if ((uint32_t)(ch & UART_RXEMPTY)) {
    return -1;
  } else {
    return(ch & 0x0ff);
  }
#else
  return -1;
#endif
}

void kputs(const char *s)
{
  char c;
  while (*s != '\0') {
    c = *s;
    kputc((int) c);
    s++;
  }
  kputc('\r');
  kputc('\n');
}