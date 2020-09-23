//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
//
// File         : sha256.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog AES core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.sha256

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
case object PeripherySHA256Key extends Field[Seq[SHA256Params]]

trait HasPeripherySHA256 { this: BaseSubsystem =>
  val SHA256Nodes = p(PeripherySHA256Key).map { ps =>
    SHA256.attach(SHA256AttachParams(ps, pbus))
  }
}

case class SHA256Params(address: BigInt)

case class SHA256AttachParams(
  sha256params         : SHA256Params,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLSHA256(busWidthBytes: Int, params: SHA256Params)(implicit p: Parameters)
  extends SHA256(busWidthBytes, params) with HasTLControlRegMap

object SHA256 {

  def attach(params: SHA256AttachParams): TLSHA256 = {
    implicit val p = params.p
    val sha256 = LazyModule(new TLSHA256(params.controlBus.beatBytes, params.sha256params))

    params.controlBus.coupleTo(s"slave_named_sha256") {
      sha256.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { sha256.module.clock := params.controlBus.module.clock }
    InModuleBody { sha256.module.reset := params.controlBus.module.reset }

    sha256
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: sha256 TileLink Module
//--------------------------------------------------------------------------------------
abstract class SHA256(busWidthBytes: Int, val c: SHA256Params)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "sha256",
        compat = Seq("mitll,sha256"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "sha256").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Macro definition for creating rising edge detectors
            def rising_edge(x: Bool)    = x && !RegNext(x)

            // Instantitate the sha256 blackbox
            val blackbox = Module(new sha256)

            // Instantiate registers for the blackbox inputs

            val rst                            = RegInit(0.U(1.W))
            val next                           = RegInit(false.B)
            val init                           = RegInit(false.B)


            // Class and companion Object to support instantiation and initialization of
            // state due to the need to have subword assignment for vectors > 64-bits
            // 
            // Fields in the bundle enumerate from to low.  When converting this bundle
            // to UInt it is equivalent to Cat(Word1, Word0) or if subword assignment
            // was supported.

            // Class and companion Object to support instantiation and initialization of
            // key due to the need to have subword assignment for vectors > 64-bits
	    // change to BE format to conform to stadardized from CPE's SW perspective
            class block_Class extends Bundle {
                val word0               = UInt(64.W)
                val word1               = UInt(64.W)
                val word2               = UInt(64.W)
                val word3               = UInt(64.W)
                val word4               = UInt(64.W)
                val word5               = UInt(64.W)
                val word6               = UInt(64.W)
                val word7               = UInt(64.W)
            }
            object block_Class {
                def init: block_Class = {
                    val wire = Wire(new block_Class)
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
            val block                   = RegInit(block_Class.init)


            // Instantiate wires for the blackbox outputs
            val digest0                    = Wire(UInt(64.W))
            val digest1                    = Wire(UInt(64.W))
            val digest2                    = Wire(UInt(64.W))
            val digest3                    = Wire(UInt(64.W))
            val ready                          = Wire(Bool())
            val digest_valid                   = Wire(Bool())

            // Map the inputs to the blackbox
            blackbox.io.clk             := clock                    // Implicit module clock
            blackbox.io.rst             := reset                    // Implicit module reset                      
            blackbox.io.next            := rising_edge(next)        // Next rising edge generated from addressable registers    
            blackbox.io.block           := block.asUInt             // Block input data
            blackbox.io.init            := rising_edge(init)        // Init rising edge generated from addressable registers

            // Map the outputs from the blockbox
            digest0                 := blackbox.io.digest(255,192)  // Output data bits
            digest1                 := blackbox.io.digest(191,128)  // Output data bits
            digest2                 := blackbox.io.digest(127,64)   // Output data bits
            digest3                 := blackbox.io.digest(63,0)     // Output data bits
            digest_valid            := blackbox.io.digest_valid     // Output data valid bit
            ready                   := blackbox.io.ready            // Ready bit

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                SHA256Addresses.sha256_ctrlstatus_addr ->RegFieldGroup("sha256_ready", Some("sha256_ready Register"),Seq(RegField.r(1,  ready),
                                                                                                                         RegField  (1,  init),
                                                                                                                         RegField  (1,  next))),
                SHA256Addresses.sha256_block_w0 -> RegFieldGroup("sha256 0", Some("sha256 msg input word 0"),        Seq(RegField  (64, block.word0))),
                SHA256Addresses.sha256_block_w1 -> RegFieldGroup("sha256 0", Some("sha256 msg input word 1"),        Seq(RegField  (64, block.word1))),
                SHA256Addresses.sha256_block_w2 -> RegFieldGroup("sha256 2", Some("sha256 msg input word 2"),        Seq(RegField  (64, block.word2))),
                SHA256Addresses.sha256_block_w3 -> RegFieldGroup("sha256 3", Some("sha256 msg input word 3"),        Seq(RegField  (64, block.word3))),
                SHA256Addresses.sha256_block_w4 -> RegFieldGroup("sha256 4", Some("sha256 msg input word 4"),        Seq(RegField  (64, block.word4))),
                SHA256Addresses.sha256_block_w5 -> RegFieldGroup("sha256 5", Some("sha256 msg input word 5"),        Seq(RegField  (64, block.word5))),
                SHA256Addresses.sha256_block_w6 -> RegFieldGroup("sha256 6", Some("sha256 msg input word 6"),        Seq(RegField  (64, block.word6))),
                SHA256Addresses.sha256_block_w7 -> RegFieldGroup("sha256 7", Some("sha256 msg input word 7"),        Seq(RegField  (64, block.word7))),
                SHA256Addresses.sha256_done     -> RegFieldGroup("sha256 done", Some("sha256 done"),                 Seq(RegField.r(1,  digest_valid))),
                SHA256Addresses.sha256_digest_w0 -> RegFieldGroup("sha256 msg output0", Some("sha256 msg output1"),  Seq(RegField.r(64, digest0))),
                SHA256Addresses.sha256_digest_w1 -> RegFieldGroup("sha256 msg output1", Some("sha256 msg output1"),  Seq(RegField.r(64, digest1))),
                SHA256Addresses.sha256_digest_w2 -> RegFieldGroup("sha256 msg output0", Some("sha256 msg output1"),  Seq(RegField.r(64, digest2))),
                SHA256Addresses.sha256_digest_w3 -> RegFieldGroup("sha256 msg output1", Some("sha256 msg output1"),  Seq(RegField.r(64, digest3)))
               
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
class sha256() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk         = Clock(INPUT)
    val rst         = Bool(INPUT)

    // Inputs
    val init    = Bool(INPUT)
    val next    = Bool(INPUT)

    val block   = Bits(INPUT,512)

      // Outputs
    val digest_valid    = Bool(OUTPUT)
    val digest          = Bits(OUTPUT,256)
    val ready           = Bool(OUTPUT)
  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
