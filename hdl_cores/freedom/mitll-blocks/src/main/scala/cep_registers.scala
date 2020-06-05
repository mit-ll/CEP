//
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File         : cep_registers.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : General Registers for the CEP
//
package mitllBlocks.cep_registers

import Chisel._
import freechips.rocketchip.config.Field
import freechips.rocketchip.subsystem.{BaseSubsystem, PeripheryBusKey}
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.regmapper._
import freechips.rocketchip.tilelink._

import mitllBlocks.cep_addresses._

//--------------------------------------------------------------------------------------
// BEGIN: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------
case object PeripheryCEPRegistersKey extends Field[Seq[CEPREGSParams]]

trait HasPeripheryCEPRegisters { this: BaseSubsystem =>
  val CEPRegistersNodes = p(PeripheryCEPRegistersKey).map { ps =>
    CEPREGS.attach(CEPREGSAttachParams(ps, pbus))
  }
}

case class CEPREGSParams(address: BigInt)

case class CEPREGSAttachParams(
  regparams         : CEPREGSParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLCEPREGS(busWidthBytes: Int, params: CEPREGSParams)(implicit p: Parameters)
  extends CEPREGS(busWidthBytes, params) with HasTLControlRegMap

object CEPREGS {

  def attach(params: CEPREGSAttachParams): TLCEPREGS = {
    implicit val p = params.p
    val cepregs = LazyModule(new TLCEPREGS(params.controlBus.beatBytes, params.regparams))

    params.controlBus.coupleTo(s"slave_named_cepregs") {
      cepregs.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { cepregs.module.clock := params.controlBus.module.clock }
    InModuleBody { cepregs.module.reset := params.controlBus.module.reset }

    cepregs
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: TileLink Test Register
//--------------------------------------------------------------------------------------
abstract class CEPREGS(busWidthBytes: Int, val c: CEPREGSParams)(implicit p: Parameters)
    extends RegisterRouter(
      RegisterRouterParams(
        name = "cepregs",
        compat = Seq("mitll,cepregs"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

      val busWidthBits = busWidthBytes * 8
      require(busWidthBits == 64, "CEPREGS: busWidthBits must be 64")

  		ResourceBinding {Resource(ResourceAnchors.aliases, "cepregs").bind(ResourceAlias(device.label))}

		lazy val module = new LazyModuleImp(this) {
	
        class Version_Class extends Bundle {
          val reserved            = UInt(48.W)
          val major               = UInt(8.W)
          val minor               = UInt(8.W)
        }
        object Version_Class {
          def init: Version_Class = {
            val wire = Wire(new Version_Class)
            wire.reserved         := 0.U
            wire.major            := UInt(CEPVersion.CEP_MAJOR_VERSION)
            wire.minor            := UInt(CEPVersion.CEP_MINOR_VERSION)
            wire
          }
        }
        val version_register      = RegInit(Version_Class.init)
//
// Tony Duong: 05/13/20 Added scratchpad and TestNset registers for multicore sync/communication
//
	class scratch_Class extends Bundle {
                val word0               = UInt(64.W)
                val word1               = UInt(64.W)
                val word2               = UInt(64.W)
                val word3               = UInt(64.W)
                val word4               = UInt(64.W)
                val word5               = UInt(64.W)
                val word6               = UInt(64.W)
                val word7               = UInt(64.W)
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
   //
   regmap (
          CEPRegisterAddresses.version_register -> RegFieldGroup("cep_version_register", Some(""),Seq(
            RegField.r  (48, version_register.reserved),
            RegField.r  (8,  version_register.major),
            RegField.r  (8,  version_register.minor))),
	    //
	    // Added 05/13/20, Tony Duong to help bare metal Muxtex testing.
	    //
          CEPRegisterAddresses.testNset -> RegFieldGroup("test and set register", Some(""),Seq(
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
          CEPRegisterAddresses.scratch_w0 -> RegFieldGroup("scratch_in0", Some("scratch word 0"),    Seq(RegField  (64, scratch.word0))),
          CEPRegisterAddresses.scratch_w1 -> RegFieldGroup("scratch_in1", Some("scratch word 1"),    Seq(RegField  (64, scratch.word1))),
          CEPRegisterAddresses.scratch_w2 -> RegFieldGroup("scratch_in2", Some("scratch word 2"),    Seq(RegField  (64, scratch.word2))),
          CEPRegisterAddresses.scratch_w3 -> RegFieldGroup("scratch_in3", Some("scratch word 3"),    Seq(RegField  (64, scratch.word3))),
          CEPRegisterAddresses.scratch_w4 -> RegFieldGroup("scratch_in4", Some("scratch word 4"),    Seq(RegField  (64, scratch.word4))),
          CEPRegisterAddresses.scratch_w5 -> RegFieldGroup("scratch_in5", Some("scratch word 5"),    Seq(RegField  (64, scratch.word5))),
          CEPRegisterAddresses.scratch_w6 -> RegFieldGroup("scratch_in6", Some("scratch word 6"),    Seq(RegField  (64, scratch.word6))),
          CEPRegisterAddresses.scratch_w7 -> RegFieldGroup("scratch_in7", Some("scratch word 7"),    Seq(RegField  (64, scratch.word7))),
          CEPRegisterAddresses.core0_status -> RegFieldGroup("core0 Status", Some("core0 status"),    Seq(RegField  (64, core0_status))),	  
          CEPRegisterAddresses.core1_status -> RegFieldGroup("core1 Status", Some("core0 status"),    Seq(RegField  (64, core1_status))),	  
          CEPRegisterAddresses.core2_status -> RegFieldGroup("core2 Status", Some("core0 status"),    Seq(RegField  (64, core2_status))),	  
          CEPRegisterAddresses.core3_status -> RegFieldGroup("core3 Status", Some("core0 status"),    Seq(RegField  (64, core3_status))),	  
	  )
	}
}
//--------------------------------------------------------------------------------------
// END: TileLink Test Register
//--------------------------------------------------------------------------------------

