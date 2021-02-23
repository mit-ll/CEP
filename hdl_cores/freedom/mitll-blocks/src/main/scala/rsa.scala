//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : rsa.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog RSA core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.rsa

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
case object PeripheryRSAKey extends Field[Seq[COREParams]]

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasPeripheryRSA { this: BaseSubsystem =>
  val rsanode = p(PeripheryRSAKey).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val rsamodule = LazyModule(new rsaTLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      rsamodule.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus.beatBytes, coreattachparams.slave_bus.blockBytes) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
      rsamodule.llki_node :*= 
      TLSourceShrinker(16) :*= _
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { rsamodule.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { rsamodule.module.clock := coreattachparams.slave_bus.module.clock }

}}
//--------------------------------------------------------------------------------------
// END: Module "Periphery" connections
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class rsaTLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

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
      supportsGet         = TransferSizes(1, coreattachparams.llki_bus.blockBytes),
      supportsPutFull     = TransferSizes(1, coreattachparams.llki_bus.blockBytes),
      supportsPutPartial  = TransferSizes(1, coreattachparams.llki_bus.blockBytes),
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
  lazy val module = new rsaTLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class rsaTLModuleImp(coreparams: COREParams, outer: rsaTLModule) extends LazyModuleImp(outer) {

  // "Connect" to llki node's signals and parameters
  val (llki, llkiEdge)    = outer.llki_node.in(0)

  // Define the LLKI Protocol Processing blackbox and its associated IO
  class llki_pp_wrapper(val llki_ctrlsts_addr: BigInt, llki_sendrecv_addr: BigInt) extends BlackBox(
      Map(
        "CTRLSTS_ADDR"    -> IntParam(llki_ctrlsts_addr),  // Base address of the TL slave
        "SENDRECV_ADDR"   -> IntParam(llki_sendrecv_addr)  // Address depth of the TL slave
      )
  ) {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())

      // Slave - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_a_opcode      = Input(UInt(3.W))
      val slave_a_param       = Input(UInt(3.W))
      val slave_a_size        = Input(UInt(LLKITilelinkParameters.SizeBits.W))
      val slave_a_source      = Input(UInt(LLKITilelinkParameters.SourceBits.W))
      val slave_a_address     = Input(UInt(LLKITilelinkParameters.AddressBits.W))
      val slave_a_mask        = Input(UInt(LLKITilelinkParameters.BeatBytes.W))
      val slave_a_data        = Input(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
      val slave_a_corrupt     = Input(Bool())
      val slave_a_valid       = Input(Bool())
      val slave_a_ready       = Output(Bool())

      // Slave - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_d_opcode      = Output(UInt(3.W))
      val slave_d_param       = Output(UInt(3.W))
      val slave_d_size        = Output(UInt(LLKITilelinkParameters.SizeBits.W))
      val slave_d_source      = Output(UInt(LLKITilelinkParameters.SourceBits.W))
      val slave_d_sink        = Output(UInt(LLKITilelinkParameters.SinkBits.W))
      val slave_d_denied      = Output(Bool())
      val slave_d_data        = Output(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
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
  val llki_pp_inst = Module(new llki_pp_wrapper(coreparams.llki_ctrlsts_addr, 
                                                coreparams.llki_sendrecv_addr))

  // The following "requires" are included to avoid size mismatches between the
  // the Rocket Chip buses and the SRoT Black Box.  The expected values are inhereited
  // from the cep_addresses package and must match those in "top_pkg.sv", borrowed from OpenTitan
  //
  // Exceptions:
  //  - llkiEdge address gets optimized down to 31-bits during chisel generation
  //  - llkiEdge sink bits are 1, but masterEdge sink bits are 2 
  //  - llkiEdge size bits are 3, but masterEdge size bits are 4
  //
  require(llkiEdge.bundle.addressBits  == LLKITilelinkParameters.AddressBits - 1, s"SROT: llkiEdge addressBits exp/act ${LLKITilelinkParameters.AddressBits - 1}/${llkiEdge.bundle.addressBits}")
  require(llkiEdge.bundle.dataBits     == LLKITilelinkParameters.BeatBytes * 8, s"SROT: llkiEdge dataBits exp/act ${LLKITilelinkParameters.BeatBytes * 8}/${llkiEdge.bundle.dataBits}")
  require(llkiEdge.bundle.sourceBits   == LLKITilelinkParameters.SourceBits, s"SROT: llkiEdge sourceBits exp/act ${LLKITilelinkParameters.SourceBits}/${llkiEdge.bundle.sourceBits}")
  require(llkiEdge.bundle.sinkBits     == LLKITilelinkParameters.SinkBits - 1, s"SROT: llkiEdge sinkBits exp/act ${LLKITilelinkParameters.SinkBits - 1}/${llkiEdge.bundle.sinkBits}")
  require(llkiEdge.bundle.sizeBits     == LLKITilelinkParameters.SizeBits, s"SROT: llkiEdge sizeBits exp/act ${LLKITilelinkParameters.SizeBits}/${llkiEdge.bundle.sizeBits}")

  // Connect the Clock and Reset
  llki_pp_inst.io.clk                 := clock
  llki_pp_inst.io.rst                 := reset

  // Connect the Slave A Channel to the Black box IO
  llki_pp_inst.io.slave_a_opcode      := llki.a.bits.opcode    
  llki_pp_inst.io.slave_a_param       := llki.a.bits.param     
  llki_pp_inst.io.slave_a_size        := llki.a.bits.size
  llki_pp_inst.io.slave_a_source      := llki.a.bits.source    
  llki_pp_inst.io.slave_a_address     := Cat(0.U(1.W), llki.a.bits.address)
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
  llki.d.bits.sink                    := llki_pp_inst.io.slave_d_sink(0)
  llki.d.bits.denied                  := llki_pp_inst.io.slave_d_denied
  llki.d.bits.data                    := llki_pp_inst.io.slave_d_data
  llki.d.bits.corrupt                 := llki_pp_inst.io.slave_d_corrupt
  llki.d.valid                        := llki_pp_inst.io.slave_d_valid
  llki_pp_inst.io.slave_d_ready       := llki.d.ready

  // Define blackbox and its associated IO
  class modexp_core_mock_tss() extends BlackBox {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                           = Input(Clock())
      val rst                           = Input(Reset())

      // Core I/O
      val start                         = Input(Bool())
      val exponent_length               = Input(UInt(13.W))
      val modulus_length                = Input(UInt(8.W))
      val ready                         = Output(Bool())
      val cycles                        = Output(UInt(64.W))
      val exponent_mem_api_cs           = Input(Bool())
      val exponent_mem_api_wr           = Input(Bool())
      val exponent_mem_api_rst          = Input(Bool())
      val exponent_mem_api_write_data   = Input(UInt(32.W))
      val exponent_mem_api_read_data    = Output(UInt(32.W))
      val modulus_mem_api_cs            = Input(Bool())
      val modulus_mem_api_wr            = Input(Bool())
      val modulus_mem_api_rst           = Input(Bool())
      val modulus_mem_api_write_data    = Input(UInt(32.W))
      val modulus_mem_api_read_data     = Output(UInt(32.W))
      val message_mem_api_cs            = Input(Bool())
      val message_mem_api_wr            = Input(Bool())
      val message_mem_api_rst           = Input(Bool())
      val message_mem_api_write_data    = Input(UInt(32.W))
      val message_mem_api_read_data     = Output(UInt(32.W))
      val result_mem_api_cs             = Input(Bool())
      val result_mem_api_rst            = Input(Bool())
      val result_mem_api_read_data      = Output(UInt(32.W))

      // LLKI discrete interface
      val llkid_key_data                = Input(UInt(64.W))
      val llkid_key_valid               = Input(Bool())
      val llkid_key_ready               = Output(Bool())
      val llkid_key_complete            = Output(Bool())
      val llkid_clear_key               = Input(Bool())
      val llkid_clear_key_ack           = Output(Bool())

    })
  }

  // Instantiate the blackbox
  val modexp_core_mock_tss_inst   = Module(new modexp_core_mock_tss())

  // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  modexp_core_mock_tss_inst.io.llkid_key_data   := llki_pp_inst.io.llkid_key_data
  modexp_core_mock_tss_inst.io.llkid_key_valid  := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready               := modexp_core_mock_tss_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete            := modexp_core_mock_tss_inst.io.llkid_key_complete
  modexp_core_mock_tss_inst.io.llkid_clear_key  := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack           := modexp_core_mock_tss_inst.io.llkid_clear_key_ack

  // Define registers and wires associated with the Core I/O
  val start                                 = RegInit(false.B)
  val exponent_length                       = RegInit(0.U(8.W))
  val modulus_length                        = RegInit(0.U(8.W))
  val ready                                 = RegInit(false.B)
  val cycles                                = RegInit(0.U(64.W))

  val exponent_mem_api_cs                   = RegInit(false.B)
  val exponent_mem_api_wr                   = RegInit(false.B)
  val exponent_mem_api_rst                  = RegInit(false.B)
  val exponent_mem_api_write_data           = RegInit(0.U(32.W))
  val exponent_mem_api_read_data            = RegInit(0.U(32.W))

  val modulus_mem_api_cs                    = RegInit(false.B)
  val modulus_mem_api_wr                    = RegInit(false.B)
  val modulus_mem_api_rst                   = RegInit(false.B)
  val modulus_mem_api_write_data            = RegInit(0.U(32.W))
  val modulus_mem_api_read_data             = RegInit(0.U(32.W))

  val message_mem_api_cs                    = RegInit(false.B)
  val message_mem_api_wr                    = RegInit(false.B)
  val message_mem_api_rst                   = RegInit(false.B)
  val message_mem_api_write_data            = RegInit(0.U(32.W))
  val message_mem_api_read_data             = RegInit(0.U(32.W))

  val result_mem_api_cs                     = RegInit(false.B)
  val result_mem_api_rst                    = RegInit(false.B)
  val result_mem_api_read_data              = RegInit(0.U(32.W))

  // Macro definition for creating rising edge detectors
  def rising_edge(x: Bool)    = x && !RegNext(x)

  // Connect the Clock and Reset
  modexp_core_mock_tss_inst.io.clk                          := clock
  modexp_core_mock_tss_inst.io.rst                          := reset

  // Connect the Core I/O
  modexp_core_mock_tss_inst.io.start                        := start;
  modexp_core_mock_tss_inst.io.exponent_length              := exponent_length;
  modexp_core_mock_tss_inst.io.modulus_length               := modulus_length;
  ready                                                     := modexp_core_mock_tss_inst.io.ready
  cycles                                                    := modexp_core_mock_tss_inst.io.cycles

  modexp_core_mock_tss_inst.io.exponent_mem_api_cs          := rising_edge(exponent_mem_api_cs)
  modexp_core_mock_tss_inst.io.exponent_mem_api_wr          := rising_edge(exponent_mem_api_wr)
  modexp_core_mock_tss_inst.io.exponent_mem_api_rst         := rising_edge(exponent_mem_api_rst)
  modexp_core_mock_tss_inst.io.exponent_mem_api_write_data  := exponent_mem_api_write_data
  exponent_mem_api_read_data                                := modexp_core_mock_tss_inst.io.exponent_mem_api_read_data

  modexp_core_mock_tss_inst.io.modulus_mem_api_cs           := rising_edge(modulus_mem_api_cs)
  modexp_core_mock_tss_inst.io.modulus_mem_api_wr           := rising_edge(modulus_mem_api_wr)
  modexp_core_mock_tss_inst.io.modulus_mem_api_rst          := rising_edge(modulus_mem_api_rst)
  modexp_core_mock_tss_inst.io.modulus_mem_api_write_data   := modulus_mem_api_write_data
  modulus_mem_api_read_data                                 := modexp_core_mock_tss_inst.io.modulus_mem_api_read_data

  modexp_core_mock_tss_inst.io.message_mem_api_cs           := rising_edge(message_mem_api_cs)
  modexp_core_mock_tss_inst.io.message_mem_api_wr           := rising_edge(message_mem_api_wr)
  modexp_core_mock_tss_inst.io.message_mem_api_rst          := rising_edge(message_mem_api_rst)
  modexp_core_mock_tss_inst.io.message_mem_api_write_data   := message_mem_api_write_data
  message_mem_api_read_data                                 := modexp_core_mock_tss_inst.io.message_mem_api_read_data

  modexp_core_mock_tss_inst.io.result_mem_api_cs            := rising_edge(result_mem_api_cs)
  modexp_core_mock_tss_inst.io.result_mem_api_rst           := rising_edge(result_mem_api_rst)

  when (rising_edge(result_mem_api_cs)) {
    result_mem_api_read_data                                := modexp_core_mock_tss_inst.io.result_mem_api_read_data
  }

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
    RSAAddresses.rsa_ctrlstatus_addr  ->    RegFieldGroup("rsa_ready", Some("rsa_ready Register"),Seq(RegField.r(1,  ready),
                                                                                                      RegField  (1,  start))),
    RSAAddresses.rsa_exp_data_addr    ->    Seq(RegField   (32, exponent_mem_api_write_data),
                                                RegField.r (32, exponent_mem_api_read_data)), // [63;32]
    RSAAddresses.rsa_exp_ctrl_addr    ->    Seq(RegField   (1 , exponent_mem_api_cs),
                                                RegField   (1 , exponent_mem_api_wr),
                                                RegField   (1 , exponent_mem_api_rst)),
    RSAAddresses.rsa_mod_data         ->    Seq(RegField   (32, modulus_mem_api_write_data),
                                                RegField.r (32, modulus_mem_api_read_data)), // [63:32]
    RSAAddresses.rsa_mod_ctrl_addr    ->    Seq(RegField   (1 , modulus_mem_api_cs),
                                                RegField   (1 , modulus_mem_api_wr),
                                                RegField   (1 , modulus_mem_api_rst)),
    RSAAddresses.rsa_message_data     ->    Seq(RegField   (32, message_mem_api_write_data),
                                                RegField.r (32, message_mem_api_read_data)), // [63:32]    
    RSAAddresses.rsa_message_ctrl_addr->    Seq(RegField   (1 , message_mem_api_cs),
                                                RegField   (1 , message_mem_api_wr),
                                                RegField   (1 , message_mem_api_rst)),
    RSAAddresses.rsa_mod_length       ->    Seq(RegField   (8 , modulus_length)),
    RSAAddresses.rsa_exp_length       ->    Seq(RegField   (13, exponent_length)),
    RSAAddresses.rsa_result_data_addr ->    Seq(RegField.r (32, result_mem_api_read_data)),
    RSAAddresses.rsa_result_ctrl_addr ->    Seq(RegField   (1 , result_mem_api_cs),
                                                RegField   (1 , result_mem_api_rst)),  
    RSAAddresses.rsa_cycles_addr      ->    Seq(RegField.r (64, cycles))
  )  // regmap

}
//--------------------------------------------------------------------------------------
// END: AES TileLink Module
//--------------------------------------------------------------------------------------

