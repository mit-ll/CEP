//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : iir.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog IIR core
// Note         : The "control" logic described in the IIR abstract class
//                is intended to mimic the verilog in the iir_top_wb.v
//                module.
//
//--------------------------------------------------------------------------------------

package mitllBlocks.iir

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
case object PeripheryIIRKey extends Field[Seq[COREParams]]

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasPeripheryIIR { this: BaseSubsystem =>
  val iirnode = p(PeripheryIIRKey).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val iirmodule = LazyModule(new iirTLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      iirmodule.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus.beatBytes, coreattachparams.slave_bus.blockBytes) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
      iirmodule.llki_node :*= 
      TLSourceShrinker(16) :*= _
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { iirmodule.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { iirmodule.module.clock := coreattachparams.slave_bus.module.clock }

}}

//--------------------------------------------------------------------------------------
// BEGIN: Module "Periphery" connections
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class iirTLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

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
  lazy val module = new iirTLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class iirTLModuleImp(coreparams: COREParams, outer: iirTLModule) extends LazyModuleImp(outer) {

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
  class IIR_filter_mock_tss () extends BlackBox {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())
      val rst_dut             = Input(Reset())
   
      // Inputs
      val inData              = Input(UInt(32.W))
    
      // Outputs
      val outData             = Output(UInt(32.W))

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
  val IIR_filter_mock_tss_inst   = Module(new IIR_filter_mock_tss())

  // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  IIR_filter_mock_tss_inst.io.llkid_key_data    := llki_pp_inst.io.llkid_key_data
  IIR_filter_mock_tss_inst.io.llkid_key_valid   := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready               := IIR_filter_mock_tss_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete            := IIR_filter_mock_tss_inst.io.llkid_key_complete
  IIR_filter_mock_tss_inst.io.llkid_clear_key   := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack           := IIR_filter_mock_tss_inst.io.llkid_clear_key_ack

  // Macro definition for creating rising edge detectors
  def rising_edge(x: Bool)    = x && !RegNext(x)

  // Instantiate the input and output data memories (32 words of input and output data)
  val datain_mem              = Mem(32, UInt(32.W))     // for holding the input data
  val dataout_mem             = Mem(32, UInt(32.W))     // for holding the output data

  // Define registers / wires for interfacing to the IIR blackbox
  val start                   = RegInit(false.B)      // Start bit
  val iir_reset               = RegInit(false.B)      // Addressable reset
  val iir_reset_re            = RegInit(false.B)      // Rising edge detection for addressable reset   
  val datain_we               = RegInit(false.B)      // Controlled via register mappings
  val datain_write_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
  val datain_write_data       = RegInit(0.U(32.W))    // Controlled via register mappings
  val datain_read_idx         = RegInit(0.U(6.W))     // Generated write read address from start bit
  val datain_read_data        = Wire(UInt(32.W))      // Data read from intermediate buffer into IIR_filter.v    

  val dataout_write_idx       = RegInit(0.U(6.W))     // Data output address generated from next_out
  val dataout_write_data      = RegInit(0.U(32.W))    // Data output
  val dataout_read_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
  val dataout_read_data       = Wire(UInt(32.W))      // Controlled via register mappings
  val dataout_valid           = RegInit(false.B)      // Data valid output bit

  val count                   = RegInit(0.U(6.W))     // Count syncs output buffers with filter propagation
  val next_out                = Wire(Bool())          // Bit driven high to designate start of the output sequence

  iir_reset_re                := rising_edge(iir_reset)

  // Write to the input data memory when a rising edge is detected on the write enable
  when (rising_edge(datain_we)) {
    datain_mem.write(datain_write_idx, datain_write_data)
  }

  // Implement the read logic for the datain and data out memories
  datain_read_data            := datain_mem(datain_read_idx)
  dataout_read_data           := dataout_mem(dataout_read_idx)

  // Generate the read index for the data in memory
  when (rising_edge(start)) {
    datain_read_idx         := 0.U
  } .elsewhen (datain_read_idx < 32.U) {
    datain_read_idx         := datain_read_idx + 1.U
  }

  // The following counter "counts" the propagation through the IIR filter
  when (rising_edge(start)) {
    count                   := 0.U
  } .elsewhen (datain_read_idx < 10.U) {
    count                   := count + 1.U
  }

  // Assert next out when the count reaches the appropriate value
  next_out                    := (count === 3.U)

  // Generate the write index for the output data memory (and write)
  when (rising_edge(next_out)) {
    dataout_write_idx       := 0.U;
  } .elsewhen (dataout_write_idx < 32.U) {
    dataout_write_idx       := dataout_write_idx + 1.U
    dataout_mem.write(dataout_write_idx, dataout_write_data)
  }

  // Generate the data valid signal
  when (rising_edge(start)) {
    dataout_valid           := 0.U
  } .elsewhen (rising_edge(next_out)) {
    dataout_valid           := 1.U
  }

  // Map the blackbox I/O
  // The IIR needs to be reset in between test vectors, thus a second reset
  // has been added in order to allow for the LLKI keys to persist
  IIR_filter_mock_tss_inst.io.clk       := clock                            
  IIR_filter_mock_tss_inst.io.rst       := reset.asBool
  IIR_filter_mock_tss_inst.io.rst_dut   := (reset.asBool || iir_reset_re).asAsyncReset                                                                      
  IIR_filter_mock_tss_inst.io.inData    := Mux(datain_read_idx < 32.U, datain_read_data, 0.U)
  dataout_write_data                    := IIR_filter_mock_tss_inst.io.outData

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
    IIRAddresses.iir_ctrlstatus_addr    -> RegFieldGroup("iir_ctrlstatus",Some(""), Seq(
      RegField    (1, start               ),      // Start passing data to the IIR blackbox
      RegField    (1, datain_we           ),      // Write enable for the datain memory
      RegField.r  (1, dataout_valid       ))),    // Data Out Valid
    IIRAddresses.iir_reset_addr                 -> Seq(RegField     (1,  iir_reset)),
    IIRAddresses.iir_datain_addr_addr   -> Seq(RegField     (5,  datain_write_idx)),
    IIRAddresses.iir_datain_data_addr   -> Seq(RegField     (32, datain_write_data)),
    IIRAddresses.iir_dataout_addr_addr  -> Seq(RegField     (5,  dataout_read_idx)),
    IIRAddresses.iir_dataout_data_addr  -> Seq(RegField.r   (32, dataout_read_data))
  )  // regmap

}
//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------

