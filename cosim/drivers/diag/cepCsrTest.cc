//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepCsrTest.h"

#include "cep_apis.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#endif

#if (BARE_MODE || LINUX_MODE)
#include "encoding.h"
#else
#include "simPio.h"
#endif


//#include "../bare/encoding.h"

extern void regBaseTest_Construct(regBaseTest_t *me, 
			   int target,
			   int accessSize,
			   int seed,
			   int verbose
			   );

//
// =============================
// The test itself
// =============================
//
int cepCsrTest_runTest(int cpuId, int accessSize,int revCheck,int seed, int verbose) {

  int errCnt = 0;
#if (BARE_MODE || LINUX_MODE)
  uint64_t dat64;

#if 1
  uint64_t saveCustom;
  //
  // testing custom CSR @0x7c1
  // 
  asm volatile ("csrr	%0,0x7c1" : "=r"(saveCustom));
  DUT_WRITE32_64(reg_base_addr + cep_scratch0_reg,saveCustom); // to observe
  if (saveCustom != 0x208) errCnt++;
  if (errCnt) goto apiDone;
  //
  dat64 = 0; // (uint64_t)-1;
  asm volatile ("csrw	0x7c1,%0" : : "r"(dat64));
  //
  asm volatile ("csrr	%0,0x7c1" : "=r"(dat64));
  DUT_WRITE32_64(reg_base_addr + cep_scratch0_reg,dat64); // to observe
  if (dat64 != 0) errCnt++;
  if (errCnt) goto apiDone;
  // put back
  asm volatile ("csrw	0x7c1,%0" : : "r"(saveCustom));
#endif
  
  //
  //
  regBaseTest_t *regp; //
  //
  cepCsrTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);

  //
  // start adding register to test
  //
  //(*regp->AddAReg_p)(regp, CSR_MUCOUNTEREN,(uint64_t)(1<<32)-1);
  //(*regp->AddAReg_p)(regp, CSR_MSCOUNTEREN,(uint64_t)(1<<32)-1);
  //(*regp->AddAReg_p)(regp, CSR_DSCRATCH,(uint64_t)(-1)); // trap!
  //(*regp->AddAReg_p)(regp, CSR_SBADADDR,(uint64_t)(-1));
  //(*regp->AddAReg_p)(reagp, CSR_MISA,(uint64_t)(-1)); // can't write
  /*
  */

  //  (*regp->AddAReg_p)(regp, CSR_SPTBR,(uint32_t)(-1)); // bit[1:0] = 0
  //(*regp->AddAReg_p)(regp, CSR_MINSTRET,(uint32_t)(-1)); // bit[1:0] = 0
  //(*regp->AddAReg_p)(regp, CSR_TDATA2,(uint32_t)(-1)); // bit[1:0] = 0
  //

  //(*regp->AddAReg_p)(regp, CSR_DPC,(uint32_t)(-1)-1); // bit[1:0] = 0
  //(*regp->AddAReg_p)(regp, CSR_DSCRATCH,(uint32_t)(-1)-1); // bit[1:0] = 0

  //
  // Some holes
  //

  // Read Only
  (*regp->SetNoWr2ROreg_p)(regp, 1);
  //
  (*regp->AddROReg_p)(regp, CSR_MISA,     0x800000000094112d, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MVENDORID,0x0000000000000000, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MARCHID,  0x0000000000000001, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MIMPID,   0x0000000020181004, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MHARTID,  cpuId,              (uint64_t)(-1));
#ifdef SIM_ENV_ONLY

  //
  // working
  //
  (*regp->AddAReg_p)(regp, CSR_MTVAL, 0); // 0xFFFFFFFFF0); // 40 bits 
  (*regp->AddAReg_p)(regp, CSR_MTVEC, 0xFFFFFF00); // 
  (*regp->AddAReg_p)(regp, CSR_MEDELEG,(uint64_t)(0)); // full write/no read
  (*regp->AddAReg_p)(regp, CSR_MIDELEG,(uint64_t)(0)); // full write/no read
  //
  (*regp->AddAReg_p)(regp, CSR_MEPC,0xFFFFFFFFFC); // 40 bits 
  (*regp->AddAReg_p)(regp, CSR_SSCRATCH,(uint64_t)(-1));
  (*regp->AddAReg_p)(regp, CSR_MSCRATCH,(uint64_t)(-1));
  //
  // cfg0 contains 8 cfg for PMPAdr0-7
  //
  //(*regp->AddAReg_p)(regp, CSR_PMPCFG0,(uint64_t)(-1));
  (*regp->AddAReg_p)(regp, CSR_PMPADDR0,((uint64_t)(1) << 30) -1); // 30 bits?
  (*regp->AddAReg_p)(regp, CSR_PMPADDR1,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR2,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR3,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR4,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR5,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR6,((uint64_t)(1) << 30) -1); 
  (*regp->AddAReg_p)(regp, CSR_PMPADDR7,((uint64_t)(1) << 30) -1); 

#endif
  /*
  (*regp->AddAReg_p)(regp, CSR_STVEC,(uint64_t)(-1));
  (*regp->AddAReg_p)(regp, CSR_SIE,(uint64_t)(-1));
  (*regp->AddAReg_p)(regp, CSR_SEPC,(uint64_t)(-1));
  */

  //
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);

  //
  // custom
  //
  
  //
  // These to toggle PMP_mask all bit to 1
  //
  // load CSR_PMPCFG0 with
  // bit[7] = Lock
  // bit[6:5] = -NA-
  // [4:3] = A =
  //     0 : disable
  //     1 : TOR: top-of-range
  //     2 : NA4
  //     3 : NAPOT
  // [2] = X
  // [1] = W
  // [0] = R
#ifdef SIM_ENV_ONLY
  //regWr(`CSR_PMPCFG0, {8{8'b0_00_11_1_1_1}});
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPCFG0,0x7F7F7F7F7F7F7F7F);
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR0,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR1,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR2,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR3,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR4,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR5,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR6,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPADDR7,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPCFG0,0);
  errCnt += cepCsrTest_WriteEntry(regp, CSR_PMPCFG0,(uint64_t)(-1)); // with lock
  //
  //
  // some write-Only
  //
  //
  errCnt += cepCsrTest_WriteEntry(regp, CSR_STVAL,(uint64_t)(-1));
  errCnt += cepCsrTest_WriteEntry(regp, CSR_SATP,(uint64_t)(-1));
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MCOUNTEREN,(uint64_t)(-1));
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MINSTRET,(uint64_t)(-1));
  //
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MISA,(uint64_t)(-1));
  // the rest of them just read to complete a full decoding
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER4 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER5 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER6 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER7 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER13 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER15 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER18 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER26 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER27 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER29 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_HPMCOUNTER31 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_PMPCFG2 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_PMPADDR9 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_PMPADDR10 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_PMPADDR11 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_PMPADDR12 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER3 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER4 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER5 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER6 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER7 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER8 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER13 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER15 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER17 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER18 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER19 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER27 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER29 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER30 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER31 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT4 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT5 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT6 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT13 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT14 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT15 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT17 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT18 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT19 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT20 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT21 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT28 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT29 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMEVENT30 	);
#endif
  //
  // Destructors
  //
  cepCsrTest_DELETE(regp);
#endif
  //
  apiDone:
  return errCnt;
}

//
// Overload functions
//
int 
cepCsrTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
#if (BARE_MODE || LINUX_MODE)
  switch (adr) {
#undef RISCV_ENCODING_H
#undef DECLARE_CSR
#define DECLARE_CSR(name, num) case num: write_csr(name,dat); break;
#include "encoding.h"
#undef RISCV_ENCODING_H
#undef DECLARE_CSR
  }
#endif
  return 0;
}

uint64_t  cepCsrTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
#if (BARE_MODE || LINUX_MODE)
  uint64_t dat;
  switch (adr) {
#undef RISCV_ENCODING_H
#undef DECLARE_CSR
#define DECLARE_CSR(name, num) case num: dat = read_csr(name); break;
#include "encoding.h"
#undef RISCV_ENCODING_H
#undef DECLARE_CSR
}
  return dat;
#else
  return 0;
#endif
}
