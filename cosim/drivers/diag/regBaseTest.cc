//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stddef.h>
#include <stdlib.h>

#include "simdiag_global.h"
#include "regBaseTest.h"
#include "random48.h"

#include "portable_io.h"

//
// Prototypes
//
void regBaseTest_Construct(regBaseTest_t *me, 
			   int target,
			   int accessSize,
			   int seed,
			   int verbose
			   );
int regBaseTest_WriteReg(regBaseTest_t *me,uint32_t adr, uint64_t dat);
uint64_t regBaseTest_ReadReg(regBaseTest_t *me,uint32_t adr);
int regBaseTest_AddAReg(regBaseTest_t *me,uint32_t adr, uint64_t mask);
int regBaseTest_AddROReg(regBaseTest_t *me,uint32_t adr, uint64_t ROvalue, uint64_t mask);
int regBaseTest_ReadRegNCompare(regBaseTest_t *me,uint32_t adr, uint64_t Data, uint64_t mask);
int regBaseTest_GetMaxBits(regBaseTest_t *me,uint64_t mask) ;
void regBaseTest_Srand48(regBaseTest_t *me,uint64_t seed);
int regBaseTest_uniquifyTest(regBaseTest_t *me,int doWrite, int RdNCheck);
int regBaseTest_walk_1_thruDatTest(regBaseTest_t *me,int doWrite, int RdNCheck);
int regBaseTest_walk_0_thruDatTest(regBaseTest_t *me,int doWrite, int RdNCheck);
int regBaseTest_checkerBoardTest(regBaseTest_t *me);
int regBaseTest_doRORegTest(regBaseTest_t *me);
int regBaseTest_doRegTest(regBaseTest_t *me);
void regBaseTest_SetNoWr2ROreg(regBaseTest_t *me,int value);
void regBaseTest_SetAccessSize(regBaseTest_t *me,int value);
//
// Codes
//

void regBaseTest_Construct(regBaseTest_t *me, 
			   int target,
			   int accessSize,
			   int seed,
			   int verbose
			   ) {
  me->mTarget = target;
  me->mAccessSize = accessSize; // 8/16/32/64bits
  me->mVerbose = 0;   
  //   mVerbose    = verbose;
  me->mRegCnt     = 0;
  me->mROcnt      = 0;
  me->mSeed       = seed;
  me->mMaxBits    = 0;
  me->mStep       = 1;
  me->mNoWr2ROreg = 0;
  me->mLogLevelMask = BASE_LOG_LEVEL;
  //
  // Links
  //
  me->AddAReg_p			= regBaseTest_AddAReg		;              
  me->AddROReg_p		= regBaseTest_AddROReg		;             
  me->GetMaxBits_p		= regBaseTest_GetMaxBits	;           
  me->WriteReg_p		= regBaseTest_WriteReg		;             
  me->ReadReg_p			= regBaseTest_ReadReg		;              
  me->ReadRegNCompare_p		= regBaseTest_ReadRegNCompare	;      
  me->uniquifyTest_p		= regBaseTest_uniquifyTest	;         
  me->walk_1_thruDatTest_p	= regBaseTest_walk_1_thruDatTest;   
  me->walk_0_thruDatTest_p	= regBaseTest_walk_0_thruDatTest;   
  me->checkerBoardTest_p	= regBaseTest_checkerBoardTest	;     
  me->doRORegTest_p		= regBaseTest_doRORegTest	;          
  me->doRegTest_p		= regBaseTest_doRegTest		;            
  me->SetNoWr2ROreg_p		= regBaseTest_SetNoWr2ROreg	;        
  me->SetAccessSize_p		= regBaseTest_SetAccessSize	;        
  me->Srand48_p			= regBaseTest_Srand48		;              
}


//
// Add a register to test
//
int regBaseTest_AddAReg(regBaseTest_t *me, uint32_t adr, uint64_t mask) {
  me->mRegList[(me->mRegCnt*2)+0] = adr;
  me->mRegList[(me->mRegCnt*2)+1] = mask;
  me->mRegCnt++;
  me->mMaxBits = (*me->GetMaxBits_p)(me,mask);
  return(0);
}

int  regBaseTest_WriteReg(regBaseTest_t *me, uint32_t adr, uint64_t dat) {
  LOGE("%s Must be overload\n",__FUNCTION__);
  return -1;
}
uint64_t  regBaseTest_ReadReg(regBaseTest_t *me, uint32_t adr) {
  LOGE("%s Must be overload\n",__FUNCTION__);
  return -1;
}

//
// Add a ReadOnly register to test
//
int regBaseTest_AddROReg(regBaseTest_t *me, uint32_t adr, uint64_t ROvalue, uint64_t mask) {
  me->mROList[(me->mROcnt*3)+0] = adr;
  me->mROList[(me->mROcnt*3)+1] = ROvalue;
  me->mROList[(me->mROcnt*3)+2] = mask;
  me->mROcnt ++;
  me->mMaxBits = (*me->GetMaxBits_p)(me,mask);
  return(0);
}

//
// Read the data point to by adr into rdPat and compare
//
int regBaseTest_ReadRegNCompare(regBaseTest_t *me,uint32_t adr, uint64_t Data, uint64_t mask) {
  
  //int errCnt = 0;
  
  uint64_t rdPat = (*me->ReadReg_p)(me,adr); // read data into rdPat
  
  // now do the compare
  if ((rdPat ^ Data) & mask) {
    LOGE("misCompare adr=%x exp=%016lx act=%016lx\n",adr,Data,rdPat);
    return 1;
  } else if (VERBOSE2()) {
    LOGI("OK: adr=%x exp=%016lx act=%016lx\n",adr,Data,rdPat);
  }
  return 0;
}

int regBaseTest_GetMaxBits(regBaseTest_t *me,uint64_t mask) {
  int maxBits = me->mMaxBits;
  int curBits = 0;
  int j = 31; int notFound = 1;
  while ((j >=0) && notFound) {
    if ((1 << j) & mask) {
      notFound = 0;
      curBits = j+1;
    }
    j--;
  }
  if (curBits > maxBits) { maxBits = curBits; }
  
  //LOGI("maxBits=%d mask=%d\n",maxBits,mask);
  return maxBits;
}

void regBaseTest_Srand48(regBaseTest_t *me,uint64_t seed) {
  me->customDiagRandomNumber = seed;
}

int regBaseTest_uniquifyTest(regBaseTest_t *me,int doWrite, int RdNCheck) {
  int errCnt = 0;
  if (VERBOSE1()) {
    LOGI("== regBaseTest_uniquifyTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  // initialize the seed
  (*me->Srand48_p)(me,me->mSeed + me->mRegCnt);
  if (doWrite) { // doing a write
    for (int i=0;i<me->mRegCnt;i++) {
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t wrDat = (uint64_t) (Random48_mrand48_custom(&me->customDiagRandomNumber) << 32) |
	Random48_mrand48_custom(&me->customDiagRandomNumber);
      errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
    }
  }
  if (RdNCheck) {
    for (int i=0;i<me->mRegCnt;i++) {
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t curMask = me->mRegList[(i*2)+1];
      uint64_t wrDat = (uint64_t) (Random48_mrand48_custom(&me->customDiagRandomNumber) << 32) |
	Random48_mrand48_custom(&me->customDiagRandomNumber);
      // walk a 1 thru adr
      errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    }
  }
  return errCnt;
}

int regBaseTest_walk_1_thruDatTest(regBaseTest_t *me,int doWrite, int RdNCheck) {
  int errCnt = 0;
  if (VERBOSE1()) {
    LOGI("== regBaseTest_walk_1_thruDatTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  if (doWrite) { // doing a write
    for (int i=0;i<me->mRegCnt;i++) {
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t wrDat = (1 << (i%me->mMaxBits));
      errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
    }
  }
  if (RdNCheck) {
    for (int i=0;i<me->mRegCnt;i++) {
      // first set random address base on adr
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t curMask = me->mRegList[(i*2)+1];
      uint64_t wrDat = (1 << (i%me->mMaxBits));
      // walk a 1 thru adr
      errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    }
  }
  return errCnt;
}
int regBaseTest_walk_0_thruDatTest(regBaseTest_t *me,int doWrite, int RdNCheck) {
  int errCnt = 0;
  if (VERBOSE1()) {  
    LOGI("== regBaseTest_walk_0_thruDatTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  if (doWrite) { // doing a write
    for (int i=0;i<me->mRegCnt;i++) {
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t wrDat = ~(1 << (i%me->mMaxBits));
      errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
    }
  }
  if (RdNCheck) {
    for (int i=0;i<me->mRegCnt;i++) {
      // first set random address base on adr
      uint32_t curAdr = me->mRegList[(i*2)+0];
      uint64_t curMask = me->mRegList[(i*2)+1];
      uint64_t wrDat = ~(1 << (i%me->mMaxBits));
      // walk a 1 thru adr
      errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    }
  }
  return errCnt;
}

int regBaseTest_checkerBoardTest(regBaseTest_t *me) {
  int errCnt = 0;
  if (VERBOSE1()) {  
    LOGI("== regBaseTest_checkerBoardTest\n");
  }
  uint32_t curAdr;
  uint64_t curMask,wrDat;  
  // Write 0x55555555,0x55555555 from top -> bottom
  //int lastPat;
  // top->bot
  for (int i= 0;i<me->mRegCnt;i++) {
    curAdr  = me->mRegList[(i*2)+0];
    wrDat   = 0xAAAAAAAAAAAAAAAAULL; if (i & 0x1) wrDat = ~wrDat;
    //lastPat = wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  // Read back to check and write 0xAA...AA into it (bottom -> top)
  for (int i=me->mRegCnt-1;i >= 0;i--) {
    curAdr  = me->mRegList[(i*2)+0];
    curMask = me->mRegList[(i*2)+1];
    // check
    wrDat   = 0xAAAAAAAAAAAAAAAAULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    // and write back
    wrDat   = 0x5555555555555555ULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  // with 0xFF
  for (int i= 0;i<me->mRegCnt;i++) {
    curAdr  = me->mRegList[(i*2)+0];
    curMask = me->mRegList[(i*2)+1];
    // check
    wrDat   = 0x5555555555555555ULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    // and write back
    wrDat   = 0xFFFFFFFFFFFFFFFFULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  // with 0x33...
  for (int i=me->mRegCnt-1;i >= 0;i--) {
    curAdr  = me->mRegList[(i*2)+0];
    curMask = me->mRegList[(i*2)+1];
    // check
    wrDat   = 0xFFFFFFFFFFFFFFFFULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    // and write back
    wrDat   = 0x3333333333333333ULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  // with 0xCC...
  for (int i= 0;i<me->mRegCnt;i++) {
    curAdr  = me->mRegList[(i*2)+0];
    curMask = me->mRegList[(i*2)+1];
    // check
    wrDat   = 0x3333333333333333ULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    // and write back
    wrDat   = 0xCCCCCCCCCCCCCCCCULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  // with 0x00...
  for (int i=me->mRegCnt-1;i >= 0;i--) {
    curAdr  = me->mRegList[(i*2)+0];
    curMask = me->mRegList[(i*2)+1];
    // check
    wrDat   = 0xCCCCCCCCCCCCCCCCULL; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->ReadRegNCompare_p)(me,curAdr,wrDat,curMask);
    // and write back
    wrDat   = 0x0; if (i & 0x1) wrDat = ~wrDat;
    errCnt += (*me->WriteReg_p)(me,curAdr,wrDat);
  }
  return errCnt;
}
//
// ReadOnly register test
//
int regBaseTest_doRORegTest(regBaseTest_t *me) {
  int errCnt = 0;
  if (me->mROcnt) {
    if (VERBOSE1()) {    
      LOGI("== Starting regBaseTest_doRORegTest\n");
    }
    // read back and check
    for (int i=0;i<me->mROcnt;i++) {
      errCnt += (*me->ReadRegNCompare_p)(me,me->mROList[(i*3)+0],me->mROList[(i*3)+1],me->mROList[(i*3)+2]);
    }
    if (!me->mNoWr2ROreg) {
      // write with flip data
      for (int i=0;i<me->mROcnt;i++) {
	errCnt += (*me->WriteReg_p)(me,me->mROList[(i*3)+0],~me->mROList[(i*3)+1]);
      }	
      // read back and check again
      for (int i=0;i<me->mROcnt;i++) {
	errCnt += (*me->ReadRegNCompare_p)(me,me->mROList[(i*3)+0],me->mROList[(i*3)+1],me->mROList[(i*3)+2]);
      }	
    }
  }
  return errCnt;
}
//
// Mother of all tests
//
int regBaseTest_doRegTest(regBaseTest_t *me) {
  if (VERBOSE1()) {
    LOGI("== Starting regBaseTest_doRegTest\n");
  }
  int errCnt = 0;

  // RO Test
  errCnt += (*me->doRORegTest_p)(me);
  //
  // Walking 1 & 0 tests (Write Only)
  //

  errCnt += (*me->uniquifyTest_p)(me,1,0);
  errCnt += (*me->uniquifyTest_p)(me,0,1);
  
  errCnt += (*me->walk_1_thruDatTest_p)(me,1,0);
  errCnt += (*me->walk_1_thruDatTest_p)(me,0,1);

  errCnt += (*me->walk_0_thruDatTest_p)(me,1,0);
  errCnt += (*me->walk_0_thruDatTest_p)(me,0,1);
  //
  // Finally checker board test
  //
  errCnt += (*me->checkerBoardTest_p)(me);
  if (VERBOSE1()) {  
    LOGI("== Done regBaseTest_doRegTest errCnt=%d\n",errCnt);
  }
  //
  return errCnt;

}

void regBaseTest_SetNoWr2ROreg(regBaseTest_t *me,int value) { me->mNoWr2ROreg = value; }
void regBaseTest_SetAccessSize(regBaseTest_t *me,int value) { me->mAccessSize = value; }
