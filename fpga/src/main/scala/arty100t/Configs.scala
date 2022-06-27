package chipyard.fpga.arty100t

import sys.process._
import math.min

import freechips.rocketchip.config.{Config, Parameters}
import freechips.rocketchip.subsystem.{SystemBusKey, PeripheryBusKey, ControlBusKey, ExtMem, WithDTS}
import freechips.rocketchip.devices.debug.{DebugModuleKey, ExportDebug, JTAG}
import freechips.rocketchip.devices.tilelink.{DevNullParams, BootROMLocated}
import freechips.rocketchip.diplomacy.{DTSModel, DTSTimebase, RegionType, AddressSet}
import freechips.rocketchip.tile.{XLen}

import sifive.blocks.devices.spi.{PeripherySPIKey, SPIParams}
import sifive.blocks.devices.uart.{PeripheryUARTKey, UARTParams}
import sifive.blocks.devices.gpio.{PeripheryGPIOKey, GPIOParams}

import sifive.fpgashells.shell.{DesignKey}
import sifive.fpgashells.shell.xilinx.{ArtyDDRSize}

import testchipip.{SerialTLKey}

import chipyard.{BuildSystem, ExtTLMem, DefaultClockFrequencyKey}

class WithDefaultPeripherals extends Config((site, here, up) => {
  case PeripheryUARTKey   => List(UARTParams(address  = BigInt(0x64000000L)))
  case PeripherySPIKey    => List(SPIParams(rAddress  = BigInt(0x64001000L)))
  case PeripheryGPIOKey => {
    if (Arty100TGPIOs.width > 0) {
      require(Arty100TGPIOs.width <= 64) // currently only support 64 GPIOs (change addrs to get more)
      val gpioAddrs = Seq(BigInt(0x64002000), BigInt(0x64007000))
      val maxGPIOSupport = 32 // max gpios supported by SiFive driver (split by 32)
      List.tabulate(((Arty100TGPIOs.width - 1)/maxGPIOSupport) + 1)(n => {
        GPIOParams(address = gpioAddrs(n), width = min(Arty100TGPIOs.width - maxGPIOSupport*n, maxGPIOSupport))
      })
    }
    else {
      List.empty[GPIOParams]
    }
  }
})

class WithSystemModifications (enableCEPRegs: Int = 0) extends Config((site, here, up) => {
  case DTSTimebase => BigInt((1e6).toLong)
  case BootROMLocated(x) => up(BootROMLocated(x), site).map { p =>
    // invoke makefile for sdboot
    val freqMHz = (site(DefaultClockFrequencyKey) * 1e6).toLong
    val make = s"make -B -C fpga/src/main/resources/arty100t/sdboot PBUS_CLK=${freqMHz} ENABLE_CEPREG=${enableCEPRegs} bin"
    require (make.! == 0, "Failed to build bootrom")
    p.copy(hang = 0x10000, contentFileName = s"./fpga/src/main/resources/arty100t/sdboot/build/sdboot.bin")
  }
  case ExtMem       => up(ExtMem, site).map(x => x.copy(master = x.master.copy(size = site(ArtyDDRSize)))) // set extmem to DDR size
  case SerialTLKey  => None // remove serialized tl port
})

// DOC include start: AbstractArty100T and Rocket
class WithArty100TTweaks (enableCEPRegs: Int = 0) extends Config(
  // harness binders
  new WithUART ++
  new WithSPISDCard ++
  new WithDDRMem ++
  new WithGPIO ++
  // io binders
  new WithUARTIOPassthrough ++
  new WithSPIIOPassthrough ++
  new WithTLIOPassthrough ++
  new WithGPIOPassthrough ++
  // other configuration
  new WithDefaultPeripherals ++
  new chipyard.config.WithTLBackingMemory ++      // use TL backing memory
  new WithSystemModifications(enableCEPRegs) ++   // setup busses, use sdboot bootrom, setup ext. mem. size
  new chipyard.config.WithNoDebug ++              // remove debug module
  new freechips.rocketchip.subsystem.WithoutTLMonitors ++
  new freechips.rocketchip.subsystem.WithNMemoryChannels(1) ++
  new WithFPGAFrequency(100)                      // default 100MHz freq
)

class RocketArty100TConfig extends Config(
  // reduce L2 size to fit in 100T's BRAMs
  new freechips.rocketchip.subsystem.WithInclusiveCache(capacityKB=256) ++
  // with reduced cache size, closes timing at 50 MHz
  new WithFPGAFrequency(50) ++
  new WithArty100TTweaks ++
  new chipyard.RocketConfig
)
// DOC include end: AbstractArty100T and Rocket

class RocketArty100TSimConfig extends Config(
   new WithFPGASimSerial ++
   new testchipip.WithDefaultSerialTL ++
   new chipyard.harness.WithSimSerial ++
   new chipyard.harness.WithTiedOffDebug ++
   new RocketArty100TConfig
 )

class RocketArty100TCEPConfig extends Config(
  // Add the CEP registers
  new chipyard.config.WithCEPRegisters ++
  new chipyard.config.WithAES ++
  new chipyard.config.WithSROTFPGA ++

  // Overide the chip info 
  new WithDTS("mit-ll,rocketchip-cep", Nil) ++

  // with reduced cache size, closes timing at 50 MHz
  new WithFPGAFrequency(50) ++

  // Include the Arty100T Tweaks with CEP Registers enabled (passed to the bootrom build)
  new WithArty100TTweaks(1) ++
  new chipyard.RocketNoL2Config
)

// A minimum CEP configuration with only the registers component
class RocketArty100TMinCEPConfig extends Config(
  // Add the CEP registers
  new chipyard.config.WithCEPRegisters ++

  // Overide the chip info 
  new WithDTS("mit-ll,rocketchip-cep", Nil) ++

  // with reduced cache size, closes timing at 50 MHz
  new WithFPGAFrequency(50) ++

  // Include the Arty100T Tweaks with CEP Registers enabled (passed to the bootrom build)
  new WithArty100TTweaks(1) ++
  new chipyard.RocketNoL2Config
)

class WithFPGAFrequency(fMHz: Double) extends Config(
  new chipyard.config.WithPeripheryBusFrequency(fMHz) ++ // assumes using PBUS as default freq.
  new chipyard.config.WithMemoryBusFrequency(fMHz)
)

class WithFPGAFreq25MHz extends WithFPGAFrequency(25)
class WithFPGAFreq50MHz extends WithFPGAFrequency(50)
class WithFPGAFreq75MHz extends WithFPGAFrequency(75)
class WithFPGAFreq100MHz extends WithFPGAFrequency(100)
