//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : aes.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog AES core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.aes

import chisel3._
import chisel3.util._
import chisel3.experimental.{IntParam, BaseModule}
import freechips.rocketchip.config.Field
import freechips.rocketchip.subsystem.{BaseSubsystem, PeripheryBusKey}
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.regmapper._
import freechips.rocketchip.tilelink._

import mitllBlocks.cep_addresses._

//--------------------------------------------------------------------------------------
// BEGIN: Module "Periphery" connections
//--------------------------------------------------------------------------------------

// Parameters associated with the core
case object PeripheryAESKey extends Field[Seq[COREParams]](Nil)

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait CanHavePeripheryAES { this: BaseSubsystem =>
  val aesnode = p(PeripheryAESKey).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val aesmodule = LazyModule(new aesTLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      aesmodule.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
      aesmodule.llki_node :*= 
      TLSourceShrinker(16) :*=
      TLFragmenter(coreattachparams.llki_bus) :*=_
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { aesmodule.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { aesmodule.module.clock := coreattachparams.slave_bus.module.clock }

}}
//--------------------------------------------------------------------------------------
// END: Module "Periphery" connections
//--------------------------------------------------------------------------------------
 


//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class aesTLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

  // Create a Manager / Slave / Sink node
  // The OpenTitan-based Tilelink interfaces support 4 beatbytes only
  val llki_node = TLManagerNode(Seq(TLSlavePortParameters.v1(
    Seq(TLSlaveParameters.v1(
      address             = Seq(AddressSet(
                              coreattachparams.coreparams.llki_base_addr, 
                              coreattachparams.coreparams.llki_depth)),
      resources           = new SimpleDevice(coreattachparams.coreparams.dev_name + "-llki-slave", 
                              Seq("mitll," + coreattachparams.coreparams.dev_name + "-llki-slave")).reg,
      regionType          = RegionType.IDEMPOTENT,
      supportsGet         = TransferSizes(1, 8),
      supportsPutFull     = TransferSizes(1, 8),
      supportsPutPartial  = TransferSizes.none,
      supportsArithmetic  = TransferSizes.none,
      supportsLogical     = TransferSizes.none,
      fifoId              = Some(0))), // requests are handled in order
    beatBytes = coreattachparams.llki_bus.beatBytes)))

  // Create the RegisterRouter node
  val slave_node = TLRegisterNode(
    address     = Seq(AddressSet(
                    coreattachparams.coreparams.slave_base_addr, 
                    coreattachparams.coreparams.slave_depth)),
    device      = new SimpleDevice(coreattachparams.coreparams.dev_name + "-slave", 
                    Seq("mitll," + coreattachparams.coreparams.dev_name + "-slave")),
    beatBytes   = coreattachparams.slave_bus.beatBytes
  )

  // Instantiate the implementation
  lazy val module = new aesTLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class aesTLModuleImp(coreparams: COREParams, outer: aesTLModule) extends LazyModuleImp(outer) {

  // "Connect" to llki node's signals and parameters
  val (llki, llkiEdge)    = outer.llki_node.in(0)

  // Define the LLKI Protocol Processing blackbox and its associated IO
  class llki_pp_wrapper(  llki_ctrlsts_addr     : BigInt, 
                          llki_sendrecv_addr    : BigInt,
                          slave_tl_szw          : Int,
                          slave_tl_aiw          : Int,
                          slave_tl_aw           : Int,
                          slave_tl_dbw          : Int,
                          slave_tl_dw           : Int,
                          slave_tl_diw          : Int) extends BlackBox (

      Map(
        "CTRLSTS_ADDR"    -> IntParam(llki_ctrlsts_addr),   // Address of the LLKI PP Control/Status Register
        "SENDRECV_ADDR"   -> IntParam(llki_sendrecv_addr),  // Address of the LLKI PP Message Send/Receive interface
        "SLAVE_TL_SZW"    -> IntParam(slave_tl_szw),
        "SLAVE_TL_AIW"    -> IntParam(slave_tl_aiw),
        "SLAVE_TL_AW"     -> IntParam(slave_tl_aw),
        "SLAVE_TL_DBW"    -> IntParam(slave_tl_dbw),
        "SLAVE_TL_DW"     -> IntParam(slave_tl_dw),
        "SLAVE_TL_DIW"    -> IntParam(slave_tl_diw)
      )
  ) {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())

      // Slave - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_a_opcode      = Input(UInt(3.W))
      val slave_a_param       = Input(UInt(3.W))
      val slave_a_size        = Input(UInt(slave_tl_szw.W))
      val slave_a_source      = Input(UInt(slave_tl_aiw.W))
      val slave_a_address     = Input(UInt(slave_tl_aw.W))
      val slave_a_mask        = Input(UInt(slave_tl_dbw.W))
      val slave_a_data        = Input(UInt(slave_tl_dw.W))
      val slave_a_corrupt     = Input(Bool())
      val slave_a_valid       = Input(Bool())
      val slave_a_ready       = Output(Bool())

      // Slave - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_d_opcode      = Output(UInt(3.W))
      val slave_d_param       = Output(UInt(3.W))
      val slave_d_size        = Output(UInt(slave_tl_szw.W))
      val slave_d_source      = Output(UInt(slave_tl_aiw.W))
      val slave_d_sink        = Output(UInt(slave_tl_diw.W))
      val slave_d_denied      = Output(Bool())
      val slave_d_data        = Output(UInt(slave_tl_dw.W))
      val slave_d_corrupt     = Output(Bool())
      val slave_d_valid       = Output(Bool())
      val slave_d_ready       = Input(Bool())

      // LLKI discrete interface
      val llkid_key_data      = Output(UInt(64.W))
      val llkid_key_valid     = Output(Bool())
      val llkid_key_ready     = Input(Bool())
      val llkid_key_complete  = Input(Bool())
      val llkid_clear_key     = Output(Bool())
      val llkid_clear_key_ack = Input(Bool())

    })
  } // end class llki_pp_wrapper

  // Instantiate the LLKI Protocol Processing Block with CORE SPECIFIC decode constants
  val llki_pp_inst = Module(new llki_pp_wrapper(
    coreparams.llki_ctrlsts_addr, 
    coreparams.llki_sendrecv_addr,
    llkiEdge.bundle.sizeBits,
    llkiEdge.bundle.sourceBits,
    llkiEdge.bundle.addressBits,
    llkiEdge.bundle.dataBits / 8,
    llkiEdge.bundle.dataBits,
    llkiEdge.bundle.sinkBits
  ))

  // Connect the Clock and Reset
  llki_pp_inst.io.clk                 := clock
  llki_pp_inst.io.rst                 := reset

  // Connect the Slave A Channel to the Black box IO
  llki_pp_inst.io.slave_a_opcode      := llki.a.bits.opcode
  llki_pp_inst.io.slave_a_param       := llki.a.bits.param
  llki_pp_inst.io.slave_a_size        := llki.a.bits.size
  llki_pp_inst.io.slave_a_source      := llki.a.bits.source
  llki_pp_inst.io.slave_a_address     := llki.a.bits.address
  llki_pp_inst.io.slave_a_mask        := llki.a.bits.mask
  llki_pp_inst.io.slave_a_data        := llki.a.bits.data
  llki_pp_inst.io.slave_a_corrupt     := llki.a.bits.corrupt
  llki_pp_inst.io.slave_a_valid       := llki.a.valid
  llki.a.ready                        := llki_pp_inst.io.slave_a_ready  

  // Connect the Slave D Channel to the Black Box IO    
  llki.d.bits.opcode                  := llki_pp_inst.io.slave_d_opcode
  llki.d.bits.param                   := llki_pp_inst.io.slave_d_param
  llki.d.bits.size                    := llki_pp_inst.io.slave_d_size
  llki.d.bits.source                  := llki_pp_inst.io.slave_d_source
  llki.d.bits.sink                    := llki_pp_inst.io.slave_d_sink
  llki.d.bits.denied                  := llki_pp_inst.io.slave_d_denied
  llki.d.bits.data                    := llki_pp_inst.io.slave_d_data
  llki.d.bits.corrupt                 := llki_pp_inst.io.slave_d_corrupt
  llki.d.valid                        := llki_pp_inst.io.slave_d_valid
  llki_pp_inst.io.slave_d_ready       := llki.d.ready

  // Define blackbox and its associated IO
  class aes_192_mock_tss() extends BlackBox with HasBlackBoxResource {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())

      // Inputs
      val start               = Input(Bool())
      val state               = Input(UInt(128.W))
      val key                 = Input(UInt(192.W))

      // Outputs
      val out                 = Output(UInt(128.W))
      val out_valid           = Output(Bool())

      // LLKI discrete interface
      val llkid_key_data      = Input(UInt(64.W))
      val llkid_key_valid     = Input(Bool())
      val llkid_key_ready     = Output(Bool())
      val llkid_key_complete  = Output(Bool())
      val llkid_clear_key     = Input(Bool())
      val llkid_clear_key_ack = Output(Bool())

    })

    // Add the SystemVerilog/Verilog files associated with the BlackBox
    // Relative to ./src/main/resources
    addResource("/vsrc/aes/aes_192_mock_tss.sv")
    addResource("/vsrc/aes/aes_192.v")
    addResource("/vsrc/aes/round.v")
    addResource("/vsrc/aes/table.v")

    //Common Resources used by all modules (LLKI, Opentitan, etc.)

      // Provide an optional override of the Blackbox module name
    override def desiredName(): String = {
      return coreparams.verilog_module_name.getOrElse(super.desiredName)
    }
  }

  // Instantiate the blackbox
  val aes_192_inst   = Module(new aes_192_mock_tss())

  // Provide an optional override of the Blackbox module instantiation name
  aes_192_inst.suggestName(aes_192_inst.desiredName()+"_inst")

  // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  aes_192_inst.io.llkid_key_data      := llki_pp_inst.io.llkid_key_data
  aes_192_inst.io.llkid_key_valid     := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready     := aes_192_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete  := aes_192_inst.io.llkid_key_complete
  aes_192_inst.io.llkid_clear_key     := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack := aes_192_inst.io.llkid_clear_key_ack

  // Instantiate registers for the blackbox inputs
  val start               = RegInit(0.U(1.W))
  val state0              = RegInit(0.U(64.W))
  val state1              = RegInit(0.U(64.W))
  val key0                = RegInit(0.U(64.W))
  val key1                = RegInit(0.U(64.W))
  val key2                = RegInit(0.U(64.W))
  val out                 = Wire(UInt(128.W))
  val out_valid           = Wire(Bool())

  // Map the core specific blackbox IO
  aes_192_inst.io.clk    := clock
  aes_192_inst.io.rst    := reset
  aes_192_inst.io.start  := start
  aes_192_inst.io.state  := Cat(state0, state1)
  aes_192_inst.io.key    := Cat(key0, key1, key2)
  out                    := aes_192_inst.io.out
  out_valid              := aes_192_inst.io.out_valid

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
    AESAddresses.aes_ctrlstatus_addr -> RegFieldGroup("aes_ctrlstatus", Some("AES_Control_Status_Register"),Seq(
      RegField    (1, start,      RegFieldDesc("start", "")),
      RegField.r  (1, out_valid,  RegFieldDesc("out_valid", "", volatile=true)))),
    AESAddresses.aes_pt0_addr -> RegFieldGroup("aes_pt0", Some(""), Seq(RegField(64, state0))),
    AESAddresses.aes_pt1_addr -> RegFieldGroup("aes_pt1", Some(""), Seq(RegField(64, state1))),
    AESAddresses.aes_ct0_addr -> RegFieldGroup("aes_ct0", Some(""), Seq(RegField.r(64, out(127,64)))),
    AESAddresses.aes_ct1_addr -> RegFieldGroup("aes_ct1", Some(""), Seq(RegField.r(64, out(63,0)))),
    AESAddresses.aes_key0_addr -> RegFieldGroup("aes_key0", Some(""), Seq(RegField(64, key0))),
    AESAddresses.aes_key1_addr -> RegFieldGroup("aes_key1", Some(""), Seq(RegField(64, key1))),
    AESAddresses.aes_key2_addr -> RegFieldGroup("aes_key2", Some(""), Seq(RegField(64, key2)))
  )  // regmap

}

//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------


