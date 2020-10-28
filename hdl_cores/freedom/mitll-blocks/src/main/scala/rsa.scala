//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : rsa.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog AES core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.rsa

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
case object PeripheryRSAKey extends Field[Seq[RSAParams]]

trait HasPeripheryRSA { this: BaseSubsystem =>
  val RSANodes = p(PeripheryRSAKey).map { ps =>
    RSA.attach(RSAAttachParams(ps, pbus))
  }
}

case class RSAParams(address: BigInt)

case class RSAAttachParams(
  rsaparams         : RSAParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLRSA(busWidthBytes: Int, params: RSAParams)(implicit p: Parameters)
  extends RSA(busWidthBytes, params) with HasTLControlRegMap

object RSA {

  def attach(params: RSAAttachParams): TLRSA = {
    implicit val p = params.p
    val rsa = LazyModule(new TLRSA(params.controlBus.beatBytes, params.rsaparams))

    params.controlBus.coupleTo(s"slave_named_rsa") {
      rsa.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { rsa.module.clock := params.controlBus.module.clock }
    InModuleBody { rsa.module.reset := params.controlBus.module.reset }

    rsa
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: rsa TileLink Module
//--------------------------------------------------------------------------------------
abstract class RSA(busWidthBytes: Int, val c: RSAParams)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "rsa",
        compat = Seq("mitll,rsa"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "rsa").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Macro definition for creating rising edge detectors
            def rising_edge(x: Bool)    = x && !RegNext(x)

            // Instantitate the rsa blackbox
            val blackbox = Module(new modexp_core)

            // Instantiate registers for the blackbox inputs
            
            val st                                   = RegInit(0.U(1.W))    // Start bit
            val exp_len                              = RegInit(0.U(8.W))    // Exp length
            val mod_len                              = RegInit(0.U(8.W))    // Mod length

            val ready                                = RegInit(0.U(1.W))    // Ready bit drive by modexp_core.v
            val cycles                               = RegInit(0.U(64.W))   // Cycles count driven by modexp_core.v

            val exp_mem_api_write_data               = RegInit(0.U(32.W))   // Exp memory write data
            val exp_mem_api_read_data                = RegInit(0.U(32.W))   // Exp memory read data
            val exp_mem_api_ctrl                     = RegInit(0.U(3.W))    // Exp memory control register
            val exp_mem_api_ctrl_wire1               = Wire(Bool())         // Exp memory chip select signal
            val exp_mem_api_ctrl_wire2               = Wire(Bool())         // Exp memory write enable signal
            val exp_mem_api_ctrl_wire3               = Wire(Bool())         // Exp memory pointer reset signal 

            val mod_mem_api_write_data               = RegInit(0.U(32.W))   // Mod memory write data   
            val mod_mem_api_read_data                = RegInit(0.U(32.W))   // Mod memory read data
            val mod_mem_api_ctrl                     = RegInit(0.U(3.W))    // Mod memory control register
            val mod_mem_api_ctrl_wire1               = Wire(Bool())         // Mod memory chip select signal
            val mod_mem_api_ctrl_wire2               = Wire(Bool())         // Mod memory write enable signal
            val mod_mem_api_ctrl_wire3               = Wire(Bool())         // Mod memory pointer reset signal 

            val mess_mem_api_write_data              = RegInit(0.U(32.W))   // Message memory write data
            val mess_mem_api_read_data               = RegInit(0.U(32.W))   // Message memory read data
            val mess_mem_api_ctrl                    = RegInit(0.U(3.W))    // Message memory control register
            val mess_mem_api_ctrl_wire1              = Wire(Bool())         // Message memory chip select signal          
            val mess_mem_api_ctrl_wire2              = Wire(Bool())         // Message memory write enable signal
            val mess_mem_api_ctrl_wire3              = Wire(Bool())         // Message memory pointer reset signal 

            val res_mem_api_read_data                = RegInit(0.U(32.W))   // Result memory read data
            val res_mem_api_ctrl                     = RegInit(0.U(2.W))    // Result memory control register
            val res_mem_api_ctrl_wire1               = Wire(Bool())         // Result memory chip select signal
            val res_mem_api_ctrl_wire2               = Wire(Bool())         // Result memory pointer reset signal 

           
            exp_mem_api_ctrl_wire1 := exp_mem_api_ctrl(0)
            exp_mem_api_ctrl_wire2 := exp_mem_api_ctrl(1)
            exp_mem_api_ctrl_wire3 := exp_mem_api_ctrl(2)

            mod_mem_api_ctrl_wire1 := mod_mem_api_ctrl(0)
            mod_mem_api_ctrl_wire2 := mod_mem_api_ctrl(1)
            mod_mem_api_ctrl_wire3 := mod_mem_api_ctrl(2)


            mess_mem_api_ctrl_wire1 := mess_mem_api_ctrl(0)
            mess_mem_api_ctrl_wire2 := mess_mem_api_ctrl(1)
            mess_mem_api_ctrl_wire3 := mess_mem_api_ctrl(2)

            res_mem_api_ctrl_wire1 := res_mem_api_ctrl(0)
            res_mem_api_ctrl_wire2 := res_mem_api_ctrl(1)


            // Map the inputs to the blackbox
            blackbox.io.clk                       := clock                    // Implicit module clock
            blackbox.io.reset_n                   := ~reset                   // Implicit module reset                      
            blackbox.io.start                     := st    
            blackbox.io.exponent_length           := exp_len
            blackbox.io.modulus_length            := mod_len

            // Map ctrl outputs from blackbox
            ready := blackbox.io.ready
            cycles := blackbox.io.cycles            
            
            //Exponent Inteferace
            blackbox.io.exponent_mem_api_cs         := rising_edge(exp_mem_api_ctrl_wire1)
            blackbox.io.exponent_mem_api_wr         := rising_edge(exp_mem_api_ctrl_wire2)  
            blackbox.io.exponent_mem_api_rst        := rising_edge(exp_mem_api_ctrl_wire3) 
            blackbox.io.exponent_mem_api_write_data := exp_mem_api_write_data
            exp_mem_api_read_data                   := blackbox.io.exponent_mem_api_read_data

            //Modulus Inteferace
            blackbox.io.modulus_mem_api_cs          := rising_edge(mod_mem_api_ctrl_wire1) 
            blackbox.io.modulus_mem_api_wr          := rising_edge(mod_mem_api_ctrl_wire2) 
            blackbox.io.modulus_mem_api_rst         := rising_edge(mod_mem_api_ctrl_wire3) 
            blackbox.io.modulus_mem_api_write_data  := mod_mem_api_write_data
            mod_mem_api_read_data                   := blackbox.io.modulus_mem_api_read_data

            //Message Inteferace
            blackbox.io.message_mem_api_cs          := rising_edge(mess_mem_api_ctrl_wire1) 
            blackbox.io.message_mem_api_wr          := rising_edge(mess_mem_api_ctrl_wire2)   
            blackbox.io.message_mem_api_rst         := rising_edge(mess_mem_api_ctrl_wire3) 
            blackbox.io.message_mem_api_write_data  := mess_mem_api_write_data
            mess_mem_api_read_data                  := blackbox.io.message_mem_api_read_data

            //Result Inteferace
            blackbox.io.result_mem_api_cs          :=  rising_edge(res_mem_api_ctrl_wire1) 
            blackbox.io.result_mem_api_rst         :=  rising_edge(res_mem_api_ctrl_wire2) 

            when (rising_edge(res_mem_api_ctrl_wire1) ) {
                res_mem_api_read_data := blackbox.io.result_mem_api_read_data
            }    
            


            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                RSAAddresses.rsa_ctrlstatus_addr  -> RegFieldGroup("rsa_ready", Some("rsa_ready Register"),Seq(RegField.r(1,  ready),
                                                                                                               RegField  (1,  st))),
                RSAAddresses.rsa_exp_data_addr    ->    Seq(RegField   (32, exp_mem_api_write_data),
						  	    RegField.r (32, exp_mem_api_read_data)), // [63;32]
                RSAAddresses.rsa_exp_ctrl_addr    ->    Seq(RegField   (3 , exp_mem_api_ctrl)),

                RSAAddresses.rsa_mod_data         ->    Seq(RegField   (32, mod_mem_api_write_data),
						  	    RegField.r (32, mod_mem_api_read_data)), // [63:32]
                RSAAddresses.rsa_mod_ctrl_addr    ->    Seq(RegField   (3,  mod_mem_api_ctrl)),

                RSAAddresses.rsa_message_data     ->    Seq(RegField   (32, mess_mem_api_write_data),
						  	    RegField.r (32, mess_mem_api_read_data)), // [63:32]		
                RSAAddresses.rsa_message_ctrl_addr->    Seq(RegField  (3,  mess_mem_api_ctrl)),    

                RSAAddresses.rsa_mod_length       ->    Seq(RegField  (8 , mod_len)),
                RSAAddresses.rsa_exp_length       ->    Seq(RegField  (13 , exp_len)),

                RSAAddresses.rsa_result_data_addr ->    Seq(RegField.r(32, res_mem_api_read_data)),
                RSAAddresses.rsa_result_ctrl_addr ->    Seq(RegField  (2,  res_mem_api_ctrl)),  

                RSAAddresses.rsa_cycles_addr      ->    Seq(RegField.r(64, cycles))
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
class modexp_core() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk             = Clock(INPUT)
    val reset_n         = Bool(INPUT)

    // Control/Status
        //Inputs
    val start    = Bool(INPUT)
    val exponent_length = Bits(INPUT,13)
    val modulus_length = Bits(INPUT,8)
        //Outputs
    val ready    = Bool(OUTPUT)
    val cycles = Bits(OUTPUT, 64)

    //Exponent Interface
        //Inputs
    val exponent_mem_api_cs = Bool(INPUT)
    val exponent_mem_api_wr = Bool(INPUT)
    val exponent_mem_api_rst = Bool(INPUT)
    val exponent_mem_api_write_data = Bits(INPUT,32)
        //Outputs
    val exponent_mem_api_read_data = Bits(OUTPUT,32)

    //Modulus Inteferace
        //Inputs
    val modulus_mem_api_cs = Bool(INPUT)
    val modulus_mem_api_wr = Bool(INPUT)
    val modulus_mem_api_rst = Bool(INPUT)
    val modulus_mem_api_write_data = Bits(INPUT,32)
        //Outputs
    val modulus_mem_api_read_data = Bits(OUTPUT,32)

    //Message Inteferace
        //Inputs
    val message_mem_api_cs = Bool(INPUT)
    val message_mem_api_wr = Bool(INPUT)
    val message_mem_api_rst = Bool(INPUT)
    val message_mem_api_write_data = Bits(INPUT,32)
        //Outputs
    val message_mem_api_read_data = Bits(OUTPUT,32)

    //Result interface
        //Inputs
    val result_mem_api_cs = Bool(INPUT)
    val result_mem_api_rst = Bool(INPUT)
        //Outputs
    val result_mem_api_read_data = Bits(OUTPUT,32)

  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
