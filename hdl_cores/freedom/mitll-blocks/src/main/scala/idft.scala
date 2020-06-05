//
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File         : idft.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : TileLink interface to the verilog idft core
// Note         : The "control" logic described in the idft abstract class
//                is intended to mimic the verilog in the idft_top_wb.v
//                module.
//

package mitllBlocks.idft

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
case object PeripheryIDFTKey extends Field[Seq[IDFTParams]]

trait HasPeripheryIDFT { this: BaseSubsystem =>
  val IDFTNodes = p(PeripheryIDFTKey).map { ps =>
    IDFT.attach(IDFTAttachParams(ps, pbus))   // pbus = Periphery Bus
  }
}

case class IDFTParams(address: BigInt)

case class IDFTAttachParams(
  idftparams         : IDFTParams,
  controlBus        : TLBusWrapper)
  (implicit val p   : Parameters)

class TLIDFT(busWidthBytes: Int, params: IDFTParams)(implicit p: Parameters)
  extends IDFT(busWidthBytes, params) with HasTLControlRegMap

object IDFT {

  def attach(params: IDFTAttachParams): TLIDFT = {
    implicit val p = params.p
    val idft = LazyModule(new TLIDFT(params.controlBus.beatBytes, params.idftparams))

    // Connect our module to the specified bus (per the controlBus parameter)
    params.controlBus.coupleTo(s"slave_named_idft") {
      idft.controlXing(NoCrossing) := TLFragmenter(params.controlBus.beatBytes, params.controlBus.blockBytes) := _
    }

    InModuleBody { idft.module.clock := params.controlBus.module.clock }
    InModuleBody { idft.module.reset := params.controlBus.module.reset }

    idft
  }

}
//--------------------------------------------------------------------------------------
// END: Classes, Objects, and Traits to support connecting to TileLink
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: idft TileLink Module
//--------------------------------------------------------------------------------------
abstract class IDFT(busWidthBytes: Int, val c: IDFTParams)(implicit p: Parameters)
    extends RegisterRouter (
      RegisterRouterParams(
        name = "idft",
        compat = Seq("mitll,idft"), 
        base = c.address,
        size = 0x10000,    // Size should be an even power of two, otherwise the compilation causes an undefined exception
        beatBytes = busWidthBytes))
    {

        ResourceBinding {Resource(ResourceAnchors.aliases, "idft").bind(ResourceAlias(device.label))}

        lazy val module = new LazyModuleImp(this) {

            // Macro definition for creating rising edge detectors
            def rising_edge(x: Bool)    = x && !RegNext(x)

            // Instantitate the idft blackbox
            val blackbox                = Module(new idft_top)

            // Instantiate the input and output data memories (32 words of input and output data)
            val datain_mem              = Mem(32, UInt(64.W))     // for holding the input data
            val dataout_mem             = Mem(32, UInt(64.W))     // for holding the output data

            // Define registers / wires for interfacing to the idft blackbox
            val start                   = RegInit(false.B)      // Start bit controlled via register mappings
            val datain_we               = RegInit(false.B)      // Controlled via register mappings
            val datain_write_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
            val datain_write_data       = RegInit(0.U(64.W))    // Controlled via register mappings
            val datain_read_idx         = RegInit(0.U(6.W))     // Generated read address from start bit
            val datain_read_data        = Wire(UInt(64.W))      // Data read from intermediate buffer into idft_top.v 

            val dataout_write_idx       = RegInit(0.U(6.W))     // Data output address generated from next_out
            val dataout_write_data      = RegInit(0.U(64.W))    // Data output
            val dataout_read_idx        = RegInit(0.U(6.W))     // Controlled via register mappings
            val dataout_read_data       = Wire(UInt(64.W))      // Controlled via register mappings
            val dataout_valid           = RegInit(false.B)      // Data valid output bit drive by idft_top.v

            val next_out                = RegInit(false.B)

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

            // Generate the write index for the output data memory (and write)
            //hen (rising_edge(next_out)) {
            when (next_out) {
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
            blackbox.io.reset           := reset                    // idft has an active low reset (signal name is misleading)

            blackbox.io.X0          := Mux(datain_read_idx < 32.U, datain_read_data(63,48), 0.U)
            blackbox.io.X1          := Mux(datain_read_idx < 32.U, datain_read_data(47,32), 0.U)
            blackbox.io.X2          := Mux(datain_read_idx < 32.U, datain_read_data(31,16), 0.U)	    
            blackbox.io.X3          := Mux(datain_read_idx < 32.U, datain_read_data(15,0), 0.U)

            blackbox.io.next        := rising_edge(start) 
                											        // Map the idft input data only when pointing to
                											        // a valid memory location
            // Map the blackbox outputs
            dataout_write_data      := Cat(blackbox.io.Y0,blackbox.io.Y1,blackbox.io.Y2,blackbox.io.Y3)      // idft output data
            next_out                := blackbox.io.next_out

            // Define the register map
            // Registers with .r suffix to RegField are Read Only (otherwise, Chisel will assume they are R/W)
            regmap (
                IDFTAddresses.idft_ctrlstatus_addr    -> RegFieldGroup("idft_ctrlstatus",Some(""), Seq(
                    RegField    (1, start               ),      // Start passing data to the idft blackbox
                    RegField    (1, datain_we           ),      // Write enable for the datain memory
                    RegField.r  (1, dataout_valid       ))),    // Data Out Valid
                IDFTAddresses.idft_datain_addr_addr   -> Seq(RegField     (5,  datain_write_idx)),
                IDFTAddresses.idft_datain_data_addr   -> Seq(RegField     (64, datain_write_data)),
                IDFTAddresses.idft_dataout_addr_addr  -> Seq(RegField     (5,  dataout_read_idx)),
                IDFTAddresses.idft_dataout_data_addr  -> Seq(RegField.r   (64, dataout_read_data))
            )  // regmap


        } // lazy val module

    }  // abstract class idft
//--------------------------------------------------------------------------------------
// END: idft TileLink Module
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// BEGIN: Black box wrapper for Verilog Module
//
// Note: Name must match Verilog module name, signal names
//   declared within much match the name, width, and direction of
//   the Verilog module.
//--------------------------------------------------------------------------------------
class idft_top() extends BlackBox {

  val io = IO(new Bundle {
    // Clock and Reset
    val clk     = Clock(INPUT)
    val reset   = Bool(INPUT)

    val next    = Bool(INPUT)
    val next_out= Bool(OUTPUT)

    // Inputs
    val X0      = Bits(INPUT,16)
    val X1      = Bits(INPUT,16)
    val X2      = Bits(INPUT,16)
    val X3      = Bits(INPUT,16)

      // Outputs
    val Y0      = Bits(OUTPUT,16)
    val Y1      = Bits(OUTPUT,16)
    val Y2      = Bits(OUTPUT,16)
    val Y3      = Bits(OUTPUT,16)

  })

}
//--------------------------------------------------------------------------------------
// END: Black box wrapper for Verilog Module
//--------------------------------------------------------------------------------------
