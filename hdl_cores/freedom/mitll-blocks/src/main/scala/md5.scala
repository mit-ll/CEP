//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : md5.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog AES core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.md5

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
case object PeripheryMD5Key extends Field[Seq[MD5Params]]

trait HasPeripheryMD5 { this: BaseSubsystem =>
  val MD5Nodes = p(PeripheryMD5Key).map { ps =>
    MD5.attach(MD5AttachParams(ps, pbus))
  }
}

case class MD5Params(address: BigInt)

case class MD5AttachParams(
  md5params         : MD5Params,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLMD5(busWidthBytes: Int, params: MD5Params)(implicit p: Parameters)
  extends MD5(busWidthBytes, params) with HasTLControlRegMap

object MD5 {

  def attach(params: MD5AttachParams): TLMD5 = {
    implicit val p = params.p
    val md5 = LazyModule(new TLMD5(params.controlBus.beatBytes, params.md5params))

    params.controlBus.coupleTo(s"slave_named_md5") {
      md5.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { md5.module.clock := params.controlBus.module.clock }
    InModuleBody { md5.module.reset := params.controlBus.module.reset }

    md5
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: md5 TileLink Module
//--------------------------------------------------------------------------------------
abstract class MD5(busWidthBytes: Int, val c: MD5Params)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "md5",
        compat = Seq("mitll,md5"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "md5").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Instantitate the md5 blackbox
//            val blackbox = Module(new pancham)
// use md5.v to conform to standardized BE format
	      val blackbox = Module(new md5)

            // Instantiate registers for the blackbox inputs
            val md5_msg_in_valid               = RegInit(0.U(1.W))
            val rst                            = RegInit(0.U(1.W))
	    // 
            val init                           = RegInit(0.U(1.W))
            // Class and companion Object to support instantiation and initialization of
            // state due to the need to have subword assignment for vectors > 64-bits
            // 
            // Fields in the bundle enumerate from to low.  When converting this bundle
            // to UInt it is equivalent to Cat(Word1, Word0) or if subword assignment
            // was supported.

            // Class and companion Object to support instantiation and initialization of
            // key due to the need to have subword assignment for vectors > 64-bits
            class Msgin_Class extends Bundle {
                val word0               = UInt(64.W)
                val word1               = UInt(64.W)
                val word2               = UInt(64.W)
                val word3               = UInt(64.W)
                val word4               = UInt(64.W)
                val word5               = UInt(64.W)
                val word6               = UInt(64.W)
                val word7               = UInt(64.W)
            }
            object Msgin_Class {
                def init: Msgin_Class = {
                    val wire = Wire(new Msgin_Class)
                    wire.word0          := 0.U
                    wire.word1          := 0.U
                    wire.word2          := 0.U
                    wire.word3          := 0.U
                    wire.word4          := 0.U
                    wire.word5          := 0.U
                    wire.word6          := 0.U
                    wire.word7          := 0.U
                    wire
                }
            }
            val msg_padded                   = RegInit(Msgin_Class.init)


            // Instantiate wires for the blackbox outputs
            val msg_output0                    = Wire(UInt(64.W))
            val msg_output1                    = Wire(UInt(64.W))
            val ready                          = Wire(Bool())
            val md5_msg_out_valid              = Wire(Bool())

            // Map the inputs to the blackbox
            blackbox.io.clk             := clock                    // Implicit module clock
            blackbox.io.rst             := reset | rst              // Implicit module reset or'ed with addressable reset
            blackbox.io.init            := init                     // Implicit module reset or'ed with addressable reset        	    
            blackbox.io.msg_in_valid    := md5_msg_in_valid         // Message input valid bit
            blackbox.io.msg_padded      := msg_padded.asUInt        // Message input
            // Map the outputs from the blockbox
            msg_output0                 := blackbox.io.msg_output(127,64)   // Message output bits
            msg_output1                 := blackbox.io.msg_output(63,0)     // Message output bits  	    
            md5_msg_out_valid           := blackbox.io.msg_out_valid        // Message output valid bit driven by pancham.v
            ready                       := blackbox.io.ready                // Ready bit driven by pancham.v

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                MD5Addresses.md5_ready         -> RegFieldGroup("md5_ready", Some("md5_ready Register"),    Seq(RegField.r(1,  ready           ))),
                MD5Addresses.md5_msg_padded_w0 -> RegFieldGroup("md5_in0", Some("md5 msg input word 0"),    Seq(RegField  (64, msg_padded.word0))),
                MD5Addresses.md5_msg_padded_w1 -> RegFieldGroup("md5_in1", Some("md5 msg input word 1"),    Seq(RegField  (64, msg_padded.word1))),
                MD5Addresses.md5_msg_padded_w2 -> RegFieldGroup("md5_in2", Some("md5 msg input word 2"),    Seq(RegField  (64, msg_padded.word2))),
                MD5Addresses.md5_msg_padded_w3 -> RegFieldGroup("md5_in3", Some("md5 msg input word 3"),    Seq(RegField  (64, msg_padded.word3))),
                MD5Addresses.md5_msg_padded_w4 -> RegFieldGroup("md5_in4", Some("md5 msg input word 4"),    Seq(RegField  (64, msg_padded.word4))),
                MD5Addresses.md5_msg_padded_w5 -> RegFieldGroup("md5_in5", Some("md5 msg input word 5"),    Seq(RegField  (64, msg_padded.word5))),
                MD5Addresses.md5_msg_padded_w6 -> RegFieldGroup("md5_in6", Some("md5 msg input word 6"),    Seq(RegField  (64, msg_padded.word6))),
                MD5Addresses.md5_msg_padded_w7 -> RegFieldGroup("md5_in7", Some("md5 msg input word 7"),    Seq(RegField  (64, msg_padded.word7))),
                MD5Addresses.md5_msg_output_w0 -> RegFieldGroup("md5 msg output0", Some("md5 msg output1"), Seq(RegField.r(64, msg_output0))),
                MD5Addresses.md5_msg_output_w1 -> RegFieldGroup("md5 msg output1", Some("md5 msg output1"), Seq(RegField.r(64, msg_output1))),
                MD5Addresses.md5_in_valid      -> RegFieldGroup("md5 msg in valid", Some("md5 in valid"),   Seq(RegField  (1,  md5_msg_in_valid))),
                MD5Addresses.md5_out_valid     -> RegFieldGroup("md5 msg out valid", Some("md5 out valid"), Seq(RegField.r(1,  md5_msg_out_valid))),
                MD5Addresses.md5_rst           -> RegFieldGroup("message_rst", Some("message_rst"),         Seq(RegField  (1, rst),
					       	  			       				        RegField  (1, init)))
            )  // regmap

        }   // lazy val module
    }  // abstract class AES
//--------------------------------------------------------------------------------------
// END: AES TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Black box wrapper for Verilog Module
//
// Note: Name must match Verilog module name, signal names
//   declared within much match the name, width, and direction of
//   the Verilog module.
//--------------------------------------------------------------------------------------
//class pancham() extends BlackBox {
// use md5.v to conform to standardized BE format
class md5() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk         = Clock(INPUT)
    val rst         = Bool(INPUT)

    // Added 05/12/2020: To clear internal states to start new transaction
    val init        = Bool(INPUT)

    // Inputs
    val msg_in_valid    = Bool(INPUT)
    val msg_padded      = Bits(INPUT,512)

      // Outputs
    val msg_output      = Bits(OUTPUT,128)
    val msg_out_valid   = Bool(OUTPUT)
    val ready           = Bool(OUTPUT)
  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
