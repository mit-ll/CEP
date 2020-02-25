[//]: # (Copyright 2020 Massachusetts Institute of Technology)

# README for CEP co-simulation environment

This SW/HW co-simulation evironment has been developed to support "chip-level" simulation of the CEP.  

Three environments are supported:

* Bus Functional Model (BFM)
* Bare Metal
* CEP diagnostics running on Linux.  Build and installation instructions can be found in: [../README.md](../README.md)


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

- Vivado and Modelsim have been installed (tested versions are listed in [../README.md](../README.md))
- CEP hardware has be built as described in "Building the Hardware" in [../README.md](../README.md)

Assuming you already have the CEP-master (version 2.0 or later) sandbox pulled from git and went thru the vivado build successfully. In other words, all the design files (Verilog/VHDL files) are all created.


## Verify environment settings and tools: ##

For CEP, the path to tools required are as listed below (see **common.make**)

    VIVADO_PATH     ?= /opt/xilinx-2018.3/Vivado/2018.3
    SIMULATOR_PATH  ?= /opt/questa-2019.1/questasim/bin
    RISCV_DIR       ?= /opt/riscv

**NOTE**: If they are not matched your setup, dont need to edit the common.make file, use enviroment variables to ovlerride as such (assuming bash shell is used). 
```
export VIVADO_PATH=<Your_path_to_Vivado_tool>
export SIMULATOR_PATH=<Your_path_to_questa_tool>
```


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
        
    bin                     <-- the rest of the directories below are there to
    drivers                     support cosim environment
    dvt
    include
    lib
    pli
    share
    simDiag
    src
    xil_lib                 <-- xilinx generated library files (~1Gbytes) (see note below)
```


## Compiling the Xilinx Simulation Libraries ##

**NOTE**: xil_lib is the generated library packages created via vivado 's compile_simlib command.  Not all Vivado / Modelsim version combinations yield the desired result.  See the following notes:
- When Vivado 2018.3 and Questa 2019.1 (or Quest 10.7c) is selected, the compilation will return with an error in the qdma_v3_0_0 library.  This library is not required for CEP simulation and thus the error can be safely ignored.
- When Vivado 2018.3 and Question 10.6c is selected, the compilation will complete without error.  However, it is recommended that Questa 2019.1 be used in order to take advantage of optimizations and bug fixes.  

```
  cd <CEP_ROOT>/cosim
  source <VIVADO_PATH>/settings64.sh                            <-- Setup your Vivado environment
  vivado -mode tcl                                              <-- open Vivado in TCL mode
```

At the Vivado's tcl console (bottom), type/cut/paste and execute the below command:
  
`compile_simlib -simulator questa -simulator_exec_path {/opt/questa-2019.1/questasim/bin} -family all -language all -library all -dir {./xil_lib} -force -verbose`
 
Replace /opt/questa-2019.1/questasim/bin and ./cosim/xil_lib above with appropriate paths of your setup

```

  
## How to run test(s): ##

NOTE: type "make usage" for help.

* Run a test interactively: (use regTest as example)

```
cd <CEP_ROOT>/cosim/bfmTests/regTest 
make
```

* Run full regression:

```
<CEP_ROOT>/cosim
make        <-- this will run all tests in both BFM and bare metal
```

or

```
cd <CEP_ROOT>/cosim/bfmTests  
make        <-- this will run all tests under BFM mode only
```

or

```
cd <CEP_ROOT>/cosim/bareMetalTests  
make        <-- this will run all tests under bare metal mode only
```

* How to check regression results (assuming tests have been run):

```
cd <CEP_ROOT>/cosim  
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

 
## How to add new test: ##

   First pick which test category the new test should be under: bfmTests or bareMetalTests or <others_yet_created>

    cd <bfmTests_or_bareMetalTests_or_others>
    cp -rp ddr3Test <your_new_test>     <-- cut/paste any existing test as reference to your new test


    cd <your_new_test>
    ls  (should have the list of file shown below)

    c_dispatch.h        <-- make sure all the header files and Makefile are there and don't need to touch any of these
    c_module.h
    Makefile
    testHostory.txt     <-- history of PASS/FAIL and random seeds used for this particular test from day of creation.
    
    c_dispatch.cc       <-- top dispatch to control how many cores to run 
    c_module.cc     <-- the test wrapper to run under each core 
                Note: 
                    in BFM mode, this is the test wrapper that calls your new test
                    in Bare Metal mode, don't need to touch. See riscv_wrapper.cc
                    
    riscv_wrapper.cc    <-- This is the test wrapper that calls your new test in Bare Metal Mode only.
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


## Cheat Sheet ##

* Since it is all makefile driven, there are some switches that can be added to the make command line to alter the default settings. Some of those switches are:

```make

make usage      <-- print make help for list of options and targets

make USE_GDB=1      <-- run simulation under GDB debugger. This should only be run under interactive session. 
                Very helpful tool to debug when program seg fault either in HW or SW sides.

make NOWAVE=1       <-- run simulation without wave capturing. Default is ON during interactive session

make PROFILE=1      <-- run simulation with profile capturing. This helps identify which module consumes most CPU cycles

make COVERAGE=1     <-- run simulation with coverage turned on. Coverage data are saved in ../coverage directory where they can 
                be merged together for analysis (via make merge)
```

* By default, under each test directory, one file will be created **if and only if** it is not yet there: **vsim.do**. It is used when **vsim** command is called to control the wave capturing.. If there is a need to override, users are free to modify and change it to anyway to fit the needs. Makefile will not overwrite it as long as it is there. 

* Under bare metal mode, some of main memory locations are used as mailbox to help RISCV core tracking and printing. See .**../cosim/dvt/cep_adrMap.incl** file. **NOTE**: there is also a file under .../cosim/include/cep_adrMap.h This file is auto-generated from the cep_adrMap.incl mentioned. Therefore, any modification should be made to the cep_adrMap.incl file.

# Food for thought #

 * At the time of this writing, running regression is done one test at a time on the same machine, serially. It takes about 1 hour+ to run all tests. As more tests are added or moving the platform to support ASIC, there will be hundreds of tests added to verify full coverage of the design before any fabrication attempted. Running serially is not do-able as turn around time is key during physical design and verification process for ASIC. This requires moving to support multiple machines (as on a cloud or grid/networks) where tests can be batched to run concurrently...The Makefile structure already takes that in mind and supports it. The requirements are the grid setup where all the machines on the networks need to be configured identically w.r.t where the tools are, all disks are mounted and visible... Not sure MIT LL support such a thing due to security??
 
 * The environment is also capable of supporting distributed simulation where multiple large blocks (multi corer per board, big large ASIC, etc...) can be spawn off to run on different machines (or physical cores) to speed up simulation. However, this requires more simulation license usage and some manual system splitting during test bench construction.
 
 * The environment also can be expanded to support socket communication instead of shared memory such that the DUT can be run off an emulated system (Amazon FPGA cloud??) to get more performance. This should be able to support booting Linux OS for higher level verification.
 
   




