//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      gpiotest.c
// Program:        Common Evaluation Platform
// Description:    A program that sets up the GPIO to read the
//                 Arty100T switches and set the User LEDs accordingly
// Notes:          GPIO mapping from arty100t_gpio.h
//
//************************************************************************

#include <stdint.h>
#include <stdio.h>
#include "encoding.h"
#include "compiler.h"
#include "kprintf.h"
#include "platform.h"
#include "mmio.h"

#define   DEBOUNCE_CNT  10
#define   DEBOUNCE_WS   50000

// A simple routine to debouce gpio reads
uint32_t get_gpio_debounced(uint32_t mask) {

  uint32_t  gpio_old = 0;
  uint32_t  gpio_new = 0;
  int       debounce_counter = 0;

  while (debounce_counter < DEBOUNCE_CNT) {
    gpio_new = reg_read32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_INPUT_VAL)) & mask;

    // Introduce some wait states beteween reads
    for (volatile int i = 0; i < DEBOUNCE_WS; i++) {}

    if (gpio_new == gpio_old) {
      debounce_counter++;
    } else {
      debounce_counter = 0;
    }

    gpio_old = gpio_new;

  } // end while

  return gpio_new;

} // get_switch


int main() {

  int c;

  // Enable UART TX & RX (let's not assume the bootrom did it)
  REG32(uart, UART_REG_TXCTRL) |= UART_TXEN;
  REG32(uart, UART_REG_RXCTRL) |= UART_RXEN;

  puts("");
  puts("");
  puts("--------------------------");
  puts("     RISC-V GPIO Test     ");
  puts("--------------------------");
#ifdef VCU118_TARGET
  puts("     Built for VCU118");
#elif VC707_TARGET
  puts("     Built for VC707");
#else
  puts("    Built for Arty100T");
#endif
  puts("   Console Echo Enabled   ");
  puts("");
  puts("");
  
  uint32_t gpio_old  = 0xFFFFFFFF;
  uint32_t gpio_new  = 0;

  // Enable the switch inputs
#if defined(VCU118_TARGET) || defined(VC707_TARGET)
  reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_INPUT_EN), (uint32_t)(SWN_MASK | SWE_MASK | SWS_MASK | SWW_MASK));
#else
  reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_INPUT_EN), (uint32_t)(SW0_MASK | SW1_MASK | SW2_MASK | SW3_MASK));
#endif

  // Enable the LED outputs
  reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_OUTPUT_EN), (uint32_t)(LED0_MASK | LED1_MASK | LED2_MASK | LED3_MASK));

  // Infinite loop where you read the switches and write the LEDs
  while (1) {

    // Get the switches state
#if defined(VCU118_TARGET) || defined(VC707_TARGET)
    gpio_new = get_gpio_debounced(SWN_MASK | SWE_MASK | SWS_MASK | SWW_MASK);
#else
    gpio_new = get_gpio_debounced(SW0_MASK | SW1_MASK | SW2_MASK | SW3_MASK);
#endif

    // A change of switch state has been detected... post debounce
    if (gpio_new != gpio_old) {
      printf("gpio = %08x\n", gpio_new);
      gpio_old = gpio_new;
    }

    // A simple tty echo routine where CR and LF are converted to CR+LF (getchar, which mapps to kgetc in baremetal, is non-blocking)
    c = getchar();
    if (c == '\r' || c == '\n') {
      putchar('\r');
      putchar('\n');
    } else if (c >= 0) {
      putchar(c);
    }

    // Write the LEDs
    reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_OUTPUT_VAL), (gpio_new & SWtoLED_MASK) << SWtoLED_SHIFT);
  }

  return 0;
}
