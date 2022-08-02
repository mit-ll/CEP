//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      arty100t_gpio.h
// Program:        Common Evaluation Platform
// Description:    Bit mappings for the CEP's Arty100T GPIO
// Notes:          
//
//************************************************************************

#ifdef ARTY100T_TARGET
#ifndef _ARTY100T_GPIO_H
#define _ARTY100T_GPIO_H

#define GPIO0_MASK      (0x00000001)
#define GPIO1_MASK      (0x00000002)
#define GPIO2_MASK      (0x00000004)
#define GPIO3_MASK      (0x00000008)
#define GPIO4_MASK      (0x00000010)
#define GPIO5_MASK      (0x00000020)
#define GPIO6_MASK      (0x00000040)
#define GPIO7_MASK      (0x00000080)
#define SW0_MASK        (0x00000100)
#define SW1_MASK        (0x00000200)
#define SW2_MASK        (0x00000400)
#define SW3_MASK        (0x00000800)
#define BTN0_MASK       (0x00001000)
#define BTN1_MASK       (0x00002000)
#define BTN2_MASK       (0x00004000)
#define BTN3_MASK       (0x00008000)
#define LED0_MASK       (0x00010000)
#define LED1_MASK       (0x00020000)
#define LED2_MASK       (0x00040000)
#define LED3_MASK       (0x00080000)

#define SWtoLED_SHIFT	8
#define SWtoLED_MASK	(0x00000F00)

#endif /* _ARTY100T_GPIO_H */
#endif /* ARTY100T_TARGET */