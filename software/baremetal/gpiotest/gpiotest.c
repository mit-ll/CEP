//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      gpiotest.c
// Program:        Common Evaluation Platform
// Description:    A program that sets up the GPIO to read the
//                 Arty100T switches and set the User LEDs accordingly
// Notes:          
//
//************************************************************************

#include <stdint.h>
#include "encoding.h"
#include "compiler.h"
#include "kprintf.h"
#include "platform.h"
#include "mmio.h"

int main() {

  kputs("");
  kputs("");
  kputs("------------------");
  kputs(" RISC-V GPIO Test ");
  kputs("------------------");
  kputs("");
  kputs("");
  
  uint32_t switch_old 	= 0xFFFFFFFF;
  uint32_t switch_new 	= 0;

  // Enable the switch inputs
  reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_INPUT_EN), (uint32_t)(SW0_MASK | SW1_MASK | SW2_MASK | SW3_MASK));

  // Enable the LED outputs
  reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_OUTPUT_EN), (uint32_t)(LED0_MASK | LED1_MASK | LED2_MASK | LED3_MASK));

  // Infinite loop where you read the switches and write the LEDs
  while (1) {

	switch_new 	= (reg_read32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_INPUT_VAL)) >> 8) & 0xFF;

  	// A change of switch state has been detected... post debounce
  	if (switch_new != switch_old) {
  		kprintf("switches = %x\n\r", switch_new);
  		switch_old = switch_new;
  	}

  	reg_write32((uintptr_t)(GPIO_CTRL_ADDR + GPIO_OUTPUT_VAL), switch_new << 16);
  }

  return 0;
}
