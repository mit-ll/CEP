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

#include "cepRegTest.h"
#include "cepregression.h"


//#define printf(...) { return 0; }

#ifdef __cplusplus
extern "C" {
#endif

  uint64_t atomicOps_playUnit[4];
  
//int main(void)
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  int coreId = read_csr(mhartid);
  //
  set_printf(0);

  int l;
  uint64_t *ptr;
  uint64_t expVal;
  //
  //
  set_cur_status(CEP_RUNNING_STATUS);
  //
  for (l=0;l<3;l++) {
    switch (l) {
    case 0 : 
      // cbus via CLINT
      ptr = (uint64_t *)(clint_base_addr + clint_mtimecmp_offset + (coreId*8));      
      expVal = ~(clint_base_addr + clint_mtimecmp_offset * (coreId*8));
      break;

    case 1 : 
      // mbus: cacheable memory
      ptr = &atomicOps_playUnit[coreId];
      expVal = (reg_base_addr + cep_scratch0_reg * (coreId*8));      
      break;
      
    case 2 :  
      // pbus
      ptr = (uint64_t *)(reg_base_addr + cep_scratch0_reg + (coreId*8));      
      expVal = ~(reg_base_addr + cep_scratch0_reg * (coreId*8));
      break;

    }
    //
    //
    if (!errCnt) { errCnt = cep_runAtomicTest(ptr,expVal); }
  }
  //
  //
  // Done
  //
  set_status(errCnt,(l << 16) | errCnt);
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
