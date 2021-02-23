//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : idft.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog idft core
// Note         : The "control" logic described in the idft abstract class
//                is intended to mimic the verilog in the dft_top_wb.v
//                module.
//
//--------------------------------------------------------------------------------------

package mitllBlocks.idft

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
case object PeripheryIDFTKey extends Field[Seq[COREParams]]

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasPeripheryIDFT { this: BaseSubsystem =>
  val idftnode = p(PeripheryIDFTKey).map { params =>

    // Initialize the attachment parameters
    val coreattachparams = COREAttachParams(
      coreparams  = params,
      llki_bus    = pbus, // The LLKI connects to the periphery bus
      slave_bus   = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val idftmodule = LazyModule(new idftTLModule(coreattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    coreattachparams.slave_bus.coupleTo(coreattachparams.coreparams.dev_name + "_slave") {
      idftmodule.slave_node :*=
      TLFragmenter(coreattachparams.slave_bus.beatBytes, coreattachparams.slave_bus.blockBytes) :*= _
    }

    // Perform the slave "attachments" to the llki bus
    coreattachparams.llki_bus.coupleTo(coreattachparams.coreparams.dev_name + "_llki_slave") {
      idftmodule.llki_node :*= 
      TLSourceShrinker(16) :*= _
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { idftmodule.module.reset := coreattachparams.slave_bus.module.reset }
    InModuleBody { idftmodule.module.clock := coreattachparams.slave_bus.module.clock }

}}

//--------------------------------------------------------------------------------------
// BEGIN: Module "Periphery" connections
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class idftTLModule(coreattachparams: COREAttachParams)(implicit p: Parameters) extends LazyModule {

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
  lazy val module = new idftTLModuleImp(coreattachparams.coreparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class idftTLModuleImp(coreparams: COREParams, outer: idftTLModule) extends LazyModuleImp(outer) {

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
  class idft_top_mock_tss () extends BlackBox {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk                 = Input(Clock())
      val rst                 = Input(Reset())
   
      // Inputs
      val next                = Input(Bool())            
      val X0                  = Input(UInt(16.W))
      val X1                  = Input(UInt(16.W))
      val X2                  = Input(UInt(16.W))
      val X3                  = Input(UInt(16.W))
    
      // Outputs
      val next_out            = Output(Bool())
      val Y0                  = Output(UInt(16.W))
      val Y1                  = Output(UInt(16.W))
      val Y2                  = Output(UInt(16.W))
      val Y3                  = Output(UInt(16.W))

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
  val idft_top_mock_tss_inst   = Module(new idft_top_mock_tss())

  // Map the LLKI discrete blackbox IO between the core_inst and llki_pp_inst
  idft_top_mock_tss_inst.io.llkid_key_data      := llki_pp_inst.io.llkid_key_data
  idft_top_mock_tss_inst.io.llkid_key_valid     := llki_pp_inst.io.llkid_key_valid
  llki_pp_inst.io.llkid_key_ready               := idft_top_mock_tss_inst.io.llkid_key_ready
  llki_pp_inst.io.llkid_key_complete            := idft_top_mock_tss_inst.io.llkid_key_complete
  idft_top_mock_tss_inst.io.llkid_clear_key     := llki_pp_inst.io.llkid_clear_key
  llki_pp_inst.io.llkid_clear_key_ack           := idft_top_mock_tss_inst.io.llkid_clear_key_ack

  // Macro definition for creating rising edge detectors
  def rising_edge(x: Bool)    = x && !RegNext(x)

  // Instantiate the input and output data memories (32 words of input and output data)
  val datain_mem              = Mem(32, UInt(64.W))     // for holding the input data
  val dataout_mem             = Mem(32, UInt(64.W))     // for holding the output data

  // Define registers / wires for interfacing to the dft blackbox
  val start                   = RegInit(false.B)      // Start bit controlled via register mappings
  val datain_we               = RegInit(false.B)      // Controlled via register mappings
  val datain_write_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
  val datain_write_data       = RegInit(0.U(64.W))    // Controlled via register mappings
  val datain_read_idx         = RegInit(0.U(6.W))     // Generated read address from start bit
  val datain_read_data        = Wire(UInt(64.W))      // Data read from intermediate buffer into dft_top.v 

  val dataout_write_idx       = RegInit(0.U(6.W))     // Data output address generated from next_out
  val dataout_write_data      = RegInit(0.U(64.W))    // Data output
  val dataout_read_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
  val dataout_read_data       = Wire(UInt(64.W))      // Controlled via register mappings
  val dataout_valid           = RegInit(false.B)      // Data valid output bit drive by dft_top.v

  val next_out                = RegInit(false.B)

  // Write to the input data memory when a rising edge is detected on the write enable
  when (rising_edge(datain_we)) {
    datain_mem.write(datain_write_idx, datain_write_data)
  }

  // Implement the read logic for the datain and data out memories
  datain_read_data              := datain_mem(datain_read_idx)
  dataout_read_data             := dataout_mem(dataout_read_idx)

  // Generate the read index for the data in memory
  when (rising_edge(start)) {
    datain_read_idx             := 0.U
  } .elsewhen (datain_read_idx < 32.U) {
    datain_read_idx             := datain_read_idx + 1.U
  }

  // Generate the write index for the output data memory (and write)
  when (next_out){
    dataout_write_idx           := 0.U;
  } .elsewhen (dataout_write_idx < 32.U) {
    dataout_write_idx           := dataout_write_idx + 1.U
    dataout_mem.write(dataout_write_idx, dataout_write_data)
  }

  // Generate the data valid signal
  when (rising_edge(start)) {
    dataout_valid               := 0.U
  } .elsewhen (rising_edge(next_out)) {
    dataout_valid               := 1.U
  }

  // Map the blackbox inputs
  idft_top_mock_tss_inst.io.clk     := clock                // Implicit module clock
  idft_top_mock_tss_inst.io.rst     := reset                // dft top has an active high reset 
  idft_top_mock_tss_inst.io.X0      := Mux(datain_read_idx < 32.U, datain_read_data(63,48), 0.U) // Concatenating data into 64 bit blackbox input
  idft_top_mock_tss_inst.io.X1      := Mux(datain_read_idx < 32.U, datain_read_data(47,32), 0.U) // Concatenating data into 64 bit blackbox input
  idft_top_mock_tss_inst.io.X2      := Mux(datain_read_idx < 32.U, datain_read_data(31,16), 0.U) // Concatenating data into 64 bit blackbox input       
  idft_top_mock_tss_inst.io.X3      := Mux(datain_read_idx < 32.U, datain_read_data(15,0),  0.U) // Concatenating data into 64 bit blackbox input
  idft_top_mock_tss_inst.io.next    := rising_edge(start) 
                                                            // Map the dft input data only when pointing to
                                                            // a valid memory location
  // Map the blackbox outputs
  dataout_write_data                := Cat(idft_top_mock_tss_inst.io.Y0,
                                           idft_top_mock_tss_inst.io.Y1,
                                           idft_top_mock_tss_inst.io.Y2,
                                           idft_top_mock_tss_inst.io.Y3)      // dft output data
  next_out                          := idft_top_mock_tss_inst.io.next_out

  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
    IDFTAddresses.idft_ctrlstatus_addr    -> RegFieldGroup("dft_ctrlstatus",Some(""), Seq(
      RegField    (1, start               ),      // Start passing data to the dft blackbox
      RegField    (1, datain_we           ),      // Write enable for the datain memory
      RegField.r  (1, dataout_valid       ))),    // Data Out Valid
    IDFTAddresses.idft_datain_addr_addr   -> Seq(RegField     (5,  datain_write_idx)), // Address for data write, written to datain_write_idx by cepregression.cpp
    IDFTAddresses.idft_datain_data_addr   -> Seq(RegField     (64, datain_write_data)),// Data for data write, written to datain_write_data by cepregression.cpp
    IDFTAddresses.idft_dataout_addr_addr  -> Seq(RegField     (5,  dataout_read_idx)), // Address for data read, written to dataout_read_idx by cepregression.cpp 
    IDFTAddresses.idft_dataout_data_addr  -> Seq(RegField.r   (64, dataout_read_data)) // Data for data write, written to dataout_read_data by cepregression.cpp
  )  // regmap

}
//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------
