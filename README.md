[//]: # (Copyright 2022 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

# Common Evaluation Platform v4.2

[![DOI](https://zenodo.org/badge/108179132.svg)](https://zenodo.org/badge/latestdoi/108179132)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

<p align="center">
    <img src="./cep_docs/cep_logo.jpg" width="375" height="159">
</p>
<p align="center">
   Copyright 2022 Massachusetts Institute of Technology
</p>
<p align="center">
   <img src="./cep_docs/cep_architecture.jpg" width="1114" height="450">
</p>
<p align="center">
    <img src="./cep_docs/related_logos.jpg" width="442" height="120">
</p>

The Common Evaluation Platform (CEP) is an SoC design that contains only license-unencumbered, freely available components.  The CEP includes a range of accelerator cores coupled with a key delivery mechanism, and parametrically-defined challenge modules which can be synthesized to support developmental testing. The implementation of the CEP includes a comprehensive verification environment to ensure modifications do not impede intended functionality. It is intended to be targeted to either an FPGA or ASIC implementation. 

Please check the [CEP changelog](./CHANGELOG.CEP.md) for release history.

Beginning with CEP v4.0, the platform has been ported to the UCB Chipyard Framework.  The original Chipyard Readme can be found [here](./README.Chipyard.md).

Throughout the CEP READMEs, `<CEP_ROOT>` refers to the root directory of the cloned CEP repository.

## Pre-requisites (validated test/build configurations):
The following items describe the configuration of the system that CEP has been developed and tested on:
* Ubuntu 18.04 LTS x86_64 with Modelsim Questa Sim-64 v2019.1 (for co-simulation)
* Red Hat Enterprise Linux 7 with Cadence XCELIUMAGILE20.09.001, VMANAGERAGILE20.06.001
* Xilinx Vivado 2020.1 (needed for building FPGA targets)
  - Plus Digilent Adept Drivers for programming the FPGA target, https://reference.digilentinc.com/reference/software/adept/start?redirect=1#software_downloads)
* Terminal emulator (such as `minicom`)
* bash

Other configurations may work, but they have not been explicitly verified.

Instructions on how to modelsim, xcelium, and Vivado are beyond the scope of this README.

## Setting up your environment

To build the CEP, several packages and toolsets must be installed and built.  The typical steps are listed below.  Additional information can be found in the Chipyard Documentation [here](https://chipyard.readthedocs.io/en/latest/Chipyard-Basics/index.html).

A note about proxies: If your system is behind a proxy, you'll want to ensure your environment is properly configured.  Exact details vary by system, but the proxy needs to be available to apt / yum, curl, and sbt (Simple Build Tool for Scala)

If using RHEL7, you need to ensure gcc 7.x.x+ is installed.  This can be found in the `rhel-workstation-rhscl-7-rpms`  or `rhel-server-rhscl-7-rpms` repos, whose available is RHEL subscription dependent.  Once the repo is enabled, the appropriate gcc can be installed by running `sudo yum install devtoolset-7-gcc-c++`.  Once installed, you want to run `scl enable devtoolset-7 bash` (or whatever version you have installed) to ensure g++ maps to the new version.

* Install git if not already present on your system
  * Ubuntu - `sudo apt install git`
  * RHEL7  - `sudo yum install git`
* Clone the CEP repository, change to the directory of the clone
  * `git clone https://github.com/mit-ll/CEP.git`
* Install package dependencies.  Copies of these files can also be found in the Chipyard Documentation listed above
  * Ubuntu - `./scripts/ubuntu-reqs.sh`
  * RHEL7  - `./scripts/centos-reqs.sh`
* Initialize all the git submodules (including FPGA-related submodules).  There may be a warning about this not being a true chipyard repository which you can answer yes to.
  * `./scripts/init-submodules-no-riscv-tools.sh`
  * `./scripts/init-fpga.sh`
* Build the RISC-V Toolchain.  
  * Depending on your available hardware, you can expedite the build by executing `export MAKEFLAGS=-jN` prior to running the build script.  N is the number of cores you can devote to the build.
  * `./scripts/build-toolchains.sh riscv-tools`
* RHEL7: The chipyard build needs make v4.x or later, which is not included in the default packages.  Recommend building from source (https://ftp.gnu.org/gnu/make/).  Once installed, you can force the version of make used using the following: `MAKE=/usr/local/bin/make ./scripts/build-toolchains.sh riscv-tools`
* It is advisable to move the compiled toolchain outside of the current repo if you plan to have multiple CEP working directories.  Complete directions are beyond the scope of this document, but they do include moving the `riscv-tools-install` directory and `env-riscv-tools.sh` file.  Modification of the aforementioned file as well as `env.sh` will required for smooth operation
* Sometimes the toolchain build may fail.  One may need to run the build several times.
* Once the toolchain is built, your want to source the new environment script: `source <CEP_ROOT>/env.sh`.

## Repository Directory Structure
Providing a complete directory structure is impractical, but some items are highlighted here.  It is worth noting that most of the structure is inherited from Chipyard.

```
<CEP_ROOT> 
  |- ./sims/cep_cosim/ -  
  |     Defines the CEP co-simulation evironment for performing "chip" level simulations of the CEP in 
  |     either bare metal or bus functional model (BFM) mode.  
  |- ./generators/mitll-blocks/
  |   |- src/main/scala - Contains all the custom CEP Chisel code
  |   |- src/main/resources/vsrc/ - SystemVerilog / Verilog files associated with the CEP build
  |       |- generated_dsp_code - Location to place the `dft_top.v` and `idft_top.v'
  |       |- opentitan  - Soft-link to the opentitan submodule located at ./opentitan
  |       |- aeees      - Challenge module.  Read the README.md in this directory for more information.
  |       |- auto-fir   - Challenge module.  Read the README.md in this directory for more information.
  |       |- shaaa      - Challenge module.  Read the README.md in this directory for more information.
  |- ./cep_docs - Documents and images unique to the CEP
  |- ./software/baremetal - Examples of bare metal code that can be run on the Arty100T FPGA target
                            independent of the CEP Co-Simulation environment
```

### Building the CEP FPGA
Multiple Chipyard *SUB_PROJECTS* have been defined for the CEP when targetting FPGA Development boards.

These subprojects define the system configuration and are as follows:

`cep_arty100t` - Arty100T Development Board 
- 50 MHz Core Frequency
- 98% LUT Utilization
- 1x WithNBigCore
- CEP Registers
- AES Core
- Surrogate Root of Trust (SRoT)

`cep_vc707` - VC707 Development Board
- 100 MHz Core Frequency4
- 11% LUT Utilization
- 1x WithNBigCore
- CEP Registers
- AES Core
- Surrogate Root of Trust (SRoT)

`cep_big_vc707` - VC707 Development Board
- 100 MHz Core Frequency
- 70% LUT Utilization
- 4x WithNBigCores
- CEP Registers
- AES Core
- DES3 Core
- FIR Core
- IIR Core
- DFT Core
- IDFT Core
- MD5 Core
- 4x GPS Cores
- 4x SHA-256 Cores
- RSA Core
- Surrogate Root of Trust

`cep_vcu118` - VCU118 Development Board
- 100 MHz Core Frequency
- 5% LUT Utilization
- 1x WithNBigCore
- CEP Registers
- AES Core
- Surrogate Root of Trust (SRoT)

Assuming the Vivado environment scripts have been sourced within your current shell, the following commands can be used to build and program the FPGA *SUB_PROJECT*.  Programming requires that the digilent drivers have been installed and that you have a USB connection to the JTAG USB-port of you preffered FPGA board.

Default CEP builds can be customized by following the instructions in the Chipyard documentation.

The FPGA boards will configure from FLASH or JTAG based on the state of the MODE jumper.  Additional information can be found:
* Arty100T - [here](https://digilent.com/shop/arty-a7-artix-7-fpga-development-board/).
* VC707    - [here](https://www.xilinx.com/products/boards-and-kits/ek-v7-vc707-g.html/).
* VCU118   - [here](https://www.xilinx.com/products/boards-and-kits/vcu118.html/).

```
cd <REPO_ROOT>/fpga
make SUB_PROJECT=<cep_arty100t | cep_vc707 | cep_vcu118>

./program_<arty100t | vc707 | vcu118>_flash.sh - Create the MCS file & program the development board's flash.  Power needs to be cycled or the *PROG* button needs to be asserted to reboot with the new configuration.

OR

./program_<arty100t | vc707 | vcu118>_jtag.sh - Program the FPGA via JTAG.  System will automatically reset or you can use the *RESET* button.
```

### Building Bare Metal software for the CEP FPGA

The Arty100T shares a single microUSB connector for JTAG and UART, while the VC707 and VCU118 have seperate ports for each.

For the Arty100T, connect a Digilent SD or microSD PMOD board o connector JA.  For the VCU118, connect the same to the PMOD connector on the right side of the board.  The PMOD connectors can be ordered from Digikey, Digilent, or other distributors.

Additional information can be found here: (https://digilent.com/shop/pmod-sd-full-sized-sd-card-slot/ or https://digilent.com/shop/pmod-microsd-microsd-card-slot/).

As noted, for the Arty100T the microUSB port uses an FTDI chip to provide both JTAG and UART functionality.  Your system may differ, but typically the UART shows up as `/dev/ttyUSB0` or `/dev/ttyUSB1`.  UART settings are 115200baud, 8N1 and should be visible to any terminal program.  Both HW and SW flow control should be disabled.  

It is worth noting that *minicom* enables HW flow control by default.

Once released from reset, the CEP's bootrom will read the baremetal executable from the SD card, copy it DDR memory, and then jump to that location and execute the program.  The bootrom's default payload size is large enough for a linux boot.  For bare metal executables, the payloads are typically much smaller.  The payload size can be overriden at boot time by holding *BTN0* on the Arty100T or *SWN* on the VC707/VCU118 when the chip is released from reset.

An example UART output for the baremetal gpiotest follows:
```
---          Common Evaluation Platform v4.20            ---
---         Based on the UCB Chipyard Framework          ---
--- Copyright 2022 Massachusetts Institute of Technology ---
---     BootRom Image built on Aug  1 2022 12:41:36      ---

INIT
CMD0
CMD8
ACMD41
CMD58
CMD16
CMD18
LOADING 128kB PAYLOAD
....
BOOT


--------------------------
     RISC-V GPIO Test     
--------------------------
     Built for VCU118
   Console Echo Enabled   


gpio = 00000010
gpio = 00000000
gpio = 00000020
gpio = 00000000
gpio = 00000080
gpio = 00000000
gpio = 00000040
gpio = 00000000
...
```

A developer may use baremetal software from the CEP cosimulation or the examples as provided in `<CEP_ROOT>/software/baremetal`.  

The (micro)SD card needs to be partitioned as described in (https://chipyard.readthedocs.io/en/latest/Prototyping/VCU118.html#running-linux-on-vcu118-designs).  Once partitioned, proceed to the next step.

In either case, it is important to note what device your (micro)SD card gets mapped to (e.g., `/dev/sdd`).

Using `<CEP_ROOT>/sims/cep_cosim/testSuites/bareMetal/regTest` as an example, the following steps will build and load the executable onto the (micro)SD card.

```
cd <CEP_ROOT>/sims/cep_cosim/testSuites/bareMetal/regTest
make DISABLE_KPRINTF=0 riscv_wrapper            <-- builds riscv_wrapper.img with console printf enabled
make DISK=/dev/sdd1 riscv_wrapper_sd_write      <-- copies riscv_wrapper.img to partition /dev/sdd1 (subsitute with your partition name)
```

In the above example, the bare metal regTest is built with the console printf function enabled.

The steps in `<CEP_ROOT>/software/baremetal/gpiotest` are slight different.

```
cd <CEP_ROOT>/software/baremetal/gpiotest
make DISK=/dev/sdd1 sd_write                    <-- copies gpiotest.img to partition /dev/sdd1 (subsitute with your device name)
```

It is worth noting that the examples in `<CEP_ROOT>/software/baremetal` do not require the compilation of the all the cosimulation libraries, but as a result, will not have access to those support functions.

### Booting Linux
The CEP Arty100T/VC707/VCU118 builds has been verified to support a firemarshall-based linux build by following the default workload instructions [here](https://chipyard.readthedocs.io/en/latest/Prototyping/VCU118.html#running-linux-on-vcu118-designs).

A couple of notes:
- The SD card must be partitioned as instructed
- Due to a bug in libguestfs on Ubuntu, the firemarshal build *may* fail.  Ensure your current shell has active sudo permissions before running the build.  I used a quick `sudo su`, exited the root shell, and then ran the build.
- Customization of the linux build will affect the resulting image size `<CEP_ROOT>/software/firemarshal/images/br-base-bin-nodisk-flat`.  Ensure that `PAYLOAD_SIZE_B` in `<CEP_ROOT>/sims/cep_cosim/bootrom/sd.c` is sized to accomodated the image.

### Linux Applications
You can install an example application in firemarshal's buildroot prior to building linux by running `make MAINPROGRAM=<prog name> install` from <CEP_ROOT>/software/linux.  Applications include `helloworld` and `gpiotest`. 

It is advisable to clean the buildroot build, should you change the configuration.  This can accomplished by running `make clean` within `<CEP_ROOT>/software/firemarshal/boards/prototype/distros/br/buildroot/`.

If you ncurses-based gpiotest application crashes before cleanup, executing the `reset` command should restore terminal behavior.

### CEP Co-Simulation
For simulation using the CEP Co-Simulation environment, the `cep_cosim` and `cep_cosim_asic` *SUB_PROJECTS* are defined in `<CEP_ROOT>/variables.mk`.  At this time, due to licensing constraints, the CEP ASIC build is not available as part of this repository.  As a result, any attempt to build it will fail given that a multitude of files are missing.  

Instructions on the CEP Co-Simulation (including the Chipyard build) can be found [here](./sims/cep_cosim/README.md).

### Generated DSP code notes
Due to licensing contraints, two of the DSP cores used during CEP development cannot be included in our repository.  Instructions on generating all the cores can be found [here](./generators/mitll-blocks/src/main/resources/vsrc/dsp/README.md).

## Errors? Ommissions? Questions?
Please feel free to file a github issue which the CEP developers check frequently.

## Citation Information
Citation information is contained [here](./citation.cff)

## Licensing
As the CEP has been developed with input from many sources, multiple licenses apply.  Please refer to the following files for licensing info. 
* [CEP License](./LICENSE.md)
* [CEP Components Licenses](./LICENSE.md)
* [Chipyard License](./LICENSE.md)
* [SiFive License](./LICENSE.SiFive.md)

## DISTRIBUTION STATEMENT A. Approved for public release. Distribution is unlimited.
This material is based upon work supported by the Defense Advanced Research Projects Agency under Air Force Contract No. FA8702-15-D-0001. Any opinions, findings, conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the Defense Advanced Research Projects Agency.

© 2022 Massachusetts Institute of Technology

The software/firmware is provided to you on an As-Is basis.

Delivered to the U.S. Government with Unlimited Rights, as defined in DFARS Part 252.227-7013 or 7014 (Feb 2014). Notwithstanding any copyright notice, U.S. Government rights in this work are defined by DFARS 252.227-7013 or DFARS 252.227-7014 as detailed above. Use of this work other than as specifically authorized by the U.S. Government may violate any copyrights that exist in this work
