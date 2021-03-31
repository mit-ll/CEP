//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#include "v2c_cmds.h"
#include "simPio.h"
//
#include "shMem.h"
#include "c_module.h"
#include <unistd.h>
#include "random48.h"

#include "cep_adrMap.h"
#include "cep_apis.h"
#include "simdiag_global.h"
#include "cepFragmentorTest.h"

//
void *c_module(void *arg) {


  // ======================================
  // Set up
  // ======================================
  pthread_parm_t *tParm = (pthread_parm_t *)arg;
  int errCnt = 0;
  int slotId = tParm->slotId;
  int cpuId = tParm->cpuId;
  int verbose = tParm->verbose;
  Int32U seed = tParm->seed;
  int restart = tParm->restart;
  int offset = GET_OFFSET(slotId,cpuId);
  GlobalShMemory.getSlotCpuId(offset,&slotId,&cpuId);
  //printf("offset=%x seed=%x verbose=%x GlobalShMemory=%x\n",offset,seed, verbose,(unsigned long) &GlobalShMemory);
  // notify I am Alive!!!
  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetAliveStatus();
  sleep(1);

  // ======================================
  // Test is Here
  // ======================================
  simPio pio;
  pio.MaybeAThread(); // chec
  pio.EnableShIpc(1);
  pio.SetVerbose(verbose);

  //
  // ======================================
  // Test starts here
  // ======================================
  // MUST
  // wait until Calibration is done..
  //int calibDone = calibrate_ddr3(50);
  pio.RunClk(500);
  uint64_t rdBuf[32];
  //
#if 1
  switch (cpuId) {
  case 0 : 
    // HOLES
    errCnt += cepFragmentorTest_baseTest(cpuId, uart_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, spi_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, gpio_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, plic_base_addr + 0x8000,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, debug_controller_base_addr+ 0x100,  16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    //
    errCnt += cepFragmentorTest_baseTest(cpuId, rsa_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, dft_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, idft_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!    
    errCnt += cepFragmentorTest_baseTest(cpuId, fir_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, iir_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, gps_base_addr  + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    //   errCnt += cepFragmentorTest_baseTest(cpuId, srot_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    break;

  case 1 : 
    // AES
    errCnt += cepFragmentorTest_baseTest(cpuId, aes_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, aes_base_addr + aes_key1_addr,  2, 2, (uint64_t)-1, seed, verbose); 
    // SHA256 msg input word 0  is not aligned
    errCnt += cepFragmentorTest_baseTest(cpuId, sha256_base_addr + 0x0100,     16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, sha256_base_addr + 0x10,        2, 2, (uint64_t)-1, seed, verbose); 
    errCnt += cepFragmentorTest_baseTest(cpuId, sha256_base_addr + 0x20,        4, 4, (uint64_t)-1, seed, verbose); 
    break;

  case 2 : 
    // MaskRom
    errCnt += cepFragmentorTest_verifyMaskRom(cpuId, bootrom_base_addr, "../../drivers/bootbare/bootbare.bin", 8, seed,verbose);
    // MD5
    errCnt += cepFragmentorTest_baseTest(cpuId, md5_base_addr + 0x0100,        16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, md5_base_addr + 0x10,           2, 2, (uint64_t)-1, seed, verbose); 
    errCnt += cepFragmentorTest_baseTest(cpuId, md5_base_addr + 0x20,           4, 4, (uint64_t)-1, seed, verbose); 
    //
    // CEP regs
    //
    errCnt += cepFragmentorTest_baseTest(cpuId, reg_base_addr + 0x0100,          16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, reg_base_addr + cep_scratch0_reg, 8, 8, (uint64_t)-1, seed, verbose);
    break;
    //
    // max is 8, write 16 words OK but hang on read
    //
  case 3 : 
    // DES3 Plaintext Word 1
    errCnt += cepFragmentorTest_baseTest(cpuId, des3_base_addr + 0x0100,       16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, des3_base_addr + 0x10,          2, 2, ((uint64_t)1 << 56)-1, seed, verbose); 
    errCnt += cepFragmentorTest_baseTest(cpuId, des3_base_addr + 0x20,          4, 4, ((uint64_t)1 << 56)-1, seed, verbose); 
    // CLINT
    errCnt += cepFragmentorTest_baseTest(cpuId, clint_base_addr + 0x0100,               16, 8, (uint64_t) 0, seed, verbose); // HOLE!!
    errCnt += cepFragmentorTest_baseTest(cpuId, clint_base_addr + clint_mtimecmp_offset, 8, 8, (uint64_t)-1, seed, verbose);
    break;

  }
#endif
  //
  //
  pio.RunClk(100);  
  //
  // ======================================
  // Exit
  // ======================================
cleanup:
  if (errCnt != 0) {
    LOGI("======== TEST FAIL ========== %x\n",errCnt);
  } else {
    LOGI("======== TEST PASS ========== \n");    
  }
  //  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetError(errCnt);
  ptr->SetThreadDone();
  pthread_exit(NULL);
  return ((void *)NULL);
}

