//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : scratch.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : Provides a discrete blackbox scratchpad instantiation to facilitate
//                replacement during ASIC builds
//
//--------------------------------------------------------------------------------------
package mitllBlocks.scratchpad

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
// BEGIN: Scratchpad "Periphery" connections
//--------------------------------------------------------------------------------------

// Parameters associated with the Scratchpad
case object ScratchpadKey extends Field[Seq[ScratchpadParams]]

// This trait "connects" the Scratchpad to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasScratchpad { this: BaseSubsystem =>
  val ScratchpadNodes = p(ScratchpadKey).map { params =>

    // Initialize the attachment parameters
    val scratchpadattachparams = ScratchpadAttachParams(
      scratchpadparams  = params,
      slave_bus         = mbus    // The scratchpad is connected to the Memory Bus
    )

    // Define the SRoT Tilelink module
    val scratchpadModule = LazyModule(new scratchpadTLModule(scratchpadattachparams)(p))

    // Perform the slave "attachments" to the specified bus... fragment as required
    scratchpadattachparams.slave_bus.coupleTo(scratchpadattachparams.scratchpadparams.dev_name) {
      scratchpadModule.slave_node :*=
      TLSourceShrinker(16) :*=
      TLFragmenter(scratchpadattachparams.slave_bus) :*=_
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { scratchpadModule.module.clock := scratchpadattachparams.slave_bus.module.clock }
    InModuleBody { scratchpadModule.module.reset := scratchpadattachparams.slave_bus.module.reset }

}}

//--------------------------------------------------------------------------------------
// END: Scratchpad "Periphery" connections
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Tilelink Scratchpad Module and Module Implementation Declerations
//
// Note: If one does not explicitly put "supportsPutFull" and/or "supportsPutPartial"
//   in the slave parameters, the manager will be instantiated as Read Only (and will
//   show up as such in the device tree.  Also, the chisel optimization that gets
//   "kicked off" because of the inclusion of diplomacy widgets will result in the A
//   channel data bus being tied to ZERO.
//--------------------------------------------------------------------------------------
class scratchpadTLModule(scratchpadattachparams: ScratchpadAttachParams)(implicit p: Parameters) extends LazyModule {

  // Create a Manager / Slave / Sink node
  // These parameters have been copied from SRAM.scala
  val slave_node = TLManagerNode(Seq(TLSlavePortParameters.v1(
    Seq(TLSlaveParameters.v1(
      address             = Seq(AddressSet(
                              scratchpadattachparams.scratchpadparams.slave_address, 
                              scratchpadattachparams.scratchpadparams.slave_depth)),
      resources           = new SimpleDevice(scratchpadattachparams.scratchpadparams.dev_name, 
                              Seq("mitll," + scratchpadattachparams.scratchpadparams.dev_name)).reg("mem"),
      regionType          = RegionType.UNCACHED,
      executable          = true,
      supportsGet         = TransferSizes(1, 8),
      supportsPutFull     = TransferSizes(1, 8),
      supportsPutPartial  = TransferSizes(1, 8),
      supportsArithmetic  = TransferSizes.none,
      supportsLogical     = TransferSizes.none,
      fifoId              = Some(0))),  // requests are handled in order
    beatBytes   = 8,                    // Scratchpad width is fixed at 8 bytes
    minLatency  = 1)))
    
    // Instantiate the implementation
    lazy val module = new scratchpadTLModuleImp(scratchpadattachparams.scratchpadparams, this)

} // end scratchpadTLModule

class scratchpadTLModuleImp(scratchpadparams: ScratchpadParams, outer: scratchpadTLModule) extends LazyModuleImp(outer) {

  // "Connect" to Slave Node's signals and parameters
  val (slave, slaveEdge)    = outer.slave_node.in(0)

  // Ensure unused channels are tied off
  slave.b.valid   := false.B
  slave.c.ready   := true.B
  slave.e.ready   := true.B

  // Define scratchpad_wrapper blackbox and its associated IO
  class scratchpad_wrapper(val address: BigInt, depth: BigInt) extends BlackBox(
      Map(
        "ADDRESS"                       -> IntParam(address),   // Base address of the TL slave
        "DEPTH"                         -> IntParam(depth)      // Address depth of the TL slave
      )
  ) {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk               = Input(Clock())
      val rst               = Input(Bool())

      // Slave - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_a_opcode    = Input(UInt(3.W))
      val slave_a_param     = Input(UInt(3.W))
      val slave_a_size      = Input(UInt(LLKITilelinkParameters.SizeBits.W))
      val slave_a_source    = Input(UInt(LLKITilelinkParameters.SourceBits.W))
      val slave_a_address   = Input(UInt(LLKITilelinkParameters.AddressBits.W))
      val slave_a_mask      = Input(UInt(LLKITilelinkParameters.BeatBytes.W))
      val slave_a_data      = Input(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
      val slave_a_corrupt   = Input(Bool())
      val slave_a_valid     = Input(Bool())
      val slave_a_ready     = Output(Bool())

      // Slave - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_d_opcode    = Output(UInt(3.W))
      val slave_d_param     = Output(UInt(3.W))
      val slave_d_size      = Output(UInt(LLKITilelinkParameters.SizeBits.W))
      val slave_d_source    = Output(UInt(LLKITilelinkParameters.SourceBits.W))
      val slave_d_sink      = Output(UInt(LLKITilelinkParameters.SinkBits.W))
      val slave_d_denied    = Output(Bool())
      val slave_d_data      = Output(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
      val slave_d_corrupt   = Output(Bool())
      val slave_d_valid     = Output(Bool())
      val slave_d_ready     = Input(Bool())

    })
  } // end class scratchpad_wrapper

  // Instantiate the scratchpad_wrapper
  // As the depth parameter is being used to define the size of the instantiated memory, it must be incremented by +1 before
  // passing it down to the scratchpad_wrapper
  val scratchpad_wrapper_inst = Module(new scratchpad_wrapper(scratchpadparams.slave_address, scratchpadparams.slave_depth + 1))

  // The following "requires" are included to avoid size mismatches between the
  // the Rocket Chip buses and the Scratchpad Black Box.  The expected values are inherited
  // from the cep_addresses package and must match those in "top_pkg.sv", borrowed from OpenTitan
  //
  // Exceptions:
  //  - slaveEdge address gets optimized down to 31-bits during chisel generation
  //  - slaveEdge sink bits are 1, but masterEdge sink bits are 2 
  //  - slaveEdge size bits are 3, but masterEdge size bits are 4
  //
  require(slaveEdge.bundle.addressBits  == LLKITilelinkParameters.AddressBits - 1, s"SCRATCHPAD: slaveEdge addressBits exp/act ${LLKITilelinkParameters.AddressBits - 1}/${slaveEdge.bundle.addressBits}")
  require(slaveEdge.bundle.dataBits     == LLKITilelinkParameters.BeatBytes * 8, s"SCRATCHPAD: slaveEdge dataBits exp/act ${LLKITilelinkParameters.BeatBytes * 8}/${slaveEdge.bundle.dataBits}")
  require(slaveEdge.bundle.sourceBits   == LLKITilelinkParameters.SourceBits, s"SCRATCHPAD: slaveEdge sourceBits exp/act ${LLKITilelinkParameters.SourceBits}/${slaveEdge.bundle.sourceBits}")
  require(slaveEdge.bundle.sinkBits     == LLKITilelinkParameters.SinkBits - 1, s"SCRATCHPAD: slaveEdge sinkBits exp/act ${LLKITilelinkParameters.SinkBits - 1}/${slaveEdge.bundle.sinkBits}")
  require(slaveEdge.bundle.sizeBits     == LLKITilelinkParameters.SizeBits, s"SCRATCHPAD: slaveEdge sizeBits exp/act ${LLKITilelinkParameters.SizeBits}/${slaveEdge.bundle.sizeBits}")

  // Connect the Clock and Reset
  scratchpad_wrapper_inst.io.clk                := clock
  scratchpad_wrapper_inst.io.rst                := reset

  // Connect the Slave A Channel to the Black box IO
  scratchpad_wrapper_inst.io.slave_a_opcode     := slave.a.bits.opcode    
  scratchpad_wrapper_inst.io.slave_a_param      := slave.a.bits.param     
  scratchpad_wrapper_inst.io.slave_a_size       := slave.a.bits.size
  scratchpad_wrapper_inst.io.slave_a_source     := slave.a.bits.source    
  scratchpad_wrapper_inst.io.slave_a_address    := Cat(0.U(1.W), slave.a.bits.address)
  scratchpad_wrapper_inst.io.slave_a_mask       := slave.a.bits.mask      
  scratchpad_wrapper_inst.io.slave_a_data       := slave.a.bits.data      
  scratchpad_wrapper_inst.io.slave_a_corrupt    := slave.a.bits.corrupt   
  scratchpad_wrapper_inst.io.slave_a_valid      := slave.a.valid          
  slave.a.ready                                 := scratchpad_wrapper_inst.io.slave_a_ready  

  // Connect the Slave D Channel to the Black Box IO    
  slave.d.bits.opcode                           := scratchpad_wrapper_inst.io.slave_d_opcode
  slave.d.bits.param                            := scratchpad_wrapper_inst.io.slave_d_param
  slave.d.bits.size                             := scratchpad_wrapper_inst.io.slave_d_size
  slave.d.bits.source                           := scratchpad_wrapper_inst.io.slave_d_source
  slave.d.bits.sink                             := scratchpad_wrapper_inst.io.slave_d_sink(0)
  slave.d.bits.denied                           := scratchpad_wrapper_inst.io.slave_d_denied
  slave.d.bits.data                             := scratchpad_wrapper_inst.io.slave_d_data
  slave.d.bits.corrupt                          := scratchpad_wrapper_inst.io.slave_d_corrupt
  slave.d.valid                                 := scratchpad_wrapper_inst.io.slave_d_valid
  scratchpad_wrapper_inst.io.slave_d_ready      := slave.d.ready
  
} // end scratchpadTLModuleImp
//--------------------------------------------------------------------------------------
// END: Tilelink Scratchpad Module and Module Implementation Declerations
//--------------------------------------------------------------------------------------

