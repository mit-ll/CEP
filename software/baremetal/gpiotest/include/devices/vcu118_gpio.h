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

#ifdef VCU118_TARGET
#ifndef _VCU118_GPIO_H
#define _VCU118_GPIO_H

#define SW0_MASK        (0x00000001)
#define SW1_MASK        (0x00000002)
#define SW2_MASK        (0x00000004)
#define SW3_MASK        (0x00000008)
#define SWN_MASK        (0x00000010)
#define SWE_MASK        (0x00000020)
#define SWW_MASK        (0x00000040)
#define SWS_MASK        (0x00000080)
#define SWC_MASK        (0x00000100)
#define LED0_MASK       (0x00000200)
#define LED1_MASK       (0x00000400)
#define LED2_MASK       (0x00000800)
#define LED3_MASK       (0x00001000)
#define LED4_MASK       (0x00002000)
#define LED5_MASK       (0x00004000)
#define LED6_MASK       (0x00008000)
#define LED7_MASK       (0x00010000)

#define SWtoLED_SHIFT	5
#define SWtoLED_MASK	(0x000001F0)

#endif /* _VCU118_GPIO_H */
#endif /* VCU118_TARGET */