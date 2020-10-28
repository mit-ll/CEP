//--------------------------------------------------------------------------------------
// Copyright (C) 2020 Massachusetts Institute of Technology
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
case object PeripheryIIRKey extends Field[Seq[IIRParams]]

trait HasPeripheryIIR { this: BaseSubsystem =>
  val IIRNodes = p(PeripheryIIRKey).map { ps =>
    IIR.attach(IIRAttachParams(ps, pbus))   // pbus = Periphery Bus
  }
}

case class IIRParams(address: BigInt)

case class IIRAttachParams(
  iirparams         : IIRParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLIIR(busWidthBytes: Int, params: IIRParams)(implicit p: Parameters)
  extends IIR(busWidthBytes, params) with HasTLControlRegMap

object IIR {

  def attach(params: IIRAttachParams): TLIIR = {
    implicit val p = params.p
    val iir = LazyModule(new TLIIR(params.controlBus.beatBytes, params.iirparams))

    // Connect our module to the specified bus (per the controlBus parameter)
    params.controlBus.coupleTo(s"slave_named_iir") {
      iir.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { iir.module.clock := params.controlBus.module.clock }
    InModuleBody { iir.module.reset := params.controlBus.module.reset }

    iir
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: IIR TileLink Module
//--------------------------------------------------------------------------------------
abstract class IIR(busWidthBytes: Int, val c: IIRParams)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "iir",
        compat = Seq("mitll,iir"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "iir").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Macro definition for creating rising edge detectors
            def rising_edge(x: Bool)    = x && !RegNext(x)

            // Instantitate the IIR blackbox
            val blackbox                = Module(new IIR_filter)

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

            iir_reset_re := rising_edge(iir_reset)

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
//            next_out                    := (count === 9.U)
// Tony Duong 04/21/2020: HW core output iirst sample after 3 cycles from iirst input sample!!
//
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
            blackbox.io.clk             := clock                    // Implicit module clock
            blackbox.io.reset           := ~(reset | iir_reset_re)  // IIR filter has an active low reset (signal name is misleading)
            blackbox.io.inData		:= Mux(datain_read_idx < 32.U, datain_read_data, 0.U)
                											        // Map the IIR input data only when pointing to
                											        // a valid memory location
            dataout_write_data          := blackbox.io.outData      // IIR output data

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
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


        } // lazy val module

    }  // abstract class IIR
//--------------------------------------------------------------------------------------
// END: IIR TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Black box wrapper for Verilog Module
//
// Note: Name must match Verilog module name, signal names
//   declared within much match the name, width, and direction of
//   the Verilog module.
//--------------------------------------------------------------------------------------
class IIR_filter() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk         = Clock(INPUT)
    val reset       = Bool(INPUT)

    
    // Inputs
    val inData      = Bits(INPUT,32)

      // Outputs
    val outData     = Bits(OUTPUT,32)

  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------

