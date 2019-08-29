//
// Copyright (C) 2019 Massachusetts Institute of Technology
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

  			regmap (
          CEPRegisterAddresses.version_register -> RegFieldGroup("cep_version_register", Some(""),Seq(
            RegField.r  (48, version_register.reserved),
            RegField.r  (8,  version_register.major),
            RegField.r  (8,  version_register.minor)))
  			)

  		}

	}
//--------------------------------------------------------------------------------------
// END: TileLink Test Register
//--------------------------------------------------------------------------------------

