// See LICENSE for license details.
package chipyard.fpga.arty

import sys.process._

import freechips.rocketchip.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy.{DTSModel, DTSTimebase}
import freechips.rocketchip.system._
import freechips.rocketchip.tile._

import sifive.blocks.devices.uart._

import testchipip.{SerialTLKey}

import chipyard.{BuildSystem, DefaultClockFrequencyKey}

class WithDefaultPeripherals extends Config((site, here, up) => {
  case PeripheryUARTKey => List(
    UARTParams(BigInt(0x64000000L)))
  case BootROMLocated(x) => up(BootROMLocated(x), site).map { p =>
    // invoke makefile for sdboot
    val freqMHz = (site(DefaultClockFrequencyKey) * 1e6).toLong
    val make = s"make -C fpga/src/main/resources/vcu118/sdboot PBUS_CLK=${freqMHz} bin"
    require (make.! == 0, "Failed to build bootrom")
    p.copy(hang = 0x10000, contentFileName = s"./fpga/src/main/resources/vcu118/sdboot/build/sdboot.bin")
  }
  case DTSTimebase => BigInt((1e6).toLong)
  case JtagDTMKey => new JtagDTMConfig (
    idcodeVersion = 2,
    idcodePartNum = 0x000,
    idcodeManufId = 0x489,
    debugIdleCycles = 5)
  case SerialTLKey => None // remove serialized tl port
})
// DOC include start: AbstractArty and Rocket
class WithArtyTweaks extends Config(
  new WithArtyJTAGHarnessBinder ++
  new WithArtyUARTHarnessBinder ++
  new WithArtyResetHarnessBinder ++
  new WithDebugResetPassthrough ++
  new WithDefaultPeripherals ++
  new freechips.rocketchip.subsystem.WithNBreakpoints(2))

class TinyRocketArtyConfig extends Config(
  new WithArtyTweaks ++
  new chipyard.TinyRocketConfig)

class TinyRocketArtySimConfig extends Config(
  new WithFPGASimSerial ++
  new testchipip.WithDefaultSerialTL ++
  new chipyard.harness.WithSimSerial ++
  new chipyard.harness.WithTiedOffDebug ++
  new TinyRocketArtyConfig)
// DOC include end: AbstractArty and Rocket
