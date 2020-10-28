//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : aes.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog AES core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.aes

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
case object PeripheryAESKey extends Field[Seq[AESParams]]

trait HasPeripheryAES { this: BaseSubsystem =>
  val AESNodes = p(PeripheryAESKey).map { ps =>
    AES.attach(AESAttachParams(ps, pbus))
  }
}

case class AESParams(address: BigInt)

case class AESAttachParams(
  aesparams         : AESParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLAES(busWidthBytes: Int, params: AESParams)(implicit p: Parameters)
  extends AES(busWidthBytes, params) with HasTLControlRegMap

object AES {

  def attach(params: AESAttachParams): TLAES = {
    implicit val p = params.p
    val aes = LazyModule(new TLAES(params.controlBus.beatBytes, params.aesparams))

    params.controlBus.coupleTo(s"slave_named_aes") {
      aes.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { aes.module.clock := params.controlBus.module.clock }
    InModuleBody { aes.module.reset := params.controlBus.module.reset }

    aes
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: AES TileLink Module
//--------------------------------------------------------------------------------------
abstract class AES(busWidthBytes: Int, val c: AESParams)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "aes",
        compat = Seq("mitll,aes"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "aes").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Instantitate the AES blackbox
            val blackbox = Module(new aes_192)

            // Instantiate registers for the blackbox inputs
            val start                   = RegInit(0.U(1.W))

            // Class and companion Object to support instantiation and initialization of
            // state due to the need to have subword assignment for vectors > 64-bits
            // 
            // Fields in the bundle enumerate from to low.  When converting this bundle
            // to UInt it is equivalent to Cat(Word1, Word0) or if subword assignment
            // was supported:
            //
            //      state(127,64) := word0
            //      state(63,0)   := word1
            //
            class State_Class extends Bundle {
                val word0               = UInt(64.W)	    
                val word1               = UInt(64.W)
            }
            object State_Class {
                def init: State_Class = {
                    val wire = Wire(new State_Class)
                    wire.word0          := 0.U		    
                    wire.word1          := 0.U
                    wire
                }
            }
            val state                   = RegInit(State_Class.init)

            // Class and companion Object to support instantiation and initialization of
            // key due to the need to have subword assignment for vectors > 64-bits
            class Key_Class extends Bundle {
                val word0               = UInt(64.W)
                val word1               = UInt(64.W)
                val word2               = UInt(64.W)
            }
            object Key_Class {
                def init: Key_Class = {
                    val wire = Wire(new Key_Class)
                    wire.word0          := 0.U
                    wire.word1          := 0.U
                    wire.word2          := 0.U
                    wire
                }
            }
            val key                     = RegInit(Key_Class.init)

            // Instantiate wires for the blackbox outputs
            val out0                    = Wire(UInt(64.W))
            val out1                    = Wire(UInt(64.W))
            val out_valid               = Wire(Bool())

            // Map the blackbox I/O 
            blackbox.io.clk             := clock                    // Implicit module clock
            blackbox.io.rst             := reset                    // Implicit module reset
            blackbox.io.start           := start                    // Positive edge already exists in AES core (aes_192.v)
            blackbox.io.state           := state.asUInt             // Convert State_Class bundle to UInt(bit vector)
            blackbox.io.key             := key.asUInt               // Convert Key_Class bundle to UInt(bit vector)
            out0                        := blackbox.io.out(127,64)  // Reading subwords in Chisel is OK	    
            out1                        := blackbox.io.out(63,0)    // Reading subwords in Chisel is OK
            out_valid                   := blackbox.io.out_valid    // Out is valid until start is again asserted

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                AESAddresses.aes_ctrlstatus_addr -> RegFieldGroup("aes_ctrlstatus", Some("AES Control/Status Register"),Seq(
                    RegField    (1, start,      RegFieldDesc("start", "")),
                    RegField.r  (1, out_valid,  RegFieldDesc("out_valid", "", volatile=true)))),
                AESAddresses.aes_pt0_addr -> RegFieldGroup("aes_pt1", Some("AES Plaintext Word 1"), Seq(RegField(64, state.word0))),
                AESAddresses.aes_pt1_addr -> RegFieldGroup("aes_pt1", Some("AES Plaintext Word 1"), Seq(RegField(64, state.word1))),
                AESAddresses.aes_ct0_addr -> RegFieldGroup("aes_ct0", Some("AES Ciphertext Word 0"), Seq(RegField.r(64, out0))),
                AESAddresses.aes_ct1_addr -> RegFieldGroup("aes_ct1", Some("AES Ciphertext Word 1"), Seq(RegField.r(64, out1))),
                AESAddresses.aes_key0_addr -> RegFieldGroup("aes_key0", Some("AES Key Word 0"), Seq(RegField(64, key.word0))),
                AESAddresses.aes_key1_addr -> RegFieldGroup("aes_key1", Some("AES Key Word 1"), Seq(RegField(64, key.word1))),
                AESAddresses.aes_key2_addr -> RegFieldGroup("aes_key2", Some("AES Key Word 2"), Seq(RegField(64, key.word2)))
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
class aes_192() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk         = Clock(INPUT)
    val rst         = Bool(INPUT)

    // Inputs
    val start       = Bool(INPUT)
    val state       = Bits(INPUT,128)
    val key         = Bits(INPUT,192)

      // Outputs
    val out         = Bits(OUTPUT,128)
    val out_valid   = Bool(OUTPUT)
  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
