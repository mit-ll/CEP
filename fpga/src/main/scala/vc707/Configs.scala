package chipyard.fpga.vc707

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
import sifive.fpgashells.shell.xilinx.{VC7071GDDRSize}

import mitllBlocks.cep_addresses._

import testchipip.{SerialTLKey}

import chipyard.{BuildSystem, ExtTLMem, DefaultClockFrequencyKey}

class WithDefaultPeripherals extends Config((site, here, up) => {
  case PeripheryUARTKey   => List(UARTParams(address  = BigInt(0x64000000L)))
  case PeripherySPIKey    => List(SPIParams(rAddress  = BigInt(0x64001000L)))
  case PeripheryGPIOKey => {
    if (VC707GPIOs.width > 0) {
      require(VC707GPIOs.width <= 64) // currently only support 64 GPIOs (change addrs to get more)
      val gpioAddrs = Seq(BigInt(0x64002000), BigInt(0x64007000))
      val maxGPIOSupport = 32 // max gpios supported by SiFive driver (split by 32)
      List.tabulate(((VC707GPIOs.width - 1)/maxGPIOSupport) + 1)(n => {
        GPIOParams(address = gpioAddrs(n), width = min(VC707GPIOs.width - maxGPIOSupport*n, maxGPIOSupport))
      })
    }
    else {
      List.empty[GPIOParams]
    }
  }
})

class WithCEPSystemModifications extends Config((site, here, up) => {
  case DTSTimebase => BigInt((1e6).toLong)
  case BootROMLocated(x) => up(BootROMLocated(x), site).map { p =>
    // invoke makefile for sdboot
    val freqMHz = (site(DefaultClockFrequencyKey) * 1e6).toLong
    val make = s"make -B -C fpga/src/main/resources/vc707/cep_sdboot PBUS_CLK=${freqMHz} bin"
    require (make.! == 0, "Failed to build bootrom")
    p.copy(hang = 0x10000, contentFileName = s"./fpga/src/main/resources/vc707/cep_sdboot/build/sdboot.bin")
  }
  case ExtMem => up(ExtMem, site).map(x => x.copy(master = x.master.copy(size = site(VC7071GDDRSize)))) // set extmem to DDR size
  case SerialTLKey => None // remove serialized tl port
})

class WithVC707CEPTweaks extends Config(
  // harness bindersn
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
  new WithCEPSystemModifications ++               // setup busses, use sdboot bootrom, setup ext. mem. size
  new chipyard.config.WithNoDebug ++              // remove debug module
  new freechips.rocketchip.subsystem.WithoutTLMonitors ++
  new freechips.rocketchip.subsystem.WithNMemoryChannels(1) ++
  new WithFPGAFrequency(100)                      // default 100MHz freq
)

class RocketVC707CEPConfig extends Config(
  // Add the CEP registers
  new chipyard.config.WithCEPRegisters ++
  new chipyard.config.WithAES ++
  new chipyard.config.WithSROTFPGA ++

  // Overide the chip info 
  new WithDTS("mit-ll,cep-vc707", Nil) ++

  // with reduced cache size, closes timing at 50 MHz
  new WithFPGAFrequency(100) ++

  // Include the VC707 Tweaks with CEP Registers enabled (passed to the bootrom build)
  new WithVC707CEPTweaks ++

  // Instantiate one big core
  new freechips.rocketchip.subsystem.WithNBigCores(1) ++
  
  // Default Chipyard AbstractConfig with L2 removed
  new chipyard.config.AbstractNoL2Config
)

class RocketVC707BigCEPConfig extends Config(
  new chipyard.config.WithAES ++
  new chipyard.config.WithDES3 ++
  new chipyard.config.WithFIR ++
  new chipyard.config.WithIIR ++
  new chipyard.config.WithDFT ++
  new chipyard.config.WithIDFT ++
  new chipyard.config.WithMD5 ++
  new chipyard.config.WithGPS(params = Seq(
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.gps_0_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.gps_0_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.gps_0_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.gps_0_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.gps_0_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.gps_0_llki_sendrecv_addr),
      dev_name            = s"gps_0"),
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.gps_1_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.gps_1_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.gps_1_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.gps_1_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.gps_1_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.gps_1_llki_sendrecv_addr),
      dev_name            = s"gps_1"),
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.gps_2_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.gps_2_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.gps_2_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.gps_2_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.gps_2_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.gps_2_llki_sendrecv_addr),
      dev_name            = s"gps_2"),
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.gps_3_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.gps_3_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.gps_3_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.gps_3_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.gps_3_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.gps_3_llki_sendrecv_addr),
      dev_name            = s"gps_3")
    )) ++
  new chipyard.config.WithSHA256(params = Seq(
    COREParams( 
      slave_base_addr     = BigInt(CEPBaseAddresses.sha256_0_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.sha256_0_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.sha256_0_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.sha256_0_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.sha256_0_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.sha256_0_llki_sendrecv_addr),
      dev_name            = s"sha256_0"),
    COREParams( 
      slave_base_addr     = BigInt(CEPBaseAddresses.sha256_1_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.sha256_1_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.sha256_1_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.sha256_1_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.sha256_1_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.sha256_1_llki_sendrecv_addr),
      dev_name            = s"sha256_1"),
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.sha256_2_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.sha256_2_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.sha256_2_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.sha256_2_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.sha256_2_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.sha256_2_llki_sendrecv_addr),
      dev_name            = s"sha256_2"),
    COREParams(
      slave_base_addr     = BigInt(CEPBaseAddresses.sha256_3_base_addr),
      slave_depth         = BigInt(CEPBaseAddresses.sha256_3_depth),
      llki_base_addr      = BigInt(CEPBaseAddresses.sha256_3_llki_base_addr),
      llki_depth          = BigInt(CEPBaseAddresses.sha256_3_llki_depth),
      llki_ctrlsts_addr   = BigInt(CEPBaseAddresses.sha256_3_llki_ctrlsts_addr),
      llki_sendrecv_addr  = BigInt(CEPBaseAddresses.sha256_3_llki_sendrecv_addr),
      dev_name            = s"sha256_3")
    )) ++

  new chipyard.config.WithCEPRegisters ++

  // Instantiation of the RSA core with or w/o the ARM compiled memories
  new chipyard.config.WithRSA ++

  // Instantiation of the Surrogate Root of Trust (with or w/o the ARM compiled memories)
  new chipyard.config.WithSROT ++

  // Overide the chip info 
  new WithDTS("mit-ll,cep-vc707", Nil) ++

  // with reduced cache size, closes timing at 50 MHz
  new WithFPGAFrequency(100) ++

  // Include the VC707 Tweaks with CEP Registers enabled (passed to the bootrom build)
  new WithVC707CEPTweaks ++

  // Instantiate four big cores
  new freechips.rocketchip.subsystem.WithNBigCores(4) ++
  
  // Default Chipyard AbstractConfig with L2 removed
  new chipyard.config.AbstractNoL2Config
)

class WithFPGAFrequency(fMHz: Double) extends Config(
  new chipyard.config.WithPeripheryBusFrequency(fMHz) ++ // assumes using PBUS as default freq.
    new chipyard.config.WithMemoryBusFrequency(fMHz)
)

class WithFPGAFreq25MHz extends WithFPGAFrequency(25)
class WithFPGAFreq50MHz extends WithFPGAFrequency(50)
class WithFPGAFreq75MHz extends WithFPGAFrequency(75)
class WithFPGAFreq100MHz extends WithFPGAFrequency(100)

