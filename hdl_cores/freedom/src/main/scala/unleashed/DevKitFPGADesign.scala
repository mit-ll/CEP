/// See LICENSE for license details.

package sifive.freedom.unleashed

import Chisel._
import chisel3.experimental.{withClockAndReset}

import freechips.rocketchip.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.interrupts._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.system._
import freechips.rocketchip.util.{ElaborationArtefacts,ResetCatchAndSync}

import sifive.blocks.devices.msi._
import sifive.blocks.devices.chiplink._
import sifive.blocks.devices.spi._
import sifive.blocks.devices.uart._
import sifive.blocks.devices.gpio._
import sifive.blocks.devices.pinctrl.{BasePin}

import sifive.fpgashells.shell._
import sifive.fpgashells.clocks._

import mitllBlocks.cep_addresses._
import mitllBlocks.cep_registers._

import mitllBlocks.aes._
import mitllBlocks.rsa._
import mitllBlocks.sha256._
import mitllBlocks.fir._
import mitllBlocks.iir._
import mitllBlocks.idft._
import mitllBlocks.des3._
import mitllBlocks.md5._
import mitllBlocks.gps._
import mitllBlocks.dft._
import mitllBlocks.srot._
import mitllBlocks.scratchpad._

object PinGen {
  def apply(): BasePin = {
    new BasePin()
  }
}

class DevKitWrapper()(implicit p: Parameters) extends LazyModule
{
  val sysClock  = p(ClockInputOverlayKey).headOption.map(_.place(ClockInputDesignInput()).overlayOutput.node)
  val corePLL   = p(PLLFactoryKey)()
  val coreGroup = ClockGroup()
  val wrangler  = LazyModule(new ResetWrangler)
  val coreClock = ClockSinkNode(freqMHz = p(DevKitFPGAFrequencyKey))
  coreClock := wrangler.node := coreGroup := corePLL := sysClock.get

  // removing the debug trait is invasive, so we hook it up externally for now
  val jt = p(JTAGDebugOverlayKey).headOption.map(_.place(JTAGDebugDesignInput()).overlayOutput)

  val topMod = LazyModule(new DevKitFPGADesign(wrangler.node, corePLL)(p))

  override lazy val module = new LazyRawModuleImp(this) {
    val (core, _) = coreClock.in(0)
    childClock := core.clock

    val djtag = topMod.module.debug.get.systemjtag.get
    djtag.jtag.TCK := jt.get.jtag.TCK
    djtag.jtag.TMS := jt.get.jtag.TMS
    djtag.jtag.TDI := jt.get.jtag.TDI
    jt.get.jtag.TDO.data    := djtag.jtag.TDO.data

    djtag.mfr_id := p(JtagDTMKey).idcodeManufId.U(11.W)
    djtag.reset  := core.reset

    childReset := core.reset | topMod.module.debug.get.ndreset
  }
}

case object DevKitFPGAFrequencyKey extends Field[Double](100.0)

class DevKitFPGADesign(wranglerNode: ClockAdapterNode, corePLL: PLLNode)(implicit p: Parameters) extends RocketSubsystem
  with HasPeripheryDebug
  with HasHierarchicalBusTopology
  with HasPeripheryCEPRegisters
  with HasPeripheryAES
  with HasPeripheryRSA
  with HasPeripherySHA256
  with HasPeripheryFIR
  with HasPeripheryIIR
  with HasPeripheryIDFT
  with HasPeripheryDFT
  with HasPeripheryDES3
  with HasPeripheryGPS
  with HasPeripheryMD5
  with HasScratchpad
  with HasSROT
{
  val tlclock = new FixedClockResource("tlclk", p(DevKitFPGAFrequencyKey))

  // hook up UARTs, based on configuration and available overlays
  val divinit = (p(PeripheryBusKey).dtsFrequency.get / 115200).toInt
  val uartParams = p(PeripheryUARTKey)
  val uartOverlays = p(UARTOverlayKey)
  val uartParamsWithOverlays = uartParams zip uartOverlays
  uartParamsWithOverlays.foreach { case (uparam, uoverlay) => {
    val u = uoverlay.place(UARTDesignInput(uparam, divinit, pbus, ibus.fromAsync)).overlayOutput
    tlclock.bind(u.uart.device)
  } }

  (p(PeripherySPIKey) zip p(SDIOOverlayKey)).foreach { case (sparam, soverlay) => {
    val s = soverlay.place(SDIODesignInput(sparam, pbus, ibus.fromAsync)).overlayOutput
    tlclock.bind(s.spi.device)

    // Assuming MMC slot attached to SPIs. See TODO above.
    val mmc = new MMCDevice(s.spi.device)
    ResourceBinding {
      Resource(mmc, "reg").bind(ResourceAddress(0))
    }
  } }

  // TODO: currently, only hook up one memory channel
  val fourgbdimm = p(ExtMem).get.master.size == 0x100000000L
  val ddr = p(DDROverlayKey).headOption.map(_.place(DDRDesignInput(p(ExtMem).get.master.base, wranglerNode, corePLL, fourgbdimm)))
  ddr.foreach {_.overlayOutput.ddr := mbus.toDRAMController(Some("xilinxmig"))()}
  val mparams = p(ExtMem).get.master

  // Work-around for a kernel bug (command-line ignored if /chosen missing)
  val chosen = new DeviceSnippet {
    def describe() = Description("chosen", Map())
  }

  // hook the first PCIe the board has
  val pcies = p(PCIeOverlayKey).headOption.map(_.place(PCIeDesignInput(wranglerNode, corePLL = corePLL)).overlayOutput)
  pcies.zipWithIndex.map { case(overlayOut, i) =>  
    val pciename = Some(s"pcie_$i")
    sbus.fromMaster(pciename) { overlayOut.pcieNode }
    sbus.toFixedWidthSlave(pciename) { overlayOut.pcieNode }
    ibus.fromSync := overlayOut.intNode

  }

  // LEDs / GPIOs
  val gpioParams = p(PeripheryGPIOKey)
  val gpios = gpioParams.map { case(params) =>
    val g = GPIO.attach(GPIOAttachParams(gpio = params, pbus, ibus.fromAsync))
    g.ioNode.makeSink
  }

  
  val ledsOut = p(LEDOverlayKey).map(_.place(LEDDesignInput()).overlayOutput.led) 

  override lazy val module = new U500VC707DevKitSystemModule(this)
}

class U500VC707DevKitSystemModule[+L <: DevKitFPGADesign](_outer: L)
  extends RocketSubsystemModuleImp(_outer)
    with HasRTCModuleImp
    with HasPeripheryDebugModuleImp
{
  // Reset vector is set to the location of the mask rom
  val maskROMParams = p(PeripheryMaskROMKey)
  global_reset_vector := maskROMParams(0).address.U

  // hook up GPIOs to LEDs
  val gpioParams = _outer.gpioParams
  val gpio_pins = Wire(new GPIOPins(() => PinGen(), gpioParams(0)))

  GPIOPinsFromPort(gpio_pins, _outer.gpios(0).bundle)

  gpio_pins.pins.foreach { _.i.ival := Bool(false) }
  val gpio_cat = Cat(Seq.tabulate(gpio_pins.pins.length) { i => gpio_pins.pins(i).o.oval })
  
  // make the LED to GPIO connections
  for ( ledindex <- 0 to _outer.ledsOut.length-1 )  {
     _outer.ledsOut(ledindex) := gpio_cat(ledindex)
  }
}

// Allow frequency of the design to be controlled by the Makefile
class WithDevKitFrequency(MHz: Double) extends Config((site, here, up) => {
  case DevKitFPGAFrequencyKey => MHz
})

class WithDevKit25MHz extends WithDevKitFrequency(25)
class WithDevKit50MHz extends WithDevKitFrequency(50)
class WithDevKit100MHz extends WithDevKitFrequency(100)
class WithDevKit125MHz extends WithDevKitFrequency(125)
class WithDevKit150MHz extends WithDevKitFrequency(150)
class WithDevKit200MHz extends WithDevKitFrequency(200)

class DevKitU500FPGADesign extends Config(
  new U500DevKitConfig().alter((site, here, up) => {
    case DesignKey => { (p:Parameters) => new DevKitWrapper()(p) }
  }))
