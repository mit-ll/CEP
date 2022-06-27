//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : cep_registers.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : General Registers for the CEP
//
//--------------------------------------------------------------------------------------
package mitllBlocks.cep_registers

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
case object PeripheryCEPRegistersKey extends Field[Seq[CEPREGSParams]](Nil)

// This trait "connects" the core to the Rocket Chip and passes the parameters down
// to the instantiation
trait CanHavePeripheryCEPRegisters { this: BaseSubsystem =>
  val cepregsnode = p(PeripheryCEPRegistersKey).map { params =>

    // Initialize the attachment parameters
    val cepregsattachparams = CEPREGSAttachParams(
      cepregsparams = params,
      slave_bus     = pbus
    )

    // Instantiate th TL module.  Note: This name shows up in the generated verilog hiearchy
    // and thus should be unique to this core and NOT a verilog reserved keyword
    val cepregsmodule = LazyModule(new TLModule(cepregsattachparams)(p))

    // Perform the slave "attachments" to the slave bus
    cepregsattachparams.slave_bus.coupleTo(cepregsattachparams.cepregsparams.dev_name + "_slave") {
      cepregsmodule.slave_node :*=
      TLFragmenter(cepregsattachparams.slave_bus.beatBytes, cepregsattachparams.slave_bus.blockBytes) :*= _
    }

    // Explicitly connect the clock and reset (the module will be clocked off of the slave bus)
    InModuleBody { cepregsmodule.module.reset := cepregsattachparams.slave_bus.module.reset }
    InModuleBody { cepregsmodule.module.clock := cepregsattachparams.slave_bus.module.clock }

}}
//--------------------------------------------------------------------------------------
// END: Module "Periphery" connections
//--------------------------------------------------------------------------------------
 


//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module
//--------------------------------------------------------------------------------------
class TLModule(cepregsattachparams: CEPREGSAttachParams)(implicit p: Parameters) extends LazyModule {

  // Create the RegisterRouter node
  val slave_node = TLRegisterNode(
    address     = Seq(AddressSet(
                    cepregsattachparams.cepregsparams.slave_base_addr, 
                    cepregsattachparams.cepregsparams.slave_depth)),
    device      = new SimpleDevice(cepregsattachparams.cepregsparams.dev_name + "-slave", 
                    Seq("mitll," + cepregsattachparams.cepregsparams.dev_name + "-slave")),
    beatBytes   = cepregsattachparams.slave_bus.beatBytes
  )

  // Instantiate the implementation
  lazy val module = new TLModuleImp(cepregsattachparams.cepregsparams, this)

}
//--------------------------------------------------------------------------------------
// END: TileLink Module
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// BEGIN: TileLink Module Implementation
//--------------------------------------------------------------------------------------
class TLModuleImp(cepregsparams: CEPREGSParams, outer: TLModule) extends LazyModuleImp(outer) {

  // The following class is used to import all the miscellaneous black-box resources
  // that previously resided in srot.scala.  This effectively creates a dummy module with
  // no IO or underlying logic (the module definition on the verilog side is contained
  // in llki_pkg.sv... which is required otherwise the design won't full elaborate)
  class import_resources extends BlackBox with HasBlackBoxResource {

    val io = IO(new Bundle {})

    // LLKI Resources
    addResource("/vsrc/llki/llki_pp_wrapper.sv")
    addResource("/vsrc/llki/mock_tss_fsm.sv")
    addResource("/vsrc/llki/prim_generic_ram_1p.sv")
    addResource("/vsrc/llki/tlul_err.sv")
    addResource("/vsrc/llki/tlul_adapter_reg.sv")
    addResource("/vsrc/llki/tlul_fifo_sync.sv")
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_fifo_sync.sv")
    addResource("/vsrc/opentitan/hw/ip/tlul/rtl/tlul_adapter_host.sv")

    // Packages and Supporting Files
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_assert.sv")
    addResource("/vsrc/opentitan/hw/ip/tlul/rtl/tlul_pkg.sv")
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_util_memload.svh")
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_assert_dummy_macros.svh")
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_assert_standard_macros.svh")
    addResource("/vsrc/opentitan/hw/ip/prim/rtl/prim_util_pkg.sv")
    addResource("/vsrc/llki/top_pkg.sv")
    addResource("/vsrc/llki/llki_pkg.sv")

  }
  val import_resources_inst = Module(new import_resources)


  class Version_Class extends Bundle {
          val reserved            = UInt(48.W)
          val major               = UInt(8.W)
          val minor               = UInt(8.W)
        }
        object Version_Class {
          def init: Version_Class = {
            val wire = Wire(new Version_Class)
            wire.reserved         := 0.U
            wire.major            := CEPVersion.CEP_MAJOR_VERSION.U
            wire.minor            := CEPVersion.CEP_MINOR_VERSION.U
            wire
          }
        }
        val version_register      = RegInit(Version_Class.init)

    class scratch_Class extends Bundle {
      val word0               = UInt(64.W)
      val word1               = UInt(64.W)
      val word2               = UInt(64.W)
      val word3               = UInt(64.W)
      val word4               = UInt(64.W)
      val word5               = UInt(64.W)
      val word6               = UInt(64.W)
      val word7               = UInt(64.W)
      val word32_0            = UInt(32.W)
    }
    object scratch_Class {
      def init: scratch_Class = {
        val wire = Wire(new scratch_Class)
          wire.word0          := 0.U
          wire.word1          := 0.U
          wire.word2          := 0.U
          wire.word3          := 0.U
          wire.word4          := 0.U
          wire.word5          := 0.U
          wire.word6          := 0.U
          wire.word7          := 0.U
          wire.word32_0       := 0.U
          wire
        }
      }
   val scratch             = RegInit(scratch_Class.init)
   val core0_status        = RegInit(0. U(64.W))
   val core1_status        = RegInit(0. U(64.W))
   val core2_status        = RegInit(0. U(64.W))
   val core3_status        = RegInit(0. U(64.W))
   //
   // support 4 locks: lock0 - lock3
   //
   class lock_Class extends Bundle {
       val isLocked          = UInt(1.W)
       val coreId            = UInt(7.W)
    }
    object lock_Class {
      def init: lock_Class = {
      val wire = Wire(new lock_Class)
         wire.isLocked     := 0.U
         wire.coreId       := 0.U
     wire
      }
    }
    val lock0 = RegInit(lock_Class.init)
    val lock1 = RegInit(lock_Class.init)
    val lock2 = RegInit(lock_Class.init)
    val lock3 = RegInit(lock_Class.init)
   //
   // TestNset Registers
   //
   // SW's Write Only and self-clear
   val reqLock     = RegInit(0.U(1.W))
   val releaseLock = RegInit(0.U(1.W))
   val lockNum     = RegInit(0.U(2.W))      
   val reqId       = RegInit(0.U(7.W))
   //   
   // SW's Read-Only
   //
   //  detect a change in either reqLock or releaseLock
   //
   when (reqLock === 1.U) {
      // self clear
      reqLock     := 0.U
      //
      // check if lock is available then grant it..
      //
      when ((lockNum === 0.U) & (lock0.isLocked === 0.U)) {
         lock0.coreId   := reqId
	 lock0.isLocked := 1.U	
      }
      .elsewhen ((lockNum === 1.U) & (lock1.isLocked === 0.U)) {
         lock1.coreId   := reqId
         lock1.isLocked := 1.U      
      }
      .elsewhen ((lockNum === 2.U) & (lock2.isLocked === 0.U)) {
         lock2.coreId   := reqId
         lock2.isLocked := 1.U      
      }
      .elsewhen ((lockNum === 3.U) & (lock3.isLocked === 0.U)) {
         lock3.coreId   := reqId
         lock3.isLocked := 1.U      
      }           
   }
   //
   // Release lock when done
   //
   when (releaseLock === 1.U) {
      // self clear
      releaseLock  := 0.U
      //
      // Assume all core are cor-operating and no cheating => release only by the master and after have lock
      //
      when ((lockNum === 0.U) & (lock0.isLocked === 1.U) & (lock0.coreId === reqId)) {
         lock0.isLocked := 0.U
      }
      .elsewhen ((lockNum === 1.U) & (lock1.isLocked === 1.U) & (lock1.coreId === reqId)) {
         lock1.isLocked := 0.U
      }      
      .elsewhen ((lockNum === 2.U) & (lock2.isLocked === 1.U) & (lock2.coreId === reqId)) {
         lock2.isLocked := 0.U
      }      
      .elsewhen ((lockNum === 3.U) & (lock3.isLocked === 1.U) & (lock3.coreId === reqId)) {
         lock3.isLocked := 0.U
      }      
   }
  
  // Define the register map
  // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
  outer.slave_node.regmap (
    CEPRegisterAddresses.version_register -> RegFieldGroup("cep_version_register", Some(""),Seq(
      RegField.r  (48, version_register.reserved),
      RegField.r  (8,  version_register.major),
      RegField.r  (8,  version_register.minor))),
    CEPRegisterAddresses.testNset -> RegFieldGroup("test_and_set_register", Some(""),Seq(
      RegField.r  (1,  lock0.isLocked),         // [7:0]
      RegField.r  (7,  lock0.coreId),           // 
      RegField.r  (1,  lock1.isLocked),         // [15:8]
      RegField.r  (7,  lock1.coreId),           // 
      RegField.r  (1,  lock2.isLocked),         // [23:16]
      RegField.r  (7,  lock2.coreId),           // 
      RegField.r  (1,  lock3.isLocked),         // [31:24]
      RegField.r  (7,  lock3.coreId),           // 
      RegField    (1,  reqLock),       // [32]
      RegField    (1,  releaseLock),   // [33]
      RegField    (2,  lockNum),       // [35:34]             
      RegField.r  (4,  0.U),           // [39:36]     
      RegField    (7,  reqId),         // [46:40]
    RegField.r  (17, 0.U))),         // [63:47]     
      CEPRegisterAddresses.scratch_w0 -> RegFieldGroup("scratch_in0", Some(""),    Seq(RegField  (64, scratch.word0))),
      CEPRegisterAddresses.scratch_w1 -> RegFieldGroup("scratch_in1", Some(""),    Seq(RegField  (64, scratch.word1))),
      CEPRegisterAddresses.scratch_w2 -> RegFieldGroup("scratch_in2", Some(""),    Seq(RegField  (64, scratch.word2))),
      CEPRegisterAddresses.scratch_w3 -> RegFieldGroup("scratch_in3", Some(""),    Seq(RegField  (64, scratch.word3))),
      CEPRegisterAddresses.scratch_w4 -> RegFieldGroup("scratch_in4", Some(""),    Seq(RegField  (64, scratch.word4))),
      CEPRegisterAddresses.scratch_w5 -> RegFieldGroup("scratch_in5", Some(""),    Seq(RegField  (64, scratch.word5))),
      CEPRegisterAddresses.scratch_w6 -> RegFieldGroup("scratch_in6", Some(""),    Seq(RegField  (64, scratch.word6))),
      CEPRegisterAddresses.scratch_w7 -> RegFieldGroup("scratch_in7", Some(""),    Seq(RegField  (64, scratch.word7))),
      CEPRegisterAddresses.core0_status -> RegFieldGroup("core0_status", Some(""),    Seq(RegField  (64, core0_status))),     
      CEPRegisterAddresses.core1_status -> RegFieldGroup("core1_status", Some(""),    Seq(RegField  (64, core1_status))),     
      CEPRegisterAddresses.core2_status -> RegFieldGroup("core2_status", Some(""),    Seq(RegField  (64, core2_status))),     
      CEPRegisterAddresses.core3_status -> RegFieldGroup("core3_status", Some(""),    Seq(RegField  (64, core3_status)))
    ) // regmap
}
//--------------------------------------------------------------------------------------
// END: TileLink Module Implementation
//--------------------------------------------------------------------------------------

