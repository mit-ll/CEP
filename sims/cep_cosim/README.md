[//]: # (Copyright 2022 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

# Common Evaluation Platform - Co-Simulation Environment

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

<p align="center">
   <img src="../../cep_docs/cep_architecture.jpg" width="1114" height="450">
</p>

The CEP Co-Simulation environment supports "chip-level" simulation of the CEP.

The following provides highlights of the cosim directory structure.

```
<CEP_ROOT/sims/cep_cosim>
|- bin - Supporting scripts for cosim
|- bootrom - Source and support files for the CEP Bootrom (which replaces the default Chipyard Bootrom)
|- drivers
|  |- bare - Baremetal support files
|  |- cep_tests - Files (Classes) supporting testing of the CEP cores
|  |- diag - Supporting files used in defining some test cases
|  |- vectors - Recorded tilelink bus vectors for the CEP cores that are used with the bareMetal macroMix tests 
|  |            and unit-level simulations (not currently supported)
|  |- virtual - Virtual Mode supporting files (currently not supported)
|- dvt - Verilog and supporting files used by the cosim testbench
|- generated-src - Output of the Chipyard build
|- lib - CEP cosim build directory
|- pli - Files related to the PLI interface used by the cosim testbench
|- share - Files related to the IPC mailboxes used by the cosim testbench
|- simDiag - Additional supporting files
|- src - Logging and thread supporting files
|- testSuites
|  |- bfmTests - Tests that run in Bus Functional Model Mode
|  |- bareMetalTests - Tests that run in Bare Meta Mode
|  |- isaTests - Once generated, contains the modified RISC-V ISA tests for use in the cosim
```

These instructions assume you are running an environment as described in the pre-requisites section [here](../../README.md).

Prior to executing any simulation, the appropriate Chipyard *SUB_PROJECT* must be built.  The `cep_cosim` *SUB_PROJECT* It is selected by default when you run the following commands:

```
cd <CEP_ROOT>/sims/cep_cosim
make -f Makefile.chipyard
```

This command will generate and copy all of the necessary SystemVerilog/Verilog into the `<CEP_ROOT>/sims/cep_cosim/generated-src` directory.

## Building ISA tests for simulation ## 

The cosim supports bare metal execution of the RISC-V ISA tests which are included as submodule.  Additional information about these tests can be found [here](https://github.com/riscv-software-src/riscv-tests/tree/1ce128fa78c24bb0ed399c647e7139322b5353a7).

In the event that submodule initialization did not include the riscv-tests, one can manually initialize the submodule:
```
cd <CEP_ROOT>
git submodule update --init --recursive ./toolchains/riscv-tools/riscv-tests
```

There are a few know issues with running the ISA tests on the CEP.  Some manual changes are required before incorporating these tests into the cosim.

**Issue with TVM='p' & 'pm'**: These 2 modes are setup to run in physical address only condition. Even though the riscv-tests [README.md](https://github.com/riscv-software-src/riscv-tests/tree/1ce128fa78c24bb0ed399c647e7139322b5353a7) mentions TVM='pm' mode is supported but the make infrastructure NOT really set up the build for it. In order to improve coverage, we need to be able to run tests on 4 cores. Therefore, we need to do a minor edit to change TVM='p' (virtual memory is disabled, only core 0 boots up) to 'pm' (virtual memory is disabled, all cores boot up)

Edit the file `<CEP_ROOT>/toolchains/riscv-tools/riscv-tests/env/p/riscv_test.h` and look for the lines below:

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

Edit the file `<CEP_ROOT>/toolchains/riscv-tools/riscv-tests/env/v/vm.c` and look for the lines below:

```
  if (read_csr(mhartid) > 0)
    coherence_torture();
```

Remove/comment out these 2 lines and save the file.

**Issue with TVM='pt'**: Not supported for now.

Next edit is **OPTIONAL** (only do it if you know what you are doing :-): this edit is to add switches to the build process to including source information in the object dump file for PC's tracing. It is helpful in debugging failed test(s). Be aware, the ELF/dump files will be much bigger!! Only do this if you intend to include the source codes in the object dump file. In addition, RISCV-core will behave differently due to GDB is not supported in simulation when it taking a page fault (dud!). So only do this **edit**  to obtain the object dump file for tracing.

Edit file `<CEP_ROOT>/toolchains/riscv-tools/riscv-tests/isa/Makefile`, look for the lines as such:

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

```
  cd <CEP_ROOT>/toolchains/riscv-tools/riscv-tests
  autoconf
  ./configure 
  make isa      <-- only need to make ISA, without argument benchmark tests will also be included (benchmarks have NOT been ported)
```

The make command above will compile **mostly** assembly tests in the directory **isa** under it. These are the ones will be re-used to run in our simulation. **NOTE**: only RV64*-p/v-* tests are used since the cores are 64-bit cores. 

**Note2**: As mentioned earlier, the purpose of this is to allow all cores to be able to run simulation to improve coverages. However, some tests are explicitely written to run on core0 only, some tests MUST be run as single thread (only 1 core active at a time), some tests will allow all cores to run in parallel. With that in mind, Makefile under **<CEP_ROOT>/sims/cep_cosim/testSuites/isaTests** is setup to take that into account by seperating them into 3 categories and simulate them accordingly.

Next step is to port and prepare those ISA tests above for simulation.

```
  cd <CEP_ROOT>/sims/cep_cosim/testSuites/isaTests
  make createISATests  <-- clean old (if any) and prepare all new ISA tests for simulation (take a while.. Be patient)
```

**Finally**: There are a lots of magic happen under-the-hood for this type of tests since they are originally written such that their output are used to compare against golden output (from Spike/Verilator). We don't do that here. Therefore, in simulation, hooks are put in place to check the **core's PC** (Program Counter) to see if they enter **pass** or **fail** section. In order to do this, during test preparation, a perl script is called to look into the deassembled code (`<test>.dump` file) for given test to find where the **pass/fail** locations are, respectively. These locations are needed by the testbench during runtime to determine if the given test is passing or failing.

### OpenOCD / Debug / GDB Support
Currently, this is not supported with the Chipyard-based CEP.  We'll look to restore this functionality as soon as possible.

### Running the cosim
The cosim uses a hierarchical makefile structure.  Tests can be run individually, as a group (i.e.; testSuite), or as a complete set.

Makefile usage can be obtained by running `make usage` in any of the test, testSuite, or root cosim directories.

Default environment settings can be obtained by running `make sim_info`.

When a test is run, many files are generated.  Some are highlighted below:
  - c_module.o, c_dispatch.o, c_dispatch - Objects and executables for threads supporting the cosim
  - If a baremetal test, `riscv_wrapper[.dump, .elf, .hex, .img, .o]` - Files associated with the RISC-V executable
  - `<test_name>_<simulator>_compile.log`, `<test_name>_<simulator>_opt.log`, `<test_name>_<simulator>_sim.log` - Log files related to different phases of the simulation process
  - status - Indicates pass/fail status of the test run..  used by higher-level (testSuite, whole cosim) test runs to aggregate results
  - testHistory.txt - Contains status summary for all test runs since the last `make clean`
  - vsim.do - Automatically generated TCL script for controlling waveform captured and coverage generation.  A default one is generated by the Makefiles that captures all ports in thle design.  This can be modified post-generation to capture other signals. 
    An example follows:
    ```
    proc wave_on {} {
      echo "vsim.do: Enable logging";  
      log -ports -r /* ;
      log -r /*/aesmodule/* ;
    }
    ```
    In addition to all the ports in the design, the above example captures all the signals in the aesmodule and below.

### Tool Locations 
The following variables can be overwritten (or changed in cep_buildHW.make).  They need to match the tool locations in your system.
  XCELIUM_VERSION  ?= XCELIUMAGILE20.09.001
  XCELIUM_INSTALL  ?= /brewhouse/cad4/x86_64/Cadence/${XCELIUM_VERSION}
  QUESTASIM_PATH   ?= /opt/questa-2019.1/questasim/bin

### Return to the Root CEP [readme](../../README.md).
