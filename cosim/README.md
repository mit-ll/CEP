[//]: # (Copyright 2021 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

# README for CEP co-simulation environment

This SW/HW co-simulation evironment has been developed to support "chip-level" simulation of the CEP.  

Several environments are supported:

* Bus Functional Model Mode (BFM)
* Bare Metal Mode
* ISA Tests under Bare Metal (ported and re-used from riscv-tests).
* Linux Mode (tests run on xilinx VC707 development card).  Build and installation instructions can be found in: [../README.md](../README.md)
* Benchmarking on Linux (TBA)
* Cycle-accurate and translation-level accurate unit level simulations.
* JTAG support to inferace with Openocd tool (via bitbang adapter) for Open On-Chip debugger (version 3.3 or later)

## Benefits: ##

* Re-usability: tests or SW drivers that are written to run on the actual HW can be used here. This allows SW to be verified/checked for functionality even before the HW is built. And of course, debugging HW at the same time.

* SW and HW are de-coupled such that mistakes that are found in the test/driver (SW side) during development process can be quickly corrected and rerun without having to rebuild the HW. Sometimes, it might takes minutes (or even hours for big ASIC) just to rebuild the HW if test and DUT (Device Under Test) are tightly coupled

* Makefile is setup to support vendor independent designs such that same design can be used to target FPGA vendor as Xilinx or TSMC for ASIC, for example.

* To take advantage of multi-core machines, support up to 4K+ threads where all threads can run in parallel and target different HW sections. For example: one core can target the AES block while another core targets the FIR block, etc...

* Mailboxes are setup to support in between thread communication for synchronization, handshaking, etc...

* All are *makefile* driven with scripts to support auto-dependencies generation. Which means anything changes in either HW files or SW files: incremental builds will be called to save time. This is a must in order to support parallel batching.

* Provide 100% visibility to everything in the test bench via wave capturing. From the SW (test) side, any signal in HW can be observed and forced (for negative testing) or to be used to synchronize events (wait for something before sending packet, etc...)

* Same setup can be used for any design. Therefore, it takes only hour to set up new test bench for new design and let designer/verification engineers start writing tests right away.

* Most of the tests can even be re-used across multiple projects such as memory test, register tests, packet generator and checker. They are written as template where only simple read/write methods need to be overload.

* Open source codebase

## Pre-requisites ##

- Vivado and Modelsim (or xcelium) have been installed (tested versions are listed in [../README.md](../README.md))
- CEP hardware has be built as described in "Building the Hardware" in [../README.md](../README.md)

Assuming you already have the CEP-master (version 2.0 or later) sandbox pulled from git and went thru the vivado build successfully. In other words, all the design files (Verilog/VHDL files) are all created.

## Issues ##
   1. Release 2.7: There are currently 3 tests that fail on RHEL7 machines using new RISC-V toolchain (with gcc-7.x or gcc-5.x) and pass under Ubuntu (with gcc 5.4.0)
```
    bareMetalTests/cacheCoherence
    isaTests/rv64mi-p-access 
	  isaTests/rv64ud-p-ldst
```
   **NOTE**: All tests (including the above failed tests) are now passing with version 2.8 or later under Linux machine..

## Verify environment settings and tools: ##

For CEP, the path to tools required are as listed below (see **common.make**)

    VIVADO_PATH     ?= /opt/xilinx-2018.3/Vivado/2018.3
    SIMULATOR_PATH  ?= /opt/questa-2019.1/questasim/bin
    RISCV           ?= /opt/riscv

**NOTE**: If they are not matched your setup, dont need to edit the common.make file, use enviroment variables to override as such (assuming bash shell is used). 
```
export VIVADO_PATH=<Your_path_to_Vivado_tool>
export SIMULATOR_PATH=<Your_path_to_questa_tool>
```

As of release 3.2 or later, Cadence tool set (xcelium and vmanager) is also supported, all setup related to Cadence tool set can be found in `cadence.make`. See more details in the Cadence Supports section below:


## Directory structure: ##

First some brief inline descriptions about the directory structure of the cosim environment:

You should see something like this under *cosim* directory:

```
    Makefile                <-- Everything are driven by *make* (arranged hierarchically)
    common.make             <-- the guts of make (can be shared amongst other projects)
    cep_buildChips.make     <-- ONLY for CEP (which is called by common.make)

    bfmTests                <-- test suite to run in BFM mode
        ddr3Test            <-- DDR3 memory test from all 4 cores
            memPreload      <-- bare metal program preload to main memory test
            regression      <-- The famous *cepregression* test that run on VC707 in lab

    bareMetalTests          <-- test suite to run in bare metal mode
        regression          <-- the same *cepregression* run under bare metal mode
        ddr3                <-- same memory test to run on 4 cores
        printfTest          <-- verify printf implementation in bare metal via main memory
        cacheCoherence      <-- cache coherency test. Cache blocks bouncing around all cores.

    isaTests		            <-- Incorporate ISA test suite from rocket-chips. See "Building ISA tests for 
                                simulation section" for details
    
    bin                     <-- the rest of the directories below are there to
    drivers                     support cosim environment
    dvt
    include
    lib
    pli
    share
    simDiag
    src
    xil_lib                 <-- xilinx generated library files for questa (~1Gbytes) (see note below)
    cad_xil_lib             <-- xilinx generated library files for xcelium (~1Gbytes)
```


## Compiling the Xilinx Simulation Libraries ##

**NOTE**: xil_lib is the generated library packages created via vivado 's compile_simlib command.  Not all Vivado / Modelsim version combinations yield the desired result.  See the following notes:
- When Vivado 2019.1 and Questa 2019.1 is selected ,the compilation will stop and not generate the appropriate modelsim.ini file, which is required by the co-simulation environemt.
- When Vivado 2018.3 and Questa 2019.1 (or Quest 10.7c) is selected, the compilation will return with an error in the qdma_v3_0_0 library.  This library is not required for CEP simulation and thus the error can be safely ignored.
- When Vivado 2018.3 and Question 10.6c is selected, the compilation will complete without error.  However, it is recommended that Questa 2019.1 be used in order to take advantage of optimizations and bug fixes.  

```
  cd <CEP_ROOT>/cosim
  source <VIVADO_PATH>/settings64.sh                            <-- Setup your Vivado environment
  vivado -mode tcl                                              <-- open Vivado in TCL mode
```

At the Vivado's tcl console (bottom), type/cut/paste and execute the below command:
  
```
compile_simlib -simulator questa -simulator_exec_path {/opt/questa-2019.1/questasim/bin} -family all -language all -library all -dir {./xil_lib} -force -verbose

or/and if xcelium is also used...

compile_simlib -simulator xcelium -simulator_exec_path {$XCELIUM_INSTALL/tools/bin} -family all -language all -library all -dir {./cad_xil_lib} -force -verbose
```
 
Replace /opt/questa-2019.1/questasim/bin (or xcelium) and ./cosim/xil_lib above with appropriate paths of your setup

# Cadence Tool Supports: #

If you decide to also use Cadence's xcelium for simulation, modify `cadence.make` or override with environment variabbles to match the below variables:

```
export VMGR_VERSION	?= VMANAGERAGILE20.06.001
export XCELIUM_VERSION	?= XCELIUMAGILE20.09.001

export VMGR_PATH	?= /brewhouse/cad4/x86_64/Cadence/${VMGR_VERSION}
export XCELIUM_INSTALL  ?= /brewhouse/cad4/x86_64/Cadence/${XCELIUM_VERSION}
```

After that, just by adding a switch CADENCE=1 to the make command line, the simulation (and all related commands) will use xcelium as simulator.

For example, to run full regression with coverage enable via xcelium, just follow the below simple steps:

```
cd <...>/cosim		    <-- top of cosim directory
make CADENCE=1 COVERAGE=1   <- run full regression with coverage enable using xcelium (takes about 1 day)
make CADENCE=1 summary	    <- get the pass/fail results from full regression
make CADENCE=1 mergeAll	    <- merge all the coverage data and generate report in HTML format (all files under cosim/cad_coverage)
```

# How to run test(s): #

NOTE: type "make usage" for help.

* Run a test interactively: (use regTest as example)

```
cd .../cosim/bfmTests/regTest 
make
   or
make CADENCE=1 to use Cadence's xcelium
```

* Run full regression:

```	
cd .../cosim  
make  		<-- this will run all tests in both BFM and bare metal
```

or

```
cd .../cosim/bfmTests  
make		<-- this will run all tests under BFM mode only
```

or

```
cd .../cosim/bareMetalTests  
make		<-- this will run all tests under bare metal mode only
```

* How to check regression results:

```
cd .../cosim  
make summary
```
   it should give you something like this:

```
PASS : /home/aduong/CEP/CEP-master/cosim/bfmTests/ddr3Test (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bfmTests/memPreload (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bfmTests/regression (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bareMetalTests/cacheCoherence (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bareMetalTests/ddr3 (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bareMetalTests/printfTest (NEW SEED)
PASS : /home/aduong/CEP/CEP-master/cosim/bareMetalTests/regression (NEW SEED)
 ```
 
# How to add new test: #

   First pick which test category the new test should be under: bfmTests or bareMetalTests or <others_yet_created>

	cd <bfmTests_or_bareMetalTests_or_others>
	cp -rp ddr3Test <your_new_test>		<-- cut/paste any existing test as reference to your new test


	cd <your_new_test>
	ls  (should have the list of file shown below)

	c_dispatch.h		<-- make sure all the header files and Makefile are there and don't need to touch any of these
	c_module.h
	Makefile
	testHostory.txt		<-- history of PASS/FAIL and random seeds used for this particular test from day of creation.
	
	c_dispatch.cc		<-- top dispatch to control how many cores to run 
	c_module.cc		<-- the test wrapper to run under each core 
				Note: 
					in BFM mode, this is the test wrapper that calls your new test
					in Bare Metal mode, don't need to touch. See riscv_wrapper.cc
				    
	riscv_wrapper.cc	<-- This is the test wrapper that calls your new test in Bare Metal Mode only.
				Note: under BFM category, this file is not needed.
				

   Let dig in further to each of these files:


 ##  c_dispatch.cc: ##

   The below are the codes that you should pay attention to change how test should be run:                        

```C++
  <...>
  long unsigned int mask = 0xF;
```
   This is the mask to control how many cores should be allowed to run. One bit per core:
   
    bit[0] =1 = core#0 should be active
    bit[1] =1 = core#1 should be active, etc...

   The setting above indicates all 4 cores will be participating in this test
   Or you can pick randomly which core to run as such:

```C++
long unsigned int mask = 1 << (seed & 0x3);   // randomly select one of 4 core to run based on the seed
```

   **Note**: *seed* above will be calculated based on this formula:

   If the test fails last time (see testHistory.txt), it will use the same seed.
   The seed only be re-generated when the test passed in full regression. Running the test interactively won't change it. Since seed is used to generated different test pattern as well as sequence of events, they help increase verification coverage as regression are run often.
   
```C++    
<...>
int cycle2start=0;
int cycle2capture=-1; // til end
int wave_enable=1;
```

These 3 lines control when/how waveform should be captured. They are in unit of 100MHZ clock cycle (for CEP).  
The default setting: wave capture is enable (wave_enable=1) and it will start capture at beginning (cycle2start=0) and capture until the end of test (cycle2capture=-1). If cycle2capture=100, for example, it means it will capture for 100 clock cycles then stop.

   **NOTE**: running full regression, the wave capturing is automatically turned off to speed up (unless otherwise override)


### c_module.cc: ###

This file takes on 2 different formats, one for BFM mode and one for Bare Metal mode:

In BFM mode, these are codes you should pay attention to (cut/paste from bfmTests/regression/c_module.cc)

```C++
 //
 // ======================================
 // Test starts here
 // ======================================
 // MUST
 pio.RunClk(1000);
 //
 if (!errCnt) { errCnt = cepregression_test();  }
```

As the code indicated, one of the selected core will execute the test you_want_to_run that is **cepregression_test()**. 
This is the same code that we use to run on the actual VC707 eval board in the lab.

In BareMetal mode, c_module.cc can be used as it since it does not actually call any test to run. It is used to monitor the RISCV core progress for pass/fail status. The calling job is moved to riscv_wrapper.cc as described below


### riscv_wrapper.cc: ###

This file is the wrapper which is used to call the test you want the cores (all or any one selected) to run. And these are the lines you should look for:

```C++
void thread_entry(int cid, int nc) {
 <...>
 //
 // ===================================
 // Call your program here
 // ===================================  
 //
 int testId = 0x88;
 if (!errCnt) { errCnt = cepregression_test();  }
 <...>
```
Similar to c_module.cc, the same cepregression_test(), used under BFM mode, is called under bare metal. Once you type **make**, it will call **riscv64-unknown-elf-gcc** compiler instead of Linux's g++ to compile the codes and creates an image to preload to main memory (DDR3) to run. And note, **thread_entry** is the name of the wrapper instead of main(...). This is required to support multi cores.

Once, you modify either c_module.cc (under BFM) or riscv_wrapper.cc (under bare metal) to call your new test, just type **make** to run it..


## Bare Metal Setup ##

Let touch base about bare metal mode a little bit: the boot ROM in bare metal (see **.../cosim/drivers/bootbare/bootbare.c**) is based on hdl_cores/freedom/bootrom/sdboot/sd.c where everything is commented out to skip the SD card booting section. The processor just jump directly to main memory at 0x8000_0000 to start executing. At the beginning of the simulation run, this file (bootbare.hex) is then loaded into the boot ROM of the rocket chip overrwriting the default.        

During simulation, you will notice each of the core will print out its PC (Program Counter), DASM, and other useful information every cycle. To cut down the amount of output on the screen, only active cores allows to print and it will stop the print at end of test (even the core still running). That is the job of c_module.cc in bare metal mode (all happen under the hood)

Also, there are riscv_wrapper.dump (created from objdump to help track the PC), riscv_wrapper.elf (to preload to main memory) and riscv_wrapper.hex files are created by Make under each test directory.

**Note**: for version 3.3 or later, directory **.../cosim/drivers/bootbare** is removed and the official bootrom generation Makefile is adjusted to also produce the bootrom image for simulation.

## Building ISA tests for simulation ##

For version 2.7 or later, ISA (Instruction-Set-Architecture) tests are added to simulation to improve overall chip coverages.

**All ISA tests are re-used from https://github.com/riscv/riscv-tests.git repository**. 

**First**: Refer to **software/riscv-tests/README.md** for detailed explanations about how those ISA tests are set up and terminologies used through out this section. Or check out the link here: https://riscv.org/wp-content/uploads/2015/01/riscv-testing-frameworks-bootcamp-jan2015.pdf

**Issue with TVM='p' & 'pm'**: These 2 modes are setup to run in physical address only condition. Eventhough the riscv-tests/README.md mentions TVM='pm' mode is supported but the make infrastructure NOT really set up the build for it. In order to improve coverage, we need to be able to run tests on 4 cores. Therefore, we need to do a minor edit to change TVM='p' (virtual memory is disabled, only core 0 boots up) to 'pm' (virtual memory is disabled, all cores boot up)

Edit the file **<CEP_ROOT>/software/riscv-tests/env/p/riscv_test.h** and look for the lines below:

```
#define RISCV_MULTICORE_DISABLE                                         \
  csrr a0, mhartid;                                                     \
  1: bnez a0, 1b
  
```

This define is used to basically do a check and only allow core0 run the test. Other cores will fail if they run. We need to disable this check by removing the **bnez** line  as such:

```
#define RISCV_MULTICORE_DISABLE                                         \
  csrr a0, mhartid;                                                     \


// Remove/comment out this line
//  1: bnez a0, 1b

```
Save the file.


**Issue with TVM='v'**: Similar problem here, the tests are setup for core0 only, others cores's jobs just do interference to cause cache misses. For simulation, we need to allow any cores to run the same tests. Also, we dont need to run all the rv64*-v-* tests, since, they are the same as when TVM='p' but one run in virtual address mode and the other in strickly physical address mode. The goals for TVM='v' tests are to improve the coverages for page tables (page faults), cache hits/misses.

Edit the file **<CEP_ROOT>/software/riscv-tests/env/v/vm.c** and look for the lines below:

```
  if (read_csr(mhartid) > 0)
    coherence_torture();
```

Remove/comment out that 2 lines and save the file.

**Issue with TVM='pt'**: Not supported for now.

Next edit is **OPTIONAL** (only do it if you know what you are doing :-): this edit is to add switches to the build process to including source information in the object dump file for PC's tracing. It is helpful in debugging failed test(s). Be aware, the ELF/dump files will be much bigger!! Only do this if you intend to include the source codes in the object dump file. In addition, RISCV-core will behave differently due to GDB is not supported in simulation when it taking a page fault (dud!). So only do this **edit**  to obtain the object dump file for tracing.

Edit file **<CEP_ROOT>/software/riscv-tests/isa/Makefile**, look for the lines as such:

```
RISCV_GCC_OPTS ?= -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump  --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data
```

And change them to match the below:

```
RISCV_GCC_OPTS ?= -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -g
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump  -S -C -d -l -x --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data
```

Save the Makefile.

And now you are ready to do the build as follows

```sh
  cd <CEP_ROOT>/software/riscv-tests
  autoconf
  ./configure	
  make isa      <-- only need to make ISA, without argument benchmark tests will also be included (benchmarks have NOT been ported)
```

The make command above will compile **mostly** assembly tests in the directory **isa** under it. These are the ones will be re-used to run in our simulation. **NOTE**: only RV64*-p/v-* tests are used since the cores are 64-bit cores. 

**Note2**: As mentioned earlier, the purpose of this is to allow all cores to be able to run simulation to improve coverages. However, some tests are explicitely written to run on core0 only, some tests MUST be run as single thread (only 1 core active at a time), some tests will allow all cores to run in parallel. With that in mind, Makefile under **cosim/isaTests** is setup to take that into account by seperating them into 3 categories and simulate them accordingly.

Next step is to port and prepare those ISA tests above for simulation.

```
  cd <CEP_ROOT>/cosim/isaTests
  make isaTests	 <-- clean old (if any) and prepare all new ISA tests for simulation (take a while.. Be patient)
```

**Finally**: There are a lots of magic happen under-the-hood for this type of tests since they are originally written such that their output are used to compare against golden output (from Spike/Verilator). We dont do that here. Therefore, in simulation, hooks are put in place to check the **core's PC** (Program Counter) to see if they enter **pass** or **fail** section. In order to do this, during test preparation (**make isaTests**), a perl script is called to look into the deassembled code (<test>.dump file) for given test to find where the **pass/fail** locations are, respectively. These locations are needed by the testbench during runtime to determine if the given test is passing or failing.

And we are now done for this ISA porting.

## About JTAG/Debug port testing and Openocd ##

To test JTAG port in simulation, openocd tool is needed. The Makefile will check for the present of such tool before any jtag related test is allowed to run.

This JTAG port is connected to the CEP's DTM (DebugTransportModule). It is used to facilitate debugging via GDB. Openocd acts as transport layer between the internal DTM and GDB.

A short description of what openocd is about:

```
OpenOCD provides on-chip programming and debugging support with a
layered architecture of JTAG interface and TAP support including:

- (X)SVF playback to facilitate automated boundary scan and FPGA/CPLD
  programming;
- debug target support (e.g. ARM, MIPS): single-stepping,
  breakpoints/watchpoints, gprof profiling, etc;
- flash chip drivers (e.g. CFI, NAND, internal flash);
- embedded TCL interpreter for easy scripting.
```

To support simulation, DPI & remote-bitbang must be enable when building openocd tool.

Download openocd via this link:  **https://github.com/riscv/riscv-openocd**

And follow the README file in there to build the tool or for more details how to use the tool. Instructions are cut/paste below for quick reference:

```
To build OpenOCD, use the following sequence of commands:

  ./bootstrap (when building from the git repository)
  ./configure -enable-remote-bitbang --enable-jtag_dpi [options]
  make
  sudo make install
```

To see how this works, run this test: **<...>/cosim/isaTests/dtmTest**

## How to add your new test for regression ##

    Open ../Makefile (one level up where your test directory is located). 
    Look for BARE_TEST_LIST or BFM_TEST_LIST and add the name of your test there.
    Done.


## How to write a new test ##

At this point, you should have some ideas about how the environment is setup and know how to run and add more tests. Next is how to write your test so it can be simulated and use in the lab to target real HW: **Just as how to write test for the real thing**.  Some keys to keep in mind:

* To read/write to HW, in general, use 2 macros provided: (see portable_io.h)

```C++
#define DUT_WRITE32_64(a,d) <...>  
#define DUT_READ32_64(a,d)  <...>  

    a = physical address you want to write to or read from.  
    d = 64 bits of data to write or read 
```

Or you can use: (if CEP is the DUT)

```C++
uint64_t cep_read(int device, uint32_t pAddress) <...>
void cep_write(int device, uint32_t pAddress, uint64_t pData)  <...>
 ```
 
 * Make your test procedure as portable/modular and independent as possible. 
 
 * To use under bare metal mode, don't use any fancy system calls since there is no OS to help you.
 
 * printf can be overloaded and printed to screen but it is going to be very slow in simulation. This can be turn on/off per test

**NOTES** :

Since all C/C++ codes are visible/built for all modes (BFM, BARE Metal and Linux), there are several **defines**, test writer should be aware of:

#define **BARE_MODE** :
   In BARE Metal simulation, there are no libraries/system call supports, therefore, any codes that uses such need to be ifdef out via this define.

#define **LINUX_MODE** :
   This define is not exist in simulation and it is defined during Linux build
   
#define **SIM_ENV_ONLY** :
    This define is used to map out any codes that is used for simulation only. For example:
       ReadDvtFlag(...) and WriteDvtFlag(...) APIs are only applicable in simulation to probe certain signals or synchronize to some events..
       
## Cheat Sheet ##

* Since it is all makefile driven, there are some switches that can be added to the make command line to alter the default settings. Some of those switches are:

```make

make usage      <-- print make help for list of options and targets

make USE_GDB=1      <-- run simulation under GDB debugger. This should only be run under interactive session. 
                Very helpful tool to debug when program seg fault either in HW or SW sides.

make NOWAVE=1       <-- run simulation without wave capturing. Default is ON during interactive session

make PROFILE=1      <-- run simulation with profile capturing. This helps identify which module consumes most CPU cycles

make COVERAGE=1	    	<-- run simulation with coverage turned on. Coverage data are saved in ../coverage directory where they can 
                be merged together for analysis (via make merge)
make CADENCE=1      	  <-- run simulation targetting Cadence XCellium on RHEL7

make mergeAll	    	  <-- merge all coverage data and report in HTLM format for modelsim

make CADENCE=1 mergeAll	  <-- merge all coverage data and report in HTLM format for xcelium
```

* By default, under each test directory, one file will be created **if and only if** it is not yet there: **vsim.do**. It is used when **vsim** command is called to control the wave capturing.. If there is a need to override, users are free to modify and change it to anyway to fit the needs. Makefile will not overwrite it as long as it is there. 

* Under bare metal mode, some of main memory locations are used as mailbox to help RISCV core tracking and printing. See .**../cosim/dvt/cep_adrMap.incl** file. **NOTE**: there is also a file under .../cosim/include/cep_adrMap.h This file is auto-generated from the cep_adrMap.incl mentioned. Therefore, any modification should be made to the cep_adrMap.incl file.

* Below is the short-cut to quickly build cep_diag application for Linux:

```
cd <...>/cosim/drivers/linux
make buildMe
```

