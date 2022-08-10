Common Evaluation Platform Licensing Information

Title               : aes
Source              : https://opencores.org/project,tiny_aes
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/aes
License Description : Apache 2.0
License terms       : http://www.apache.org/licenses/LICENSE-2.0.html
Function            : This core provides a standard AES encryption function, where the key is 192 bits long.  It will be integrated as a HW accelerator into the SoC.

Title               : des3
Source              : http://asics.ws/v6/free-ip-cores/#
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/des3
License Description : BSD-based
License terms       :   Copyright (C) 2000-2009, ASICs World Services, LTD. , AUTHORS All rights reserved. Redistribution and use in source, netlist, 
                        binary and silicon forms, with or without modification, are permitted provided that the following conditions are met: 

                        Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
                        Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
                        in the documentation and/or other materials provided with the distribution.
                        Neither the name of ASICS World Services, the Authors and/or the names of its contributors may be used to endorse or promote 
                        products derived from this software without specific prior written permission.
                        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
                        BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
                        SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
                        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
                        INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
                        OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Function            :  This core provides a standard MD5 hash function.  It will be integrated as a HW accelerator into the SoC.

Title               : dsp (FIR and IIR filters)
Source              : Copyright (c) 2005-2014 Peter A. Milder for the Spiral Project, Carnegie Mellon University
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/dsp
License Description : BSD
License Terms       : http://www.opensource.org/licenses/bsd-license.php
Function            : Provides logic for the FIR and IIR

Title               : dsp (DFT) and IDFT)
Source              : Copyright (c) 2005-2014 Peter A. Milder for the Spiral Project, Carnegie Mellon University
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/generated_dsp_code
License Description : Custom
License Terms       : The generated code is NOT included within the CEP repository due to license constraints.  
                      Pointers to generating these files are included for information purposes only.

                      This design is provided for internal, non-commercial research use only
                      and is not for redistribution, with or without modifications.

                      You may not use the name "Carnegie Mellon University" or derivations
                      thereof to endorse or promote products derived from this software.
 
                      THE SOFTWARE IS PROVIDED "AS-IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER
                      EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO ANY WARRANTY
                      THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS OR BE ERROR-FREE AND ANY
                      IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
                      TITLE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
                      BE LIABLE FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO DIRECT, INDIRECT,
                      SPECIAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF, RESULTING FROM, OR IN
                      ANY WAY CONNECTED WITH THIS SOFTWARE (WHETHER OR NOT BASED UPON WARRANTY,
                      CONTRACT, TORT OR OTHERWISE).
Function            : Provides logic for the DFT and IDFT cores

Title               : gps
Source              : MIT Lincoln Laboratory
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/gps
License Description : BSD 2-Clause
License terms       : https://opensource.org/licenses/BSD-2-Clause
Function            : Surrogate GPS core for CA, P, and L code generation
                    : L-code leverages the aes core mentioned above

Title               : md5
Source              : http://pancham.sourceforge.net/
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/md5
License Description : LGPL 2.1
License terms       : https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
Function            : This core provides a standard MD5 hash function.  It will be integrated as a HW accelerator into the SoC.

Title               : rsa
Source              : http://git.level2crm.com/cryptech/core-math-modexp/tree/master
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/rsa
License Description : LGPL-ish
License terms       : // Author: Joachim Strombergson
                      // Copyright (c) 2013, Secworks Sweden AB
                      // All rights reserved.
                      //
                      // Redistribution and use in source and binary forms, with or
                      // without modification, are permitted provided that the following
                      // conditions are met:
                      //
                      // 1. Redistributions of source code must retain the above copyright
                      //    notice, this list of conditions and the following disclaimer.
                      //
                      // 2. Redistributions in binary form must reproduce the above copyright
                      //    notice, this list of conditions and the following disclaimer in
                      //    the documentation and/or other materials provided with the
                      //    distribution.
                      //
                      // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
                      // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
                      // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
                      // FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
                      // COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
                      // INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
                      // BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
                      // LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
                      // CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
                      // STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
                      // ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
                      // ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Function:             This core provides the mathematical base (modular exponentiation) required to implement a standard 2048-bit 
                      RSA public-key encryption function.  It will be integrated as a HW accelerator into the SoC.

Title               : sha256
Source              : http://git.level2crm.com/cryptech/core-hash-sha256/tree/master
CEP Directory       : ./generators/mitll-blocks/src/main/resources/vsrc/sha256
License Description : LGPL-ish
License terms       : // Author: Joachim Strombergson
                      // Copyright (c) 2013, Secworks Sweden AB
                      // All rights reserved.
                      //
                      // Redistribution and use in source and binary forms, with or
                      // without modification, are permitted provided that the following
                      // conditions are met:
                      //
                      // 1. Redistributions of source code must retain the above copyright
                      //    notice, this list of conditions and the following disclaimer.
                      //
                      // 2. Redistributions in binary form must reproduce the above copyright
                      //    notice, this list of conditions and the following disclaimer in
                      //    the documentation and/or other materials provided with the
                      //    distribution.
                      //
                      // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
                      // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
                      // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
                      // FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
                      // COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
                      // INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
                      // BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
                      // LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
                      // CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
                      // STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
                      // ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
                      // ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Function            : This core provides a standard SHA hash function, where the resulting hash value is 256 bits.  It will be integrated as a HW accelerator into the SoC.

Title               : opentitan
Source              : https://github.com/lowRISC/opentitan
CEP Directory       : ./opentitan
License Description : Apache License v2.0
License Terms       : https://github.com/lowRISC/opentitan/blob/master/LICENSE
Function            : OpenTitan is the first open source project building a transparent, high-quality reference design and integration guidelines for silicon root of trust (RoT) chips.
                      Main website: www.opentitan.org

Title               : SD Card model for CEP Co-Simulation
Source              : SD SPI Simulation Model originally released by tsuhuai.chan@gmail.com and subsequented extracted from "Warke, Tejas Pravin, "Verification of 
                      SD/MMC Controller IP Using UVM" (2018). Thesis. Rochester Institute of Technology"
CEP Directory       : ./sims/cep_cosim/dvt/sd_spi_model.v
License Description : Unknown
License Terms       : Unknown
Function            : Provides an SD Card Model for simulation of a "full boot" of the CEP

Title               : UART Model for CEP Co-Simulation
Source              : https://github.com/ben-marshall/uart
CEP Directory       : ./sims/cep_cosim/dvt/uart_rx.v
License Description : MIT License
License Terms       : https://github.com/ben-marshall/uart/blob/master/LICENSE
Function            : Provides a simple UART Receiver for the CEP testbench

Title               : Free FFT and convolution (C)
Source              : https://www.nayuki.io/page/free-small-fft-in-multiple-languages
CEP Directory       : ./sims/cep_cosim/drivers/cep_tests/fft[.cc/.h]
License Description : MIT License
License Terms       : Copyright (c) 2018 Project Nayuki. (MIT License)
                      * Permission is hereby granted, free of charge, to any person obtaining a copy of
                      * this software and associated documentation files (the "Software"), to deal in
                      * the Software without restriction, including without limitation the rights to
                      * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
                      * the Software, and to permit persons to whom the Software is furnished to do so,
                      * subject to the following conditions:
                      * - The above copyright notice and this permission notice shall be included in
                      *   all copies or substantial portions of the Software.
                      * - The Software is provided "as is", without warranty of any kind, express or
                      *   implied, including but not limited to the warranties of merchantability,
                      *   fitness for a particular purpose and noninfringement. In no event shall the
                      *   authors or copyright holders be liable for any claim, damages or other
                      *   liability, whether in an action of contract, tort or otherwise, arising from,
                      *   out of or in connection with the Software or the use or other dealings in the
                      *   Software.

Title               : libgpiod-example
Source              : https://github.com/starnight/libgpiod-example
CEP Directory       : ./software/linux/gpiotest
License Description : BSD 3-Clause

Title               : Chipyard VC707 FPGA build target
Source              : https://github.com/necst/chipyard-vc707
CEP Directory       : ./fpga/src/main/scala/vc707
License Description : BSD 3-Clause