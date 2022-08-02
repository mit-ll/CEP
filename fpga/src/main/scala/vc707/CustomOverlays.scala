//#************************************************************************
//# Copyright 2022 Massachusets Institute of Technology
//# SPDX short identifier: BSD-2-Clause
//#
//# File Name:      CustomOverlays.scala
//# Program:        Common Evaluation Platform (CEP)
//# Description:    Custom FPGA Shell overlays for VC707
//# Notes:          Modified from the Chipyard VCU118 CustomOverlays.scala
//#************************************************************************

package chipyard.fpga.vc707

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

class CustomGPIOVC707PlacedOverlay(val shell: VC707Shell, name: String, val designInput: GPIODesignInput, val shellInput: GPIOShellInput, gpioNames: Seq[String])
   extends GPIOXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {
    require(gpioNames.length == io.gpio.length)

    val packagePinsWithIOStdWithPackageIOs = (gpioNames zip io.gpio).map { case (name, io) =>
      val (pin, iostd, pullupEnable) = VC707GPIOs.pinMapping(name)
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

class CustomGPIOVC707ShellPlacer(shell: VC707Shell, val shellInput: GPIOShellInput, gpioNames: Seq[String])(implicit val valName: ValName)
  extends GPIOShellPlacer[VC707Shell] {
  def place(designInput: GPIODesignInput) = new CustomGPIOVC707PlacedOverlay(shell, valName.name, designInput, shellInput, gpioNames)
}
