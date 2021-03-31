//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
//
// For bareMetal mode ONLY
//
#ifdef BARE_MODE
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "cep_riscv.h"
#include "portable_io.h"

#include "cepRegTest.h"
#include "cepregression.h"


//#define printf(...) { return 0; }

#ifdef __cplusplus
extern "C" {
#endif

  uint64_t playUnit;
  
//int main(void)
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  int coreId = read_csr(mhartid);
  //
  set_printf(0);

  uint64_t *ptr;
  uint64_t mySig,partSig,incVal;
  uint64_t fixSig[4] = {0x12345678ABCD0000ULL,
			0x1122334455660000ULL,
			0x778899aabbcc0000ULL,
			0xddeeff0011220000ULL};
  int loop = 16;
  //
  // ALl core touch this same register
  //
  ptr = &playUnit; // (uint64_t *)(reg_base_addr + cep_scratch0_reg);
  incVal = 4;
  switch (coreId) {
  case 0:
    mySig   = fixSig[0] + 0x0;
    partSig = fixSig[1] + 0x1; // next coreId
    DUT_WRITE32_64(ptr, mySig); // start for core 0
    break;
  case 1:
    mySig   = fixSig[1] + 0x1;
    partSig = fixSig[2] + 0x2; // next coreId
    break;
  case 2:
    mySig   = fixSig[2] + 0x2;
    partSig = fixSig[3] + 0x3; // next coreId
    break;
  case 3:
    mySig   = fixSig[3] + 0x3;
    partSig = fixSig[0] + 0x4; // next coreId
    break;
  }
  //
  //
  set_cur_status(CEP_RUNNING_STATUS);
  if (!errCnt) { errCnt = cep_exchAtomicTest(coreId,ptr, mySig, partSig, incVal, loop); }
  //
  //
  //
  // Done
  //
  set_status(errCnt,errCnt);
  /*
  if (errCnt) {
    set_pass();
  } else {
    set_fail();
  }

  uint64_t *ptr=0;
  errCnt += (int)cep_atomic_op(ptr,1,ATOMIC_OP_ADD);
  */
  //
  // Stuck here forever...
  //
  exit(errCnt);
}

#ifdef __cplusplus
}
#endif
  
#endif
