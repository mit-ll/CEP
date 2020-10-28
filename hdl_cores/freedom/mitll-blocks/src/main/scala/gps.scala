//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : gps.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog GPS core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.gps

import Chisel._
import freechips.rocketchip.config.Field
import freechips.rocketchip.subsystem.{BaseSubsystem, PeripheryBusKey}
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.regmapper._
import freechips.rocketchip.tilelink._

import mitllBlocks.cep_addresses._

//--------------------------------------------------------------------------------------
// BEGIN: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------
case object PeripheryGPSKey extends Field[Seq[GPSParams]]

trait HasPeripheryGPS { this: BaseSubsystem =>
  val GPSNodes = p(PeripheryGPSKey).map { ps =>
    GPS.attach(GPSAttachParams(ps, pbus))
  }
}

case class GPSParams(address: BigInt)

case class GPSAttachParams(
  gpsparams         : GPSParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLGPS(busWidthBytes: Int, params: GPSParams)(implicit p: Parameters)
  extends GPS(busWidthBytes, params) with HasTLControlRegMap

object GPS {

  def attach(params: GPSAttachParams): TLGPS = {
    implicit val p = params.p
    val gps = LazyModule(new TLGPS(params.controlBus.beatBytes, params.gpsparams))

    params.controlBus.coupleTo(s"slave_named_gps") {
      gps.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { gps.module.clock := params.controlBus.module.clock }
    InModuleBody { gps.module.reset := params.controlBus.module.reset }

    gps
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: GPS TileLink Module
//--------------------------------------------------------------------------------------
abstract class GPS(busWidthBytes: Int, val c: GPSParams)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "gps",
        compat = Seq("mitll,gps"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "gps").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {
            
            // Instantitate the GPS blackbox
            val blackbox = Module(new gps)

            // Instantiate registers for the blackbox inputs
            val startRound                   = RegInit(0.U(1.W))
            val sv_num                       = RegInit(0.U(6.W))
            val gps_reset                    = RegInit(false.B)

            // Instantiate wires for the blackbox outputs
            val ca_code                      = Wire(UInt(13.W))
            val p_code0_u                    = Wire(UInt(32.W))
            val p_code0_l                    = Wire(UInt(32.W))
            val p_code1_u                    = Wire(UInt(32.W))
            val p_code1_l                    = Wire(UInt(32.W))
            val l_code0_u                    = Wire(UInt(32.W))
            val l_code0_l                    = Wire(UInt(32.W))
            val l_code1_u                    = Wire(UInt(32.W))
            val l_code1_l                    = Wire(UInt(32.W))
            val l_code_valid               = Wire(Bool())

            // Map the blackbox I/O 
            blackbox.io.sys_clk_50      := clock                     // Implicit module clock
            blackbox.io.sync_rst_in     := reset | gps_reset         // Implicit module reset
            blackbox.io.startRound      := startRound                // Start bit
            blackbox.io.sv_num          := sv_num                    // GPS space vehicle number written by cepregression.cpp
            ca_code                     := blackbox.io.ca_code       // Output GPS CA code
            p_code0_u                   := blackbox.io.p_code(127,96)// Output P Code bits 
            p_code0_l                   := blackbox.io.p_code(95,64) // Output P Code bits	    
            p_code1_u                   := blackbox.io.p_code(63,32) // Output P Code bits  	    
            p_code1_l                   := blackbox.io.p_code(31,0)  // Output P Code bits
            l_code0_u                   := blackbox.io.l_code(127,96)// Output L Code bits                        
            l_code0_l                   := blackbox.io.l_code(95,64) // Output L Code bits
            l_code1_u                   := blackbox.io.l_code(63,32) // Output L Code bits      
            l_code1_l                   := blackbox.io.l_code(31,0)  // Output L Code bits
	    //
            l_code_valid                := blackbox.io.l_code_valid  // Out is valid until start is again asserted

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                GPSAddresses.gps_ctrlstatus_addr -> RegFieldGroup("gps_ctrlstatus", Some("GPS Control/Status Register"),Seq(
                    RegField    (1, startRound,      RegFieldDesc("start", "")),
                    RegField.r  (1, l_code_valid,  RegFieldDesc ("l_code_valid", "", volatile=true)))),
                GPSAddresses.gps_sv_num_addr    -> RegFieldGroup("sv_num",     Some("GPS Set SV sv_num"),       Seq(RegField  (6,  sv_num))),
                GPSAddresses.gps_ca_code_addr   -> RegFieldGroup("gps_cacode", Some("GPS CA code"),             Seq(RegField.r(64, ca_code))),
                GPSAddresses.gps_reset_addr     -> RegFieldGroup("gps_reset",  Some("GPS addressable reset"),   Seq(RegField  (1,  gps_reset))),            
                GPSAddresses.gps_p_code_addr_w0 -> RegFieldGroup("gps_pcode1", Some("GPS pcode upper bits"), Seq(RegField.r(64, Cat(p_code0_u,p_code0_l)))),
                GPSAddresses.gps_p_code_addr_w1 -> RegFieldGroup("gps_pcode1", Some("GPS pcode lower 64 bits"), Seq(RegField.r(64, Cat(p_code1_u,p_code1_l)))),
                GPSAddresses.gps_l_code_addr_w0 -> RegFieldGroup("gps_lcode1", Some("GPS lcode upper 64 bits"), Seq(RegField.r(64, Cat(l_code0_u,l_code0_l)))),
                GPSAddresses.gps_l_code_addr_w1 -> RegFieldGroup("gps_lcode1", Some("GPS lcode lower 64 bits"), Seq(RegField.r(64, Cat(l_code1_u,l_code1_l))))
            )  // regmap

        }   // lazy val module
    }  // abstract class GPS
//--------------------------------------------------------------------------------------
// END: GPS TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Black box wrapper for Verilog Module
//
// Note: Name must match Verilog module name, signal names
//   declared within much match the name, width, and direction of
//   the Verilog module.
//--------------------------------------------------------------------------------------
class gps() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val sys_clk_50        = Clock(INPUT)
    val sync_rst_in       = Bool(INPUT)

    // Inputs
    val startRound        = Bool(INPUT)
    val sv_num            = Bits(INPUT,6)

      // Outputs
    val ca_code         = Bits(OUTPUT,13)
    val p_code          = Bits(OUTPUT,128)
    val l_code          = Bits(OUTPUT,128)
    val l_code_valid    = Bool(OUTPUT)
  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
