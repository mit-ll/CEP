//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : srot.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the Surrogate Root of Trust (SRoT)
//                The Surrogate Root of Trust serves as a slave on the periphery
//                bus AND a master on the Front Bus
//
//--------------------------------------------------------------------------------------
package mitllBlocks.srot

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
// BEGIN: SRoT "Periphery" connections
//--------------------------------------------------------------------------------------

// The following class is used to pass paramaters down into the SROT
case class SROTParams(
  slave_address     : BigInt,
  slave_depth       : BigInt
)

// The following parameter pass attachment info to the lower level objects/classes/etc.
case class SROTAttachParams(
  srotparams        : SROTParams,
  slave_bus         : TLBusWrapper,
  master_bus        : TLBusWrapper
)

// Parameters associated with the SROT
case object SROTKey extends Field[Seq[SROTParams]]

// This trait "connects" the SRoT to the Rocket Chip and passes the parameters down
// to the instantiation
trait HasSROT { this: BaseSubsystem =>
  val SROTNodes = p(SROTKey).map { params =>

    // Initialize the attachment parameters
    val srotattachparams = SROTAttachParams(
      srotparams  = params,
      slave_bus   = pbus,
      master_bus  = fbus
    )

    // Define the SRoT Tilelink module
    val srotModule = LazyModule(new srotTLModule(srotattachparams)(p))

    // Perform the slave "attachments" to the periphery bus
    srotattachparams.slave_bus.coupleTo("srot_slave") {
      srotModule.slave_node :*= 
      TLSourceShrinker(16) :*= _
    }

    // Perform the master "attachments" to the front bus
    srotattachparams.master_bus.coupleFrom("srot_master") {
      _ := 
      srotModule.master_node  
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { srotModule.module.clock := srotattachparams.slave_bus.module.clock }
    InModuleBody { srotModule.module.reset := srotattachparams.slave_bus.module.reset }

}}

//--------------------------------------------------------------------------------------
// END: SRoT "Periphery" connections
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Tilelink SROT Module and Module Implementation Declerations
//
// Note: If one does not explicitly put "supportsPutFull" and/or "supportsPullPartial"
//   in the slave parameters, the manager will be instantiated as Read Only (and will
//   show up as such in the device tree.  Also, the chisel optimization that gets
//   "kicked off" because of the inclusion of diplomacy widgets will result in the A
//   channel data bus being tied to ZERO.
//--------------------------------------------------------------------------------------
class srotTLModule(srotattachparams: SROTAttachParams)(implicit p: Parameters) extends LazyModule {

  // Create a Manager / Slave / Sink node
  val slave_node = TLManagerNode(Seq(TLSlavePortParameters.v1(
    Seq(TLSlaveParameters.v1(
      address             = Seq(AddressSet(
                              srotattachparams.srotparams.slave_address, 
                              srotattachparams.srotparams.slave_depth)),
      resources           = new SimpleDevice("srot-slave", Seq("mitll,srot-slave")).reg,
      regionType          = RegionType.IDEMPOTENT,
      supportsGet         = TransferSizes(1, srotattachparams.slave_bus.blockBytes),
      supportsPutFull     = TransferSizes(1, srotattachparams.slave_bus.blockBytes),
      supportsPutPartial  = TransferSizes(1, srotattachparams.slave_bus.blockBytes),
      fifoId              = Some(0))), // requests are handled in order
    beatBytes = LLKITilelinkParameters.BeatBytes)))

    // Create a Client / Master / Source node
    // The sourceID paramater assumes there are two masters on the fbus (Debug Module, SRoT)
    // This client node is "constained" to only talk to the CEP cores (via the visibility
    // parameter)
    val master_node = TLClientNode(Seq(TLMasterPortParameters.v1(
      Seq(TLMasterParameters.v1(
        name              = "srot_master0",
        sourceId          = IdRange(0, 15), 
        requestFifo       = true,
        visibility        = Seq(AddressSet(CEPBaseAddresses.cep_cores_base_addr, CEPBaseAddresses.cep_cores_depth))
      ))
    )))
    
    // Instantiate the implementation
    lazy val module = new srotTLModuleImp(srotattachparams.srotparams, this)

} // end TLSROTModule
 
class srotTLModuleImp(srotparams: SROTParams, outer: srotTLModule) extends LazyModuleImp(outer) {

  // "Connect" to Slave Node's signals and parameters
  val (slave, slaveEdge)    = outer.slave_node.in(0)

  // "Connect" to Master Node's signals and parameters
  val (master, masterEdge)  = outer.master_node.out(0)

  // Define srot_wrapper blackbox and its associated IO
  class srot_wrapper(val address: BigInt, depth: BigInt) extends BlackBox(
      Map(
        "ADDRESS"   -> IntParam(address), // Base address of the TL slave
        "DEPTH"     -> IntParam(depth)    // Address depth of the TL slave
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

      // Master - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val master_a_opcode   = Output(UInt(3.W))
      val master_a_param    = Output(UInt(3.W))
      val master_a_size     = Output(UInt(LLKITilelinkParameters.SizeBits.W))
      val master_a_source   = Output(UInt(LLKITilelinkParameters.SourceBits.W))
      val master_a_address  = Output(UInt(LLKITilelinkParameters.AddressBits.W))
      val master_a_mask     = Output(UInt(LLKITilelinkParameters.BeatBytes.W))
      val master_a_data     = Output(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
      val master_a_corrupt  = Output(Bool())
      val master_a_valid    = Output(Bool())
      val master_a_ready    = Input(Bool())

      // Master - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val master_d_opcode   = Input(UInt(3.W))
      val master_d_param    = Input(UInt(3.W))
      val master_d_size     = Input(UInt(LLKITilelinkParameters.SizeBits.W))
      val master_d_source   = Input(UInt(LLKITilelinkParameters.SourceBits.W))
      val master_d_sink     = Input(UInt(LLKITilelinkParameters.SinkBits.W))
      val master_d_denied   = Input(Bool())
      val master_d_data     = Input(UInt((LLKITilelinkParameters.BeatBytes * 8).W))
      val master_d_corrupt  = Input(Bool())
      val master_d_valid    = Input(Bool())
      val master_d_ready    = Output(Bool())

    })
  } // end class srot_wrapper

    
  // Instantiate the srot_wrapper
  val srot_wrapper_inst = Module(new srot_wrapper(srotparams.slave_address, srotparams.slave_depth))

  // The following "requires" are included to avoid size mismatches between the
  // the Rocket Chip buses and the SRoT Black Box.  The expected values are inhereited
  // from the cep_addresses package and must match those in "top_pkg.sv", borrowed from OpenTitan
  //
  // Exceptions:
  //  - slaveEdge address gets optimized down to 31-bits during chisel generation
  //  - slaveEdge sink bits are 1, but masterEdge sink bits are 2 
  //  - slaveEdge size bits are 3, but masterEdge size bits are 4
  //
  require(slaveEdge.bundle.addressBits  == LLKITilelinkParameters.AddressBits - 1, s"SROT: slaveEdge addressBits exp/act ${LLKITilelinkParameters.AddressBits - 1}/${slaveEdge.bundle.addressBits}")
  require(slaveEdge.bundle.dataBits     == LLKITilelinkParameters.BeatBytes * 8, s"SROT: slaveEdge dataBits exp/act ${LLKITilelinkParameters.BeatBytes * 8}/${slaveEdge.bundle.dataBits}")
  require(slaveEdge.bundle.sourceBits   == LLKITilelinkParameters.SourceBits, s"SROT: slaveEdge sourceBits exp/act ${LLKITilelinkParameters.SourceBits}/${slaveEdge.bundle.sourceBits}")
  require(slaveEdge.bundle.sinkBits     == LLKITilelinkParameters.SinkBits - 1, s"SROT: slaveEdge sinkBits exp/act ${LLKITilelinkParameters.SinkBits - 1}/${slaveEdge.bundle.sinkBits}")
  require(slaveEdge.bundle.sizeBits     == LLKITilelinkParameters.SizeBits, s"SROT: slaveEdge sizeBits exp/act ${LLKITilelinkParameters.SizeBits}/${slaveEdge.bundle.sizeBits}")

  require(masterEdge.bundle.addressBits == LLKITilelinkParameters.AddressBits, s"SROT: masterEdge addressBits exp/act ${LLKITilelinkParameters.AddressBits}/${masterEdge.bundle.addressBits}")
  require(masterEdge.bundle.dataBits    == LLKITilelinkParameters.BeatBytes * 8, s"SROT: masterEdge dataBits exp/act ${LLKITilelinkParameters.BeatBytes * 8}/${masterEdge.bundle.dataBits}")
  require(masterEdge.bundle.sourceBits  == LLKITilelinkParameters.SourceBits, s"SROT: masterEdge sourceBits exp/act ${LLKITilelinkParameters.SourceBits}/${masterEdge.bundle.sourceBits}")
  require(masterEdge.bundle.sinkBits    == LLKITilelinkParameters.SinkBits, s"SROT: masterEdge sinkBits exp/act ${LLKITilelinkParameters.SinkBits}/${masterEdge.bundle.sinkBits}")
  require(masterEdge.bundle.sizeBits    == LLKITilelinkParameters.SizeBits + 1, s"SROT: masterEdge sizeBits exp/act ${LLKITilelinkParameters.SizeBits + 1}/${masterEdge.bundle.sizeBits}")

  // Connect the Clock and Reset
  srot_wrapper_inst.io.clk                := clock
  srot_wrapper_inst.io.rst                := reset

  // Connect the Slave A Channel to the Black box IO
  srot_wrapper_inst.io.slave_a_opcode     := slave.a.bits.opcode    
  srot_wrapper_inst.io.slave_a_param      := slave.a.bits.param     
  srot_wrapper_inst.io.slave_a_size       := slave.a.bits.size
  srot_wrapper_inst.io.slave_a_source     := slave.a.bits.source    
  srot_wrapper_inst.io.slave_a_address    := Cat(0.U(1.W), slave.a.bits.address)
  srot_wrapper_inst.io.slave_a_mask       := slave.a.bits.mask      
  srot_wrapper_inst.io.slave_a_data       := slave.a.bits.data      
  srot_wrapper_inst.io.slave_a_corrupt    := slave.a.bits.corrupt   
  srot_wrapper_inst.io.slave_a_valid      := slave.a.valid          
  slave.a.ready                           := srot_wrapper_inst.io.slave_a_ready  

  // Connect the Slave D Channel to the Black Box IO    
  slave.d.bits.opcode                     := srot_wrapper_inst.io.slave_d_opcode
  slave.d.bits.param                      := srot_wrapper_inst.io.slave_d_param
  slave.d.bits.size                       := srot_wrapper_inst.io.slave_d_size
  slave.d.bits.source                     := srot_wrapper_inst.io.slave_d_source
  slave.d.bits.sink                       := srot_wrapper_inst.io.slave_d_sink(0)
  slave.d.bits.denied                     := srot_wrapper_inst.io.slave_d_denied
  slave.d.bits.data                       := srot_wrapper_inst.io.slave_d_data
  slave.d.bits.corrupt                    := srot_wrapper_inst.io.slave_d_corrupt
  slave.d.valid                           := srot_wrapper_inst.io.slave_d_valid
  srot_wrapper_inst.io.slave_d_ready      := slave.d.ready

  // Connect the Master A channel to the Black Box IO
  master.a.bits.opcode                    := srot_wrapper_inst.io.master_a_opcode
  master.a.bits.param                     := srot_wrapper_inst.io.master_a_param
  master.a.bits.size                      := Cat(0.U(1.W), srot_wrapper_inst.io.master_a_size)
  master.a.bits.source                    := srot_wrapper_inst.io.master_a_source
  master.a.bits.address                   := srot_wrapper_inst.io.master_a_address
  master.a.bits.mask                      := srot_wrapper_inst.io.master_a_mask
  master.a.bits.data                      := srot_wrapper_inst.io.master_a_data
  master.a.bits.corrupt                   := srot_wrapper_inst.io.master_a_corrupt
  master.a.valid                          := srot_wrapper_inst.io.master_a_valid
  srot_wrapper_inst.io.master_a_ready     := master.a.ready  

  // Connect the Master D channel to the Black Box IO
  srot_wrapper_inst.io.master_d_opcode    := master.d.bits.opcode
  srot_wrapper_inst.io.master_d_param     := master.d.bits.param
  srot_wrapper_inst.io.master_d_size      := master.d.bits.size(2,0)
  srot_wrapper_inst.io.master_d_source    := master.d.bits.source
  srot_wrapper_inst.io.master_d_sink      := master.d.bits.sink
  srot_wrapper_inst.io.master_d_denied    := master.d.bits.denied
  srot_wrapper_inst.io.master_d_data      := master.d.bits.data
  srot_wrapper_inst.io.master_d_corrupt   := master.d.bits.corrupt
  srot_wrapper_inst.io.master_d_valid     := master.d.valid
  master.d.ready                          := srot_wrapper_inst.io.master_d_ready

} // end TLSROTModuleImp
//--------------------------------------------------------------------------------------
// END: Tilelink SROT Module and Module Implementation Declerations
//--------------------------------------------------------------------------------------

