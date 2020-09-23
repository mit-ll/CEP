//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
//
// File         : des3.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog des3 core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.des3

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
case object PeripheryDES3Key extends Field[Seq[DES3Params]]

trait HasPeripheryDES3 { this: BaseSubsystem =>
  val DES3Nodes = p(PeripheryDES3Key).map { ps =>
    DES3.attach(DES3AttachParams(ps, pbus))
  }
}

case class DES3Params(address: BigInt)

case class DES3AttachParams(
  des3params        : DES3Params,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLDES3(busWidthBytes: Int, params: DES3Params)(implicit p: Parameters)
  extends DES3(busWidthBytes, params) with HasTLControlRegMap

object DES3 {

  def attach(params: DES3AttachParams): TLDES3 = {
    implicit val p = params.p
    val des3 = LazyModule(new TLDES3(params.controlBus.beatBytes, params.des3params))

    params.controlBus.coupleTo(s"slave_named_des3") {
      des3.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { des3.module.clock := params.controlBus.module.clock }
    InModuleBody { des3.module.reset := params.controlBus.module.reset }

    des3
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: DES3 TileLink Module
//--------------------------------------------------------------------------------------
abstract class DES3(busWidthBytes: Int, val c: DES3Params)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "des3",
        compat = Seq("mitll,des3"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "des3").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Instantitate the DES3 blackbox
            val blackbox = Module(new des3)

            // Instantiate registers for the blackbox inputs
            val start                   = RegInit(0.U(1.W))
            val decrypt                 = RegInit(0.U(1.W))
            val key1                    = RegInit(0.U(56.W))
            val key2                    = RegInit(0.U(56.W))
            val key3                    = RegInit(0.U(56.W))  
            val desIn                   = RegInit(0.U(64.W))


            // Instantiate wires for the blackbox outputs
            val desOut                  = Wire(UInt(64.W))
            val out_valid               = Wire(Bool())

            // Map the blackbox I/O 
            blackbox.io.clk             := clock                    // Implicit module clock
            blackbox.io.reset           := reset                    // Implicit module reset
            blackbox.io.start           := start                    // DES3 core start bit
            blackbox.io.decrypt         := decrypt                  // Decrypt/encrypt bit
            blackbox.io.desIn           := desIn                    // Input data for encryption/decryption
            blackbox.io.key1            := key1                     // DES3 key without parity bits
            blackbox.io.key2            := key2                     // DES3 key without parity bits
            blackbox.io.key3            := key3                     // DES3 key without parity bits
            desOut                      := blackbox.io.desOut       // Output data that has been encrypted/decrypted
            out_valid                   := blackbox.io.out_valid    // Output valid bit

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                DES3Addresses.des3_ctrlstatus_addr -> RegFieldGroup("des3_ctrlstatus", Some("DES3 Control/Status Register"),Seq(RegField  ( 1, start, RegFieldDesc("start", "")))),
                DES3Addresses.des3_decrypt_addr    -> RegFieldGroup("des3_decrypt",    Some("DES3 Decrypt Bit"),            Seq(RegField  ( 1, decrypt))),
                DES3Addresses.des3_desIn_addr      -> RegFieldGroup("des3_desIn",      Some("DES3 Plaintext Word 1"),       Seq(RegField  (64, desIn))),
                DES3Addresses.des3_key1_addr       -> RegFieldGroup("des3_key0",       Some("DES3 Key Word 0"),             Seq(RegField  (56, key1))),
                DES3Addresses.des3_key2_addr       -> RegFieldGroup("des3_key1",       Some("DES3 Key Word 1"),             Seq(RegField  (56, key2))),
                DES3Addresses.des3_key3_addr       -> RegFieldGroup("des3_key2",       Some("DES3 Key Word 2"),             Seq(RegField  (56, key3))),                
                DES3Addresses.des3_done            -> RegFieldGroup("des3_done",       Some("DES3 Done"),                   Seq(RegField.r( 1, out_valid))),
                DES3Addresses.des3_desOut_addr     -> RegFieldGroup("des3_desOut",     Some("DES3 Plaintext Word 1"),       Seq(RegField.r(64, desOut)))
            )  // regmap

        }   // lazy val moduledes3_done
    }  // abstract class DES3
//--------------------------------------------------------------------------------------
// END: DES3 TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Black box wrapper for Verilog Module
//
// Note: Name must match Verilog module name, signal names
//   declared within much match the name, width, and direction of
//   the Verilog module.
//--------------------------------------------------------------------------------------
class des3() extends BlackBox {

  val io = IO(new Bundle {
    // Clock
    val clk         = Clock(INPUT)
    val reset       = Bool(INPUT)

    // Inputs
    val start       = Bool(INPUT)
    val decrypt     = Bool(INPUT)
    val key1        = Bits(INPUT,56)
    val key2        = Bits(INPUT,56)
    val key3        = Bits(INPUT,56)    
    val desIn       = Bits(INPUT,64)

      // Outputs
    val desOut      = Bits(OUTPUT,64)
    val out_valid   = Bool(OUTPUT)
  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
