package chipyard.fpga.vcu118

import scala.collection.mutable.{LinkedHashMap}

object VCU118GPIOs {
    // map of the pin name (akin to die pin name) to (fpga package pin, IOSTANDARD, add pullup resistor?)
    val pinMapping = LinkedHashMap(
        // these connect to LEDs and switches on the VCU118 (and use 1.2V)
        "sw0"   -> ("B17",  "LVCMOS12", false), // Bit0  - 4-Pole DIP Bit 0
        "sw1"   -> ("G16",  "LVCMOS12", false), // Bit1  - 4-Pole DIP Bit 1
        "sw2"   -> ("J16",  "LVCMOS12", false), // Bit2  - 4-Pole DIP Bit 2
        "sw3"   -> ("D21",  "LVCMOS12", false), // Bit3  - 4-Pole DIP Bit 3
        "swN"   -> ("BB24", "LVCMOS18", false), // Bit4  - GPIO Button N
        "swE"   -> ("BE23", "LVCMOS18", false), // Bit5  - GPIO Button E
        "swW"   -> ("BF22", "LVCMOS18", false), // Bit6  - GPIO Button W
        "swS"   -> ("BE22", "LVCMOS18", false), // Bit7  - GPIO Button S
        "swC"   -> ("BD23", "LVCMOS18", false), // Bit8  - GPIO Button C
        "led0"  -> ("AT32", "LVCMOS12", false), // Bit9  - GPIO LED 0
        "led1"  -> ("AV34", "LVCMOS12", false), // Bit10 - GPIO LED 1
        "led2"  -> ("AY30", "LVCMOS12", false), // Bit11 - GPIO LED 2
        "led3"  -> ("BB32", "LVCMOS12", false), // Bit12 - GPIO LED 3
        "led4"  -> ("BF32", "LVCMOS12", false), // Bit13 - GPIO LED 4
        "led5"  -> ("AU37", "LVCMOS12", false), // Bit14 - GPIO LED 5
        "led6"  -> ("AV36", "LVCMOS12", false), // Bit15 - GPIO LED 6
        "led7"  -> ("BA37", "LVCMOS12", false)  // Bit16 - GPIO LED 7
    )

    // return list of names (ordered)
    def names: Seq[String] = pinMapping.keys.toSeq

    // return number of GPIOs
    def width: Int = pinMapping.size
}
