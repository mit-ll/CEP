//#************************************************************************
//# Copyright 2022 Massachusets Institute of Technology
//# SPDX short identifier: BSD-2-Clause
//#
//# File Name:      CustomOverlays.scala
//# Program:        Common Evaluation Platform (CEP)
//# Description:    Custom FPGA Shell overlays for Arty100T
//# Notes:          Modified from the Chipyard VCU118 CustomOverlays.scala
//#************************************************************************

package chipyard.fpga.arty100t

import chisel3._
import chisel3.experimental.{attach}

import freechips.rocketchip.diplomacy._
import freechips.rocketchip.config.{Parameters, Field}
import freechips.rocketchip.tilelink.{TLInwardNode, TLAsyncCrossingSink}

import sifive.fpgashells.shell._
import sifive.fpgashells.ip.xilinx._
import sifive.fpgashells.shell.xilinx._
import sifive.fpgashells.clocks._

/* Connect GPIOs to FPGA I/Os */
abstract class GPIOXilinxPlacedOverlay(name: String, di: GPIODesignInput, si: GPIOShellInput)
  extends GPIOPlacedOverlay(name, di, si)
{
  def shell: XilinxShell

  shell { InModuleBody {
    (io.gpio zip tlgpioSink.bundle.pins).map { case (ioPin, sinkPin) =>
      val iobuf = Module(new IOBUF)
      iobuf.suggestName(s"gpio_iobuf")
      attach(ioPin, iobuf.io.IO)
      sinkPin.i.ival := iobuf.io.O
      iobuf.io.T := !sinkPin.o.oe
      iobuf.io.I := sinkPin.o.oval
    }
  } }
}

class CustomGPIOArty100TPlacedOverlay(val shell: Arty100TShellBasicOverlays, name: String, val designInput: GPIODesignInput, val shellInput: GPIOShellInput, gpioNames: Seq[String])
   extends GPIOXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {
    require(gpioNames.length == io.gpio.length)

    val packagePinsWithIOStdWithPackageIOs = (gpioNames zip io.gpio).map { case (name, io) =>
      val (pin, iostd, pullupEnable) = Arty100TGPIOs.pinMapping(name)
      (pin, iostd, pullupEnable, IOPin(io))
    }

    packagePinsWithIOStdWithPackageIOs foreach { case (pin, iostd, pullupEnable, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, iostd)
      if (iostd == "LVCMOS12") { shell.xdc.addDriveStrength(io, "8") }
      if (pullupEnable) { shell.xdc.addPullup(io) }
    } }
  } }
}

class CustomGPIOArty100TShellPlacer(shell: Arty100TShellBasicOverlays, val shellInput: GPIOShellInput, gpioNames: Seq[String])(implicit val valName: ValName)
  extends GPIOShellPlacer[Arty100TShellBasicOverlays] {
  def place(designInput: GPIODesignInput) = new CustomGPIOArty100TPlacedOverlay(shell, valName.name, designInput, shellInput, gpioNames)
}
