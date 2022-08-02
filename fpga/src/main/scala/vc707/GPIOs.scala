//#************************************************************************
//# Copyright 2022 Massachusets Institute of Technology
//# SPDX short identifier: BSD-2-Clause
//#
//# File Name:      GPIOs.scala
//# Program:        Common Evaluation Platform (CEP)
//# Description:    Pin mappings for GPIO on the vc707
//# Notes:          Modified from the Chipyard VCU118 BringupGPIOs.scala
//#************************************************************************

package chipyard.fpga.vc707

import scala.collection.mutable.{LinkedHashMap}

object VC707GPIOs {
    // map of the pin name (akin to die pin name) to (fpga package pin, IOSTANDARD, add pullup resistor?)

    val pinMapping = LinkedHashMap(
        "sw0"   -> ("AV30", "LVCMOS18", false),  // Bit0  - 8-Pole DIP Bit0
        "sw1"   -> ("AY33", "LVCMOS18", false),  // Bit1  - 8-Pole DIP Bit1 
        "sw2"   -> ("BA31", "LVCMOS18", false),  // Bit2  - 8-Pole DIP Bit2 
        "sw3"   -> ("BA32", "LVCMOS18", false),  // Bit3  - 8-Pole DIP Bit3 
        "sw4"   -> ("AW30", "LVCMOS18", false),  // Bit4  - 8-Pole DIP Bit4 
        "sw5"   -> ("AY30", "LVCMOS18", false),  // Bit5  - 8-Pole DIP Bit5 
        "sw6"   -> ("BA30", "LVCMOS18", false),  // Bit6  - 8-Pole DIP Bit6 
        "sw7"   -> ("BB31", "LVCMOS18", false),  // Bit7  - 8-Pole DIP Bit7 
        "swN"   -> ("AR40", "LVCMOS18", false),  // Bit8  - GPIO Button N
        "swE"   -> ("AU38", "LVCMOS18", false),  // Bit9  - GPIO Button E
        "swS"   -> ("AP40", "LVCMOS18", false),  // Bit10 - GPIO Button S
        "swW"   -> ("AW40", "LVCMOS18", false),  // Bit11 - GPIO Button W
        "swC"   -> ("AV39", "LVCMOS18", false),  // Bit12 - GPIO Button C
        "led0"  -> ("AM39", "LVCMOS18", false),  // Bit13 - GPIO LED 0
        "led1"  -> ("AN39", "LVCMOS18", false),  // Bit14 - GPIO LED 1
        "led2"  -> ("AR37", "LVCMOS18", false),  // Bit15 - GPIO LED 2
        "led3"  -> ("AT37", "LVCMOS18", false),  // Bit16 - GPIO LED 3
        "led4"  -> ("AR35", "LVCMOS18", false),  // Bit17 - GPIO LED 4
        "led5"  -> ("AP41", "LVCMOS18", false),  // Bit18 - GPIO LED 5
        "led6"  -> ("AP42", "LVCMOS18", false),  // Bit19 - GPIO LED 6
        "led7"  -> ("AU39", "LVCMOS18", false)   // Bit20 - GPIO LED 7
    )

    // return list of names (ordered)
    def names: Seq[String] = pinMapping.keys.toSeq

    // return number of GPIOs
    def width: Int = pinMapping.size
}
