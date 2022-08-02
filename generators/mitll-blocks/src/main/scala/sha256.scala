//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : sha256.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog SHA256 core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.sha256

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
case object PeripherySHA256Key extends Field[Seq[COREParams]](Nil)

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait CanHavePeripherySHA256 { this: BaseSubsystem =>
  val sha256node = p(PeripherySHA256Key).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val sha256module = LazyModule(new sha256TLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      sha256module.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus.beatBytes, coreattachparams.slave_bus.blockBytes) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
      sha256module.llki_node :*= 
      TLSourceShrinker(16) :*=
      TLFragmenter(coreattachparams.llki_bus) :*=_
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { sha256module.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { sha256module.module.clock := coreattachparams.slave_bus.module.clock }

}}

//--------------------------------------------------------------------------------------
// END: Module "Periphery" connections
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class sha256TLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

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
  lazy val module = new sha256TLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class sha256TLModuleImp(coreparams: COREParams, outer: sha256TLModule) extends LazyModuleImp(outer) {

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
  class sha256_mock_tss() extends BlackBox with HasBlackBoxResource {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())

      // Inputs
      val init                = Input(Bool())
      val next                = Input(Bool())
      val block               = Input(UInt(512.W))

      // Outputs
      val digest_valid        = Output(Bool())
      val digest              = Output(UInt(256.W))
      val ready               = Output(Bool())

      // LLKI discrete interface
      val llkid_key_data      = Input(UInt(64.W))
      val llkid_key_valid     = Input(Bool())
      val llkid_key_ready     = Output(Bool())
      val llkid_key_complete  = Output(Bool())
      val llkid_clear_key     = Input(Bool())
      val llkid_clear_key_ack = Output(Bool())

    })

    // Add the SystemVerilog/Verilog associated with the module
    // Relative to /src/main/resources
    addResource("/vsrc/sha256/sha256_mock_tss.sv")
    addResource("/vsrc/sha256/sha256.v")
    addResource("/vsrc/sha256/sha256_k_constants.v")
    addResource("/vsrc/sha256/sha256_w_mem.v")

    // Provide an optional override of the Blackbox module name
    override def desiredName(): String = {
      return coreparams.verilog_module_name.getOrElse(super.desiredName)
    }
  }

  // Instantiate the blackbox
  val sha256_inst   = Module(new sha256_mock_tss())

  // Provide an optional override of the Blackbox module instantiation name
  sha256_inst.suggestName(sha256_inst.desiredName()+"_inst")

  // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  sha256_inst.io.llkid_key_data       := llki_pp_inst.io.llkid_key_data
  sha256_inst.io.llkid_key_valid      := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready     := sha256_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete  := sha256_inst.io.llkid_key_complete
  sha256_inst.io.llkid_clear_key      := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack := sha256_inst.io.llkid_clear_key_ack

  // Macro definition for creating rising edge detectors
  def rising_edge(x: Bool)    = x && !RegNext(x)

  // Define registers and wires associated with the Core I/O
  val next                   = RegInit(false.B)
  val init                   = RegInit(false.B)
  val block0                 = RegInit(0.U(64.W))
  val block1                 = RegInit(0.U(64.W))
  val block2                 = RegInit(0.U(64.W))
  val block3                 = RegInit(0.U(64.W))
  val block4                 = RegInit(0.U(64.W))
  val block5                 = RegInit(0.U(64.W))
  val block6                 = RegInit(0.U(64.W))
  val block7                 = RegInit(0.U(64.W))
  val digest_valid           = Wire(Bool())
  val digest                 = Wire(UInt(256.W))
  val ready                  = Wire(Bool())

  // Connect the Core I/O
  sha256_inst.io.clk             := clock
  sha256_inst.io.rst             := reset
  sha256_inst.io.init            := rising_edge(init)
  sha256_inst.io.next            := rising_edge(next)
  sha256_inst.io.block           := Cat(block0, block1, 
                                        block2, block3, 
                                        block4, block5, 
                                        block6, block7)
  digest                         := sha256_inst.io.digest
  digest_valid                   := sha256_inst.io.digest_valid
  ready                          := sha256_inst.io.ready

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
      SHA256Addresses.sha256_ctrlstatus_addr ->RegFieldGroup("sha256_ready", Some("sha256_ready_Register"),Seq(RegField.r(1,  ready),
                                                                                                               RegField  (1,  init),
                                                                                                               RegField  (1,  next))),
      SHA256Addresses.sha256_block_w0 -> RegFieldGroup("sha256_0", Some("sha256_msg_input_word_0"),        Seq(RegField  (64, block0))),
      SHA256Addresses.sha256_block_w1 -> RegFieldGroup("sha256_1", Some("sha256_msg_input_word_1"),        Seq(RegField  (64, block1))),
      SHA256Addresses.sha256_block_w2 -> RegFieldGroup("sha256_2", Some("sha256_msg_input_word_2"),        Seq(RegField  (64, block2))),
      SHA256Addresses.sha256_block_w3 -> RegFieldGroup("sha256_3", Some("sha256_msg_input_word_3"),        Seq(RegField  (64, block3))),
      SHA256Addresses.sha256_block_w4 -> RegFieldGroup("sha256_4", Some("sha256_msg_input_word_4"),        Seq(RegField  (64, block4))),
      SHA256Addresses.sha256_block_w5 -> RegFieldGroup("sha256_5", Some("sha256_msg_input_word_5"),        Seq(RegField  (64, block5))),
      SHA256Addresses.sha256_block_w6 -> RegFieldGroup("sha256_6", Some("sha256_msg_input_word_6"),        Seq(RegField  (64, block6))),
      SHA256Addresses.sha256_block_w7 -> RegFieldGroup("sha256_7", Some("sha256_msg_input_word_7"),        Seq(RegField  (64, block7))),
      SHA256Addresses.sha256_done     -> RegFieldGroup("sha256_done", Some("sha256_done"),                 Seq(RegField.r(1,  digest_valid))),
      SHA256Addresses.sha256_digest_w0 -> RegFieldGroup("sha256_msg_output0", Some("sha256_msg_output0"),  Seq(RegField.r(64, digest(255,192)))),
      SHA256Addresses.sha256_digest_w1 -> RegFieldGroup("sha256_msg_output1", Some("sha256_msg_output1"),  Seq(RegField.r(64, digest(191,128)))),
      SHA256Addresses.sha256_digest_w2 -> RegFieldGroup("sha256_msg_output2", Some("sha256_msg_output2"),  Seq(RegField.r(64, digest(127, 64)))),
      SHA256Addresses.sha256_digest_w3 -> RegFieldGroup("sha256_msg_output3", Some("sha256_msg_output3"),  Seq(RegField.r(64, digest( 63,  0))))               
  )  // regmap

}
//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------
