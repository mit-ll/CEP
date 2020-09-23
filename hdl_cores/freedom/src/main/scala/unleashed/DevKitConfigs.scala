// See LICENSE for license details.
package sifive.freedom.unleashed

import freechips.rocketchip.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.system._
import freechips.rocketchip.tile._

import sifive.blocks.devices.gpio._
import sifive.blocks.devices.spi._
import sifive.blocks.devices.uart._

import mitllBlocks.cep_addresses._
import mitllBlocks.cep_registers._

import mitllBlocks.aes._
import mitllBlocks.rsa._
import mitllBlocks.sha256._
import mitllBlocks.fir._
import mitllBlocks.iir._
import mitllBlocks.idft._
import mitllBlocks.des3._
import mitllBlocks.md5._
import mitllBlocks.gps._
import mitllBlocks.dft._

// Default FreedomU500Config
class FreedomU500Config extends Config(
  new WithJtagDTM            ++
  new WithNMemoryChannels(1) ++
  new WithNBigCores(4)       ++
  new BaseConfig
)

// Freedom U500 Dev Kit Peripherals
class U500DevKitPeripherals extends Config((site, here, up) => {
  case PeripheryUARTKey => List(
    UARTParams(address = BigInt(0x64000000L)))
  case PeripherySPIKey => List(
    SPIParams(rAddress = BigInt(0x64001000L)))
  case PeripheryGPIOKey => List(
    GPIOParams(address = BigInt(0x64002000L), width = 4))
  case PeripheryMaskROMKey => List(
    MaskROMParams(address = 0x10000, name = "BootROM", depth = 4096))
case PeripheryDES3Key => List(
    DES3Params(address = BigInt(CEPBaseAddresses.des3_base_addr)))
  case PeripheryAESKey => List(
    AESParams(address = BigInt(CEPBaseAddresses.aes_base_addr)))
  case PeripheryIIRKey => List(
    IIRParams(address = BigInt(CEPBaseAddresses.iir_base_addr)))
  case PeripheryIDFTKey => List(
    IDFTParams(address = BigInt(CEPBaseAddresses.idft_base_addr)))  
  case PeripheryGPSKey => List(
    GPSParams(address = BigInt(CEPBaseAddresses.gps_base_addr)))    
  case PeripheryMD5Key => List(
    MD5Params(address = BigInt(CEPBaseAddresses.md5_base_addr)))         
  case PeripheryDFTKey => List(
    DFTParams(address = BigInt(CEPBaseAddresses.dft_base_addr)))
  case PeripheryFIRKey => List(
     FIRParams(address = BigInt(CEPBaseAddresses.fir_base_addr)))  
  case PeripherySHA256Key => List(
     SHA256Params(address = BigInt(CEPBaseAddresses.sha256_base_addr)))   
  case PeripheryRSAKey => List(
     RSAParams(address = BigInt(CEPBaseAddresses.rsa_base_addr)))  
  case PeripheryCEPRegistersKey => List(
    CEPREGSParams(address = BigInt(CEPBaseAddresses.cepregisters_base_addr)))
})

// Freedom U500 Dev Kit
class U500DevKitConfig extends Config(
  new WithNExtTopInterrupts(0)   ++
  new U500DevKitPeripherals ++
  new FreedomU500Config().alter((site,here,up) => {
    case SystemBusKey => up(SystemBusKey).copy()
    case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency =
    Some(BigDecimal(site(DevKitFPGAFrequencyKey)*1000000).setScale(0, BigDecimal.RoundingMode.HALF_UP).toBigInt))  
    case DTSTimebase => BigInt(1000000)
    case JtagDTMKey => new JtagDTMConfig (
      idcodeVersion = 2,      // 1 was legacy (FE310-G000, Acai).
      idcodePartNum = 0x000,  // Decided to simplify.
      idcodeManufId = 0x489,  // As Assigned by JEDEC to SiFive. Only used in wrappers / test harnesses.
      debugIdleCycles = 5)    // Reasonable guess for synchronization
  })
)
