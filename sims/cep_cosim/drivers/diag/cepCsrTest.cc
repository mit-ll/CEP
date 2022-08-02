//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "CEP.h"
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

  uint64_t saveCustom;
  //
  // testing custom CSR @0x7c1
  // 
  asm volatile ("csrr	%0,0x7c1" : "=r"(saveCustom));
  DUT_WRITE32_64(CEPREGS_BASE_ADDR + cep_scratch0_reg,saveCustom); // to observe
  if (saveCustom != 0x208) errCnt++;
  if (errCnt) goto apiDone;
  //
  dat64 = 0; // (uint64_t)-1;
  asm volatile ("csrw	0x7c1,%0" : : "r"(dat64));
  //
  asm volatile ("csrr	%0,0x7c1" : "=r"(dat64));
  DUT_WRITE32_64(CEPREGS_BASE_ADDR + cep_scratch0_reg,dat64); // to observe
  if (dat64 != 0) errCnt++;
  if (errCnt) goto apiDone;
  // put back
  asm volatile ("csrw	0x7c1,%0" : : "r"(saveCustom));

  regBaseTest_t *regp;
  cepCsrTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);

  // Some holes

  // Read Only
  (*regp->SetNoWr2ROreg_p)(regp, 1);

  (*regp->AddROReg_p)(regp, CSR_MISA,     0x800000000094112d, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MVENDORID,0x0000000000000000, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MARCHID,  0x0000000000000001, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MIMPID,   0x0000000020181004, (uint64_t)(-1));
  (*regp->AddROReg_p)(regp, CSR_MHARTID,  cpuId,              (uint64_t)(-1));

  (*regp->AddAReg_p)(regp, CSR_MBADADDR, 0); // 0xFFFFFFFFF0); // 40 bits 
  (*regp->AddAReg_p)(regp, CSR_MTVEC, 0xFFFFFF00); // 
  (*regp->AddAReg_p)(regp, CSR_MEDELEG,(uint64_t)(0)); // full write/no read
  (*regp->AddAReg_p)(regp, CSR_MIDELEG,(uint64_t)(0)); // full write/no read
  //
  (*regp->AddAReg_p)(regp, CSR_MEPC,0xFFFFFFFFFC); // 40 bits 
  (*regp->AddAReg_p)(regp, CSR_SSCRATCH,(uint64_t)(-1));
  (*regp->AddAReg_p)(regp, CSR_MSCRATCH,(uint64_t)(-1));
  
  errCnt = (*regp->doRegTest_p)(regp);

  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER3,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER4,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER5,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER6,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER7,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER8,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER9,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MHPMCOUNTER10,(1 << 29)-1); // 'b0_<29bits Of1 = mask = all 1s

  // Write-Only
  errCnt += cepCsrTest_WriteEntry(regp, CSR_SBADADDR,(uint64_t)(-1));
  errCnt += cepCsrTest_WriteEntry(regp, CSR_SPTBR,(uint64_t)(-1));
  errCnt += cepCsrTest_WriteEntry(regp, CSR_MINSTRET,(uint64_t)(-1));

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
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER9 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER10 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER11 	);
  dat64 += cepCsrTest_ReadEntry(regp,CSR_MHPMCOUNTER12 	);
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

  // Destructors
  cepCsrTest_DELETE(regp);

#endif

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
