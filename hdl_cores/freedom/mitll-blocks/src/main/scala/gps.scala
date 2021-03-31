//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : gps.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog GPS core
//
//--------------------------------------------------------------------------------------
package mitllBlocks.gps

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
case object PeripheryGPSKey extends Field[Seq[COREParams]]

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasPeripheryGPS { this: BaseSubsystem =>
  val gpsnode = p(PeripheryGPSKey).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val gpsmodule = LazyModule(new gpsTLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      gpsmodule.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus.beatBytes, coreattachparams.slave_bus.blockBytes) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
     gpsmodule.llki_node :*= 
      TLSourceShrinker(16) :*= _
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { gpsmodule.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { gpsmodule.module.clock := coreattachparams.slave_bus.module.clock }

}}

//--------------------------------------------------------------------------------------
// BEGIN: Module "Periphery" connections
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class gpsTLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

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
  lazy val module = new gpsTLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class gpsTLModuleImp(coreparams: COREParams, outer: gpsTLModule) extends LazyModuleImp(outer) {

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
      val rst                 = Input(Bool())

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
  class gps_mock_tss () extends BlackBox {

    val io = IO(new Bundle {
      // Clock and Reset
      val sys_clk_50          = Input(Clock())
      val sync_rst_in         = Input(Reset())
      val sync_rst_in_dut     = Input(Reset())

      // Inputs
      val startRound          = Input(Bool())
      val sv_num              = Input(UInt(6.W))
      val aes_key             = Input(UInt(192.W))
      val pcode_speeds        = Input(UInt(31.W))
      val pcode_initializers  = Input(UInt(48.W))
      
      // Outputs
      val ca_code             = Output(UInt(13.W))
      val p_code              = Output(UInt(128.W))
      val l_code              = Output(UInt(128.W))
      val l_code_valid        = Output(Bool())

      // LLKI discrete interface
      val llkid_key_data      = Input(UInt(64.W))
      val llkid_key_valid     = Input(Bool())
      val llkid_key_ready     = Output(Bool())
      val llkid_key_complete  = Output(Bool())
      val llkid_clear_key     = Input(Bool())
      val llkid_clear_key_ack = Output(Bool())

    })
  }
 
  // Instantiate the blackbox
  val gps_mock_tss_inst   = Module(new gps_mock_tss())

    // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  gps_mock_tss_inst.io.llkid_key_data       := llki_pp_inst.io.llkid_key_data
  gps_mock_tss_inst.io.llkid_key_valid      := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready           := gps_mock_tss_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete        := gps_mock_tss_inst.io.llkid_key_complete
  gps_mock_tss_inst.io.llkid_clear_key      := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack       := gps_mock_tss_inst.io.llkid_clear_key_ack

  // Instantiate registers for the blackbox inputs
  val startRound                   = RegInit(0.U(1.W))
  val sv_num                       = RegInit(0.U(6.W))
  val aes_key0                     = RegInit(0.U(64.W))
  val aes_key1                     = RegInit(0.U(64.W))
  val aes_key2                     = RegInit(0.U(64.W))
  val pcode_xn_cnt_speed           = RegInit(0x001.U(12.W))
  val pcode_z_cnt_speed            = RegInit(0x001.U(19.W))
  val pcode_ini_x1a                = RegInit(0x248.U(12.W)) // 12'b001001001000
  val pcode_ini_x1b                = RegInit(0x554.U(12.W)) // 12'b010101010100
  val pcode_ini_x2a                = RegInit(0x925.U(12.W)) // 12'b100100100101
  val pcode_ini_x2b                = RegInit(0x554.U(12.W)) // 12'b010101010100
  
//
// tony duong: 02/24/21: change default to true to support unit sim
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
  val l_code_valid                 = Wire(Bool())

  // Map the blackbox I/O 
  gps_mock_tss_inst.io.sys_clk_50         := clock                                      // Implicit module clock
  gps_mock_tss_inst.io.sync_rst_in        := reset.asBool
  gps_mock_tss_inst.io.sync_rst_in_dut    := (reset.asBool || gps_reset).asAsyncReset 
                                                                                        // Implicit module reset
  gps_mock_tss_inst.io.startRound         := startRound                                 // Start bit
  gps_mock_tss_inst.io.sv_num             := sv_num                                     // GPS space vehicle number written by cepregression.cpp
  gps_mock_tss_inst.io.aes_key            := Cat(aes_key0, aes_key1, aes_key2)          // L code encryption key
  gps_mock_tss_inst.io.pcode_speeds       := Cat(pcode_z_cnt_speed, pcode_xn_cnt_speed) // PCode acceleration register
  gps_mock_tss_inst.io.pcode_initializers := Cat(pcode_ini_x2b, pcode_ini_x2a, pcode_ini_x1b, pcode_ini_x1a) // Initializers for pcode shift registers
  ca_code                                 := gps_mock_tss_inst.io.ca_code               // Output GPS CA code
  p_code0_u                               := gps_mock_tss_inst.io.p_code(127,96)        // Output P Code bits 
  p_code0_l                               := gps_mock_tss_inst.io.p_code(95,64)         // Output P Code bits      
  p_code1_u                               := gps_mock_tss_inst.io.p_code(63,32)         // Output P Code bits          
  p_code1_l                               := gps_mock_tss_inst.io.p_code(31,0)          // Output P Code bits
  l_code0_u                               := gps_mock_tss_inst.io.l_code(127,96)        // Output L Code bits                        
  l_code0_l                               := gps_mock_tss_inst.io.l_code(95,64)         // Output L Code bits
  l_code1_u                               := gps_mock_tss_inst.io.l_code(63,32)         // Output L Code bits      
  l_code1_l                               := gps_mock_tss_inst.io.l_code(31,0)          // Output L Code bits
  l_code_valid                            := gps_mock_tss_inst.io.l_code_valid          // Out is valid until start is again asserted

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  // Likewise, .w means Write Only
  outer.slave_node.regmap (
    GPSAddresses.gps_ctrlstatus_addr -> RegFieldGroup("gps_ctrlstatus", Some("GPS Control/Status Register"),Seq(
                    RegField    (1, startRound,      RegFieldDesc("start", "")),
                    RegField.r  (1, l_code_valid,  RegFieldDesc ("l_code_valid", "", volatile=true)))),
    GPSAddresses.gps_sv_num_addr     -> RegFieldGroup("sv_num",     Some("GPS Set SV sv_num"),          Seq(RegField  (6,  sv_num))),
    GPSAddresses.gps_aes_key_addr_w0 -> RegFieldGroup("aes_key",    Some("GPS Set AES Key upper bits"), Seq(RegField.w(64, aes_key0))),
    GPSAddresses.gps_aes_key_addr_w1 -> RegFieldGroup("aes_key",    Some("GPS Set AES Key middle bits"),Seq(RegField.w(64, aes_key1))),
    GPSAddresses.gps_aes_key_addr_w2 -> RegFieldGroup("aes_key",    Some("GPS Set AES Key low bits"),   Seq(RegField.w(64, aes_key2))),
    GPSAddresses.gps_pcode_speed_addr-> RegFieldGroup("pcode_speed",Some("GPS PCode acceleration"),     Seq(
                    RegField.w(12, pcode_xn_cnt_speed, RegFieldDesc("PCode Xn Counter Speed","")),
                    RegField.w(19, pcode_z_cnt_speed,  RegFieldDesc("PCode Z Counter Speed","")) )),
    GPSAddresses.gps_pcode_xini_addr -> RegFieldGroup("pcode_ini",  Some("GPS PCode x-lfsr inittial states"),Seq(
                    RegField.w(12, pcode_ini_x1a, RegFieldDesc("PCode X1A Initial State","")),
                    RegField.w(12, pcode_ini_x1b, RegFieldDesc("PCode X1B Initial State","")),
                    RegField.w(12, pcode_ini_x2a, RegFieldDesc("PCode X2A Initial State","")),
                    RegField.w(12, pcode_ini_x2b, RegFieldDesc("PCode X2B Initial State","")) )),
    GPSAddresses.gps_ca_code_addr    -> RegFieldGroup("gps_cacode", Some("GPS CA code"),                Seq(RegField.r(64, ca_code))),
    GPSAddresses.gps_reset_addr      -> RegFieldGroup("gps_reset",  Some("GPS addressable reset"),      Seq(RegField  (1,  gps_reset))),            
    GPSAddresses.gps_p_code_addr_w0  -> RegFieldGroup("gps_pcode1", Some("GPS pcode upper bits"),       Seq(RegField.r(64, Cat(p_code0_u,p_code0_l)))),
    GPSAddresses.gps_p_code_addr_w1  -> RegFieldGroup("gps_pcode1", Some("GPS pcode lower 64 bits"),    Seq(RegField.r(64, Cat(p_code1_u,p_code1_l)))),
    GPSAddresses.gps_l_code_addr_w0  -> RegFieldGroup("gps_lcode1", Some("GPS lcode upper 64 bits"),    Seq(RegField.r(64, Cat(l_code0_u,l_code0_l)))),
    GPSAddresses.gps_l_code_addr_w1  -> RegFieldGroup("gps_lcode1", Some("GPS lcode lower 64 bits"),    Seq(RegField.r(64, Cat(l_code1_u,l_code1_l))))
  )  // regmap
}
//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------

