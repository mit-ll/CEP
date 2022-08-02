package chipyard.fpga.vc707

import chisel3._
import chisel3.experimental.{BaseModule}

import freechips.rocketchip.util.{HeterogeneousBag}
import freechips.rocketchip.tilelink.{TLBundle}

import sifive.blocks.devices.uart.{HasPeripheryUARTModuleImp, UARTPortIO}
import sifive.blocks.devices.spi.{HasPeripherySPI, SPIPortIO}
import sifive.blocks.devices.gpio.{HasPeripheryGPIOModuleImp, GPIOPortIO}

import chipyard.{HasHarnessSignalReferences, CanHaveMasterTLMemPort}
import chipyard.harness.{OverrideHarnessBinder}

import testchipip._

/*** UART ***/
class WithUART extends OverrideHarnessBinder({
  (system: HasPeripheryUARTModuleImp, th: BaseModule with HasHarnessSignalReferences, ports: Seq[UARTPortIO]) => {
    th match { case vc707th: VC707FPGATestHarnessImp => {
      vc707th.vc707Outer.io_uart_bb.bundle <> ports.head
    } }
  }
})

/*** SPI ***/
class WithSPISDCard extends OverrideHarnessBinder({
  (system: HasPeripherySPI, th: BaseModule with HasHarnessSignalReferences, ports: Seq[SPIPortIO]) => {
    th match { case vc707th: VC707FPGATestHarnessImp => {
      vc707th.vc707Outer.io_spi_bb.bundle <> ports.head
    } }
  }
})

/*** GPIO ***/
class WithGPIO extends OverrideHarnessBinder({
  (system: HasPeripheryGPIOModuleImp, th: BaseModule with HasHarnessSignalReferences, ports: Seq[GPIOPortIO]) => {
    th match { case vc707th: VC707FPGATestHarnessImp => {
      (vc707th.vc707Outer.io_gpio_bb zip ports).map { case (bb_io, dut_io) =>
        bb_io.bundle <> dut_io
      }
    } }
  }
})

/*** Experimental DDR ***/
class WithDDRMem extends OverrideHarnessBinder({
  (system: CanHaveMasterTLMemPort, th: BaseModule with HasHarnessSignalReferences, ports: Seq[HeterogeneousBag[TLBundle]]) => {
    th match { case vc707th: VC707FPGATestHarnessImp => {
      require(ports.size == 1)

      val bundles = vc707th.vc707Outer.ddrClient.out.map(_._1)
      val ddrClientBundle = Wire(new HeterogeneousBag(bundles.map(_.cloneType)))
      bundles.zip(ddrClientBundle).foreach { case (bundle, io) => bundle <> io }
      ddrClientBundle <> ports.head
    } }
  }
})

