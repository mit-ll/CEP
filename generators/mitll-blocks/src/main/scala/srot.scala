//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
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
// Parameters associated with the SROT
case object SROTKey extends Field[Seq[SROTParams]](Nil)

// This trait "connects" the SRoT to the Rocket Chip and passes the parameters down
// to the instantiation
trait CanHaveSROT { this: BaseSubsystem =>
  val SROTNodes = p(SROTKey).map { params =>

    // Initialize the attachment parameters
    val srotattachparams = SROTAttachParams(
      srotparams  = params,
      slave_bus   = pbus,
      master_bus  = fbus
    )

    // Define the SRoT Tilelink module
    val srotmodule = LazyModule(new srotTLModule(srotattachparams)(p))

    // Perform the slave "attachments" to the periphery bus
    srotattachparams.slave_bus.coupleTo("srot_slave") {
      srotmodule.slave_node :*= 
      TLSourceShrinker(16) :*=
      TLFragmenter(srotattachparams.slave_bus) :*=_
   }

    // Perform the master "attachments" to the front bus
    srotattachparams.master_bus.coupleFrom("srot_master") {
      _ := 
      srotmodule.master_node  
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { srotmodule.module.clock := srotattachparams.slave_bus.module.clock }
    InModuleBody { srotmodule.module.reset := srotattachparams.slave_bus.module.reset }

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
      supportsGet         = TransferSizes(1, 8),
      supportsPutFull     = TransferSizes(1, 8),
      supportsPutPartial  = TransferSizes(1, 8),
      supportsArithmetic  = TransferSizes.none,
      supportsLogical     = TransferSizes.none,
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
        visibility        = Seq(AddressSet(
          srotattachparams.srotparams.cep_cores_base_addr,
          srotattachparams.srotparams.cep_cores_depth))
      ))
    )))
    
    // Instantiate the implementation
    lazy val module = new srotTLModuleImp(srotattachparams.srotparams, this)

} // end TLSROTModule
 
class srotTLModuleImp(srotparams: SROTParams, outer: srotTLModule) extends LazyModuleImp(outer) {

  // "Connect" to Slave Node's signals and parameters
  val (slave, slaveEdge)    = outer.slave_node.in(0)

  // Ensure unused channels are tied off
  slave.b.valid   := false.B
  slave.c.ready   := true.B
  slave.e.ready   := true.B

  // "Connect" to Master Node's signals and parameters
  val (master, masterEdge)  = outer.master_node.out(0)

  // Define srot_wrapper blackbox and its associated IO.   Parameters are being
  // used to pass vector sizes (vs constants in a package) to increase flexibility
  // when some vectors might change depending on where the SRoT is instantiated
  class srot_wrapper(   slave_tl_szw              : Int,
                        slave_tl_aiw              : Int,
                        slave_tl_aw               : Int,
                        slave_tl_dbw              : Int,
                        slave_tl_dw               : Int,
                        slave_tl_diw              : Int,
                        master_tl_szw             : Int,
                        master_tl_aiw             : Int,
                        master_tl_aw              : Int,
                        master_tl_dbw             : Int,
                        master_tl_dw              : Int,
                        master_tl_diw             : Int,
                        num_cores                 : BigInt, 
                        core_index_array_packed   : BigInt) extends BlackBox (
      Map(
        "SLAVE_TL_SZW"                  -> IntParam(slave_tl_szw),
        "SLAVE_TL_AIW"                  -> IntParam(slave_tl_aiw),
        "SLAVE_TL_AW"                   -> IntParam(slave_tl_aw),
        "SLAVE_TL_DBW"                  -> IntParam(slave_tl_dbw),
        "SLAVE_TL_DW"                   -> IntParam(slave_tl_dw),
        "SLAVE_TL_DIW"                  -> IntParam(slave_tl_diw),
        "MASTER_TL_SZW"                 -> IntParam(master_tl_szw),
        "MASTER_TL_AIW"                 -> IntParam(master_tl_aiw),
        "MASTER_TL_AW"                  -> IntParam(master_tl_aw),
        "MASTER_TL_DBW"                 -> IntParam(master_tl_dbw),
        "MASTER_TL_DW"                  -> IntParam(master_tl_dw),
        "MASTER_TL_DIW"                 -> IntParam(master_tl_diw),
        // number of LLKI cores
        "LLKI_NUM_CORES"                -> IntParam(num_cores),
        // Array of LLKI base addresses, packed into single bitstream 
        // Each address is 32bit
        // MSB => address 0
        "LLKI_CORE_INDEX_ARRAY_PACKED"  -> IntParam(core_index_array_packed) 
      )
  ) with HasBlackBoxResource {

    val io = IO(new Bundle {
      // Clock and Reset
      val clk               = Input(Clock())
      val rst               = Input(Bool())

      // Slave - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_a_opcode    = Input(UInt(3.W))
      val slave_a_param     = Input(UInt(3.W))
      val slave_a_size      = Input(UInt(slave_tl_szw.W))
      val slave_a_source    = Input(UInt(slave_tl_aiw.W))
      val slave_a_address   = Input(UInt(slave_tl_aw.W))
      val slave_a_mask      = Input(UInt(slave_tl_dbw.W))
      val slave_a_data      = Input(UInt(slave_tl_dw.W))
      val slave_a_corrupt   = Input(Bool())
      val slave_a_valid     = Input(Bool())
      val slave_a_ready     = Output(Bool())

      // Slave - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val slave_d_opcode    = Output(UInt(3.W))
      val slave_d_param     = Output(UInt(3.W))
      val slave_d_size      = Output(UInt(slave_tl_szw.W))
      val slave_d_source    = Output(UInt(slave_tl_aiw.W))
      val slave_d_sink      = Output(UInt(slave_tl_diw.W))
      val slave_d_denied    = Output(Bool())
      val slave_d_data      = Output(UInt(slave_tl_dw.W))
      val slave_d_corrupt   = Output(Bool())
      val slave_d_valid     = Output(Bool())
      val slave_d_ready     = Input(Bool())

      // Master - Tilelink A Channel (Signal order/names from Tilelink Specification v1.8.0)
      val master_a_opcode   = Output(UInt(3.W))
      val master_a_param    = Output(UInt(3.W))
      val master_a_size     = Output(UInt(master_tl_szw.W))
      val master_a_source   = Output(UInt(master_tl_aiw.W))
      val master_a_address  = Output(UInt(master_tl_aw.W))
      val master_a_mask     = Output(UInt(master_tl_dbw.W))
      val master_a_data     = Output(UInt(master_tl_dw.W))
      val master_a_corrupt  = Output(Bool())
      val master_a_valid    = Output(Bool())
      val master_a_ready    = Input(Bool())

      // Master - Tilelink D Channel (Signal order/names from Tilelink Specification v1.8.0)
      val master_d_opcode   = Input(UInt(3.W))
      val master_d_param    = Input(UInt(3.W))
      val master_d_size     = Input(UInt(master_tl_szw.W))
      val master_d_source   = Input(UInt(master_tl_aiw.W))
      val master_d_sink     = Input(UInt(master_tl_diw.W))
      val master_d_denied   = Input(Bool())
      val master_d_data     = Input(UInt(master_tl_dw.W))
      val master_d_corrupt  = Input(Bool())
      val master_d_valid    = Input(Bool())
      val master_d_ready    = Output(Bool())

    })

    // Add the SystemVerilog/Verilog associated with the module
    // Relative to /src/main/resources
    addResource("/vsrc/llki/srot_wrapper.sv")

  } // end class srot_wrapper

  // Pack core index array
  val core_index_array_packed = srotparams.llki_cores_array.foldLeft(BigInt(0)) { 
    (packed, addr) => ((packed << 32) | addr)  }
  val num_cores = srotparams.llki_cores_array.length

  // Instantiate the srot_wrapper
  val srot_wrapper_inst = Module(new srot_wrapper(
    slave_tl_szw                    = slaveEdge.bundle.sizeBits,
    slave_tl_aiw                    = slaveEdge.bundle.sourceBits,
    slave_tl_aw                     = slaveEdge.bundle.addressBits,
    slave_tl_dbw                    = slaveEdge.bundle.dataBits / 8,
    slave_tl_dw                     = slaveEdge.bundle.dataBits,
    slave_tl_diw                    = slaveEdge.bundle.sinkBits,
    master_tl_szw                   = masterEdge.bundle.sizeBits,
    master_tl_aiw                   = masterEdge.bundle.sourceBits,
    master_tl_aw                    = masterEdge.bundle.addressBits,
    master_tl_dbw                   = masterEdge.bundle.dataBits / 8,
    master_tl_dw                    = masterEdge.bundle.dataBits,
    master_tl_diw                   = masterEdge.bundle.sinkBits,
    num_cores                       = num_cores,
    core_index_array_packed         = core_index_array_packed
  ))

  // Connect the Clock and Reset
  srot_wrapper_inst.io.clk                := clock
  srot_wrapper_inst.io.rst                := reset.asBool

  // Connect the Slave A Channel to the Black box IO
  srot_wrapper_inst.io.slave_a_opcode     := slave.a.bits.opcode    
  srot_wrapper_inst.io.slave_a_param      := slave.a.bits.param     
  srot_wrapper_inst.io.slave_a_size       := slave.a.bits.size
  srot_wrapper_inst.io.slave_a_source     := slave.a.bits.source    
  srot_wrapper_inst.io.slave_a_address    := slave.a.bits.address
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
  slave.d.bits.sink                       := srot_wrapper_inst.io.slave_d_sink
  slave.d.bits.denied                     := srot_wrapper_inst.io.slave_d_denied
  slave.d.bits.data                       := srot_wrapper_inst.io.slave_d_data
  slave.d.bits.corrupt                    := srot_wrapper_inst.io.slave_d_corrupt
  slave.d.valid                           := srot_wrapper_inst.io.slave_d_valid
  srot_wrapper_inst.io.slave_d_ready      := slave.d.ready

  // Connect the Master A channel to the Black Box IO
  master.a.bits.opcode                    := srot_wrapper_inst.io.master_a_opcode
  master.a.bits.param                     := srot_wrapper_inst.io.master_a_param
  master.a.bits.size                      := srot_wrapper_inst.io.master_a_size
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
  srot_wrapper_inst.io.master_d_size      := master.d.bits.size
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

