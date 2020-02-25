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
#include "memBaseTest.h"
#include "random48.h"

#include "portable_io.h"

// Prototypes
void memBaseTest_Construct(memBaseTest_t *me, 
			   uint64_t baseAdr,
			   int adrSize,
			   int datSize,
			   int step,
			   int seed
			   );

int memBaseTest_CleanReadBuf(memBaseTest_t *me);
int memBaseTest_WriteEntry(memBaseTest_t *me, uint64_t adr) ;
int memBaseTest_ReadEntry(memBaseTest_t *me, uint64_t adr) ;
int memBaseTest_ReadEntryNCompare(memBaseTest_t *me, uint64_t adr) ;
int memBaseTest_notEqual(memBaseTest_t *me) ;
void memBaseTest_Set1Pattern(memBaseTest_t *me, int bitPos) ;
void memBaseTest_Set0Pattern(memBaseTest_t *me, int bitPos) ;
void memBaseTest_SetPattern(memBaseTest_t *me, uint64_t *pat2set) ;
uint64_t memBaseTest_SetIncPattern(memBaseTest_t *me, uint64_t startPat) ;
void memBaseTest_RepPattern(memBaseTest_t *me, uint64_t pat) ;
void memBaseTest_InvPattern(memBaseTest_t *me) ;
void memBaseTest_XORPattern(memBaseTest_t *me, int bitPos) ;
void memBaseTest_SetRandomPattern(memBaseTest_t *me, uint64_t seed) ;
void memBaseTest_Print(memBaseTest_t *me, const char *str, uint64_t adr, int printMask, int withAdr) ;
void memBaseTest_setDatMsk(memBaseTest_t *me) ;
void memBaseTest_setAdrMsk(memBaseTest_t *me) ;
void memBaseTest_setCheckerMsk(memBaseTest_t *me, int newMask) ;
int memBaseTest_incPatTest(memBaseTest_t *me, int doWrite, int RdNCheck) ;
int memBaseTest_incPatRangeTest(memBaseTest_t *me, uint64_t startAdr, uint64_t endAdr, int doWrite, int RdNCheck) ;
int memBaseTest_walk_1_thruAdrTest(memBaseTest_t *me, int doWrite, int RdNCheck) ;
int memBaseTest_walk_0_thruAdrTest(memBaseTest_t *me, int doWrite, int RdNCheck) ;
int memBaseTest_GetMaxDataSize(memBaseTest_t *me) ;
int memBaseTest_walk_1_thruDatTest(memBaseTest_t *me, int doWrite, int RdNCheck) ;
int memBaseTest_walk_0_thruDatTest(memBaseTest_t *me, int doWrite, int RdNCheck) ;
int memBaseTest_checkerBoardTest(memBaseTest_t *me) ;
int memBaseTest_doMemTest(memBaseTest_t *me) ;
int memBaseTest_dump(memBaseTest_t *me, uint64_t startAdr, uint64_t endAdr);

//
// Contructors/Destructor
//
void memBaseTest_Construct(memBaseTest_t *me, 
			   uint64_t baseAdr,
			   int adrSize,
			   int datSize,
			   int step,
			   int seed
			   )
{
  int i;
  //
  // default members
  //
  me->mBAR            = baseAdr;
  me->mAdrSize        = adrSize;
  me->mDatSize        = datSize;
  me->mInterface      = 0;
  me->mPri            = 0;
  me->mStep           = step;
  me->mSeed           = seed;
  me->mLogLevelMask   = BASE_LOG_LEVEL;
  if(GET_VERBOSE(me)) {
    LOGI("%s me->mBAR=%016lx me->mAdrSize=%d me->mDatSize=%d me->mStep=%d me->mSeed=%x\n",__FUNCTION__,
	 me->mBAR,me->mAdrSize, me->mDatSize,me->mStep,me->mSeed);
  }
  me->mStopOnMaxErr    = 1;
  me->mCheckerMask    = 0xFF; // pattern
  me->mCleanRdBuf     = 1;
  me->mRetryCnt = 0;
  for (i=0;i<16;i++) {
    me->wrPat[i] = 0;
    me->rdPat[i] = 0;
  }
  //
  // default methods
  //
  me->WriteEntry_p         = memBaseTest_WriteEntry;
  me->ReadEntry_p          = memBaseTest_ReadEntry;
  me->CleanReadBuf_p       = memBaseTest_CleanReadBuf;
  me->ReadEntryNCompare_p  = memBaseTest_ReadEntryNCompare;
  me->notEqual_p           = memBaseTest_notEqual;
  me->Set1Pattern_p        = memBaseTest_Set1Pattern;
  me->Set0Pattern_p        = memBaseTest_Set0Pattern;
  me->SetPattern_p         = memBaseTest_SetPattern;
  me->SetIncPattern_p      = memBaseTest_SetIncPattern;
  me->RepPattern_p         = memBaseTest_RepPattern;
  me->InvPattern_p         = memBaseTest_InvPattern;
  me->XORPattern_p         = memBaseTest_XORPattern;
  me->SetRandomPattern_p   = memBaseTest_SetRandomPattern;
  me->Print_p              = memBaseTest_Print;
  me->setDatMsk_p          = memBaseTest_setDatMsk;
  me->setAdrMsk_p          = memBaseTest_setAdrMsk;
  me->setCheckerMsk_p      = memBaseTest_setCheckerMsk;
  me->incPatTest_p         = memBaseTest_incPatTest;
  me->incPatRangeTest_p    = memBaseTest_incPatRangeTest;
  me->walk_1_thruAdrTest_p = memBaseTest_walk_1_thruAdrTest;
  me->walk_0_thruAdrTest_p = memBaseTest_walk_0_thruAdrTest;
  me->GetMaxDataSize_p     = memBaseTest_GetMaxDataSize;
  me->walk_1_thruDatTest_p = memBaseTest_walk_1_thruDatTest;
  me->walk_0_thruDatTest_p = memBaseTest_walk_0_thruDatTest;
  me->checkerBoardTest_p   = memBaseTest_checkerBoardTest;
  me->doMemTest_p          = memBaseTest_doMemTest;
  me->dump_p               = memBaseTest_dump;
  //
  // Derived
  //
  (*me->setDatMsk_p)(me);
  (*me->setAdrMsk_p)(me);
  //
  me->Random48_customDiagRandomNumber = seed;
}
//
// Default read/write
//
//
//
int memBaseTest_WriteEntry(memBaseTest_t *me, uint64_t adr) {
  LOGE("%s Must be overload\n",__FUNCTION__);
  return -1;
}

//
// return 0 if sucess
//
int memBaseTest_ReadEntry(memBaseTest_t *me, uint64_t adr) {
  LOGE("%s Must be overload\n",__FUNCTION__);
  return -1;
}
//
// Read the data point to by adr into rdPat and compare
//
int memBaseTest_CleanReadBuf(memBaseTest_t *me) {
  int i;
  if (me->mCleanRdBuf) {
    for (i=0;i<16;i++) {
      me->rdPat[i] = 0xDEADBEEF;
    }
  }
  return 0;
}

int memBaseTest_ReadEntryNCompare(memBaseTest_t *me, uint64_t adr) {
  
  int errCnt = 0;
  int bitErrCnt = 0;
  // 
  errCnt += (*me->CleanReadBuf_p)(me);
  errCnt += (*me->ReadEntry_p)(me,adr); // read data into rdPat
  
  // now do the compare
  bitErrCnt = (*me->notEqual_p)(me);
  // retry
  if (bitErrCnt && (me->mRetryCnt > 0)) { // read one more time
    (*me->Print_p)(me,"ERR#1:",adr,3, 0);
    LOGI("%s:retrying the read\n",__FUNCTION__);
    errCnt = (*me->ReadEntry_p)(me,adr); // read data into rdPat
    bitErrCnt = (*me->notEqual_p)(me);
  }
  if (bitErrCnt) {
    (*me->Print_p)(me,"ERROR:",adr,3, 0);
    return 1;
  } else {
    if (GET_VERBOSE(me)) {
      (*me->Print_p)(me,"OK:",adr,1, 1);
    }
    return 0;
  }
}

// compare two patterns
int memBaseTest_notEqual(memBaseTest_t *me) {
  int errCnt = 0;
  int i,b;
  for (i=0;i<16;i++) {
    if ((me->wrPat[i] ^ me->rdPat[i]) & me->mskPat[i]) {
      for (b=0;b<32;b++) {
	if ((me->wrPat[i] ^ me->rdPat[i]) & me->mskPat[i] & (1 << b)) {
	  errCnt++;
	}
      }
    }
  }
  return errCnt;
}
//
// Set functions
//
void memBaseTest_Set1Pattern(memBaseTest_t *me, int bitPos) {
  // clear the pattern to 0
  int i;
  int whichWord = bitPos >> 5; // which word contain the 1 bit
  int whichBit  = bitPos & 0x1F; // which bit within the selected word
  for (i=0;i<16;i++) me->wrPat[i] = 0;
  me->wrPat[whichWord] = (1 << whichBit);
}
void memBaseTest_Set0Pattern(memBaseTest_t *me, int bitPos) {
  // clear the pattern to 0
  int i;
  int whichWord = bitPos >> 5; // which word contain the 1 bit
  int whichBit  = bitPos & 0x1F; // which bit within the selected word
  for (i=0;i<16;i++) me->wrPat[i] = 0xFFFFFFFF;
  me->wrPat[whichWord] = ~(1 << whichBit);
}
void memBaseTest_SetPattern(memBaseTest_t *me, uint64_t *pat2set) {
  // clear the pattern to 0
  int i;
  for (i=0;i<16;i++) me->wrPat[i] = *(pat2set+i);
}
uint64_t memBaseTest_SetIncPattern(memBaseTest_t *me, uint64_t startPat) {
  int i;
  for (i=0;i<16;i++) me->wrPat[i] = startPat++;
  return startPat;
}

void memBaseTest_RepPattern(memBaseTest_t *me, uint64_t pat) {
  // clear the pattern to 0
  int i;
  for (i=0;i<16;i+=1) { me->wrPat[i] = (i & 0x1) ? ~pat : pat; }
}
void memBaseTest_InvPattern(memBaseTest_t *me) {
  // clear the pattern to 0
  int i;
  for (i=0;i<16;i++) me->wrPat[i] = ~me->wrPat[i] ;
}
void memBaseTest_XORPattern(memBaseTest_t *me, int bitPos) {
  int i;
  int whichWord = bitPos >> 5; // which word contain the 1 bit
  int whichBit  = bitPos & 0x1F; // which bit within the selected word
  for (i=0;i<16;i++) {
    if (i == whichWord)  me->wrPat[i] = me->wrPat[i] ^ (1 << whichBit);
  }
}

void memBaseTest_Random48_srand48(memBaseTest_t *me, uint64_t seed) {
  me->Random48_customDiagRandomNumber = seed;
}
void memBaseTest_SetRandomPattern(memBaseTest_t *me, uint64_t seed) {
  int sidx = (seed + me->mSeed) % 15;
  // fill up the array startig from sidx
  int i;
  memBaseTest_Random48_srand48(me,seed + me->mSeed);
  //LOGI("seed=%x/%x\n",seed,me->mSeed);
  for (i=0;i<16;i++) {
    me->wrPat[(sidx+i)&0xf] = (uint64_t) Random48_mrand48_custom(&(me->Random48_customDiagRandomNumber));
  }
}


//
// Helper functions
// ..., wrPat[1] = [63:32], wrPat[0] = [31:0]
//
void memBaseTest_Print(memBaseTest_t *me, const char *str, uint64_t adr, int printMask, int withAdr) {
  int wrdSize = (me->mDatSize >> 5) + (((me->mDatSize & 0x1f) != 0)  ? 1 : 0);
  if (withAdr) {
      switch (wrdSize) {
      case 1:  LOGI("%s adr=%016lx Act=%016lx\n",str,adr,me->rdPat[0] & me->mskPat[0]); break;
      case 2:  LOGI("%s adr=%016lx Act=%016lx_%016lx\n",str,adr,me->rdPat[1] & me->mskPat[1],me->rdPat[0]); break;
      case 3:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx\n",str,adr,me->rdPat[2] & me->mskPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 4:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx\n",str,adr,me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 5:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 6:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 7:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->rdPat[6],me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 8:  LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->rdPat[7],me->rdPat[6],me->rdPat[5],me->rdPat[4],
		    me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      default:LOGI("%s adr=%016lx Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx...\n",
		   str,adr,me->rdPat[7],me->rdPat[6],me->rdPat[5],me->rdPat[4],
		   me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      }
  } else {
    // print write pattern
    if (printMask & 0x1) {
      switch (wrdSize) {
      case 1:  LOGI("%s adr=%016lx Exp=%016lx\n",str,adr,me->wrPat[0] & me->mskPat[0]); break;
      case 2:  LOGI("%s adr=%016lx Exp=%016lx_%016lx\n",str,adr,me->wrPat[1] & me->mskPat[1],me->wrPat[0]); break;
      case 3:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx\n",str,adr,me->wrPat[2] & me->mskPat[2],me->wrPat[1],me->wrPat[0]); break;
      case 4:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx\n",str,adr,me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      case 5:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->wrPat[4],me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      case 6:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->wrPat[5],me->wrPat[4],me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      case 7:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->wrPat[6],me->wrPat[5],me->wrPat[4],me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      case 8:  LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    str,adr,me->wrPat[7],me->wrPat[6],me->wrPat[5],me->wrPat[4],
		    me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      default:LOGI("%s adr=%016lx Exp=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx...\n",
		   str,adr,me->wrPat[7],me->wrPat[6],me->wrPat[5],me->wrPat[4],
		   me->wrPat[3],me->wrPat[2],me->wrPat[1],me->wrPat[0]); break;
      }
    }
    // print read pattern
    if (printMask & 0x2) {// must be bad
      switch (wrdSize) {
      case 1:  LOGE("\t\t    Act=%016lx\n",me->rdPat[0] & me->mskPat[0]); break;
      case 2:  LOGE("\t\t    Act=%016lx_%016lx\n",me->rdPat[1] & me->mskPat[1],me->rdPat[0]); break;
      case 3:  LOGE("\t\t    Act=%016lx_%016lx_%016lx\n",me->rdPat[2] & me->mskPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 4:  LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx\n",me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 5:  LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 6:  LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 7:  LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    me->rdPat[6],me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      case 8:  LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx\n",
		    me->rdPat[7],me->rdPat[6],me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      default: LOGE("\t\t    Act=%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx_%016lx...\n",
		    me->rdPat[7],me->rdPat[6],me->rdPat[5],me->rdPat[4],me->rdPat[3],me->rdPat[2],me->rdPat[1],me->rdPat[0]); break;
      }
    }
  }
}
void memBaseTest_setDatMsk(memBaseTest_t *me) {
  int i,j;
  uint64_t msk;
  int wrdSize = (me->mDatSize >> 6) + (((me->mDatSize & 0x3f) != 0) ? 1 : 0);
  int whichBit = me->mDatSize & 0x3f;
  for (i=0;i<16;i++) {
    if (i >= wrdSize)            me->mskPat[i] = 0;
    else if (i < (wrdSize-1))    me->mskPat[i] = (u_int64_t)-1;
    else {
      if (whichBit == 0) whichBit = 64; // exact 32bits
      msk = 0;
      for (j=0;j<whichBit;j++) msk = (msk << 1) | 0x1;
      me->mskPat[i] =  msk;
    }
  }
}

void memBaseTest_setAdrMsk(memBaseTest_t *me) {
  int i;
  me->adrMask = 0;
  for (i=0;i<me->mAdrSize;i++) { me->adrMask |= (me->adrMask << 1) | 0x1; }
}
void memBaseTest_setCheckerMsk(memBaseTest_t *me, int newMask) {
  me->mCheckerMask = newMask;
}

//
// memory Test functions
//
int memBaseTest_incPatTest(memBaseTest_t *me, int doWrite, int RdNCheck) {
  int errCnt = 0;
  int i;
  uint64_t curAdr;
  uint64_t startPat = me->mSeed;
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_incPatTest (starting seed=%08x) (wr=%d rdNchk=%d) mBAR=%016lx\n",
	 me->mSeed,doWrite,RdNCheck,me->mBAR);
  }
  if (doWrite) { // doing a write
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = (1 << i) & me->adrMask;
      startPat = (*me->SetIncPattern_p)(me,startPat);
      // walk a 1 thru adr
      errCnt += (*me->WriteEntry_p)(me,curAdr);
    }
  }
  if (RdNCheck) {
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = (1 << i) & me->adrMask;
      startPat = (*me->SetIncPattern_p)(me,startPat);
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  return errCnt;
}
  
int memBaseTest_walk_1_thruAdrTest(memBaseTest_t *me, int doWrite, int RdNCheck) {
  int errCnt = 0;
  int i;
  uint64_t curAdr;
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_walk_1_thruAdrTest (wr=%d rdNchk=%d) mBAR=%016lx\n",doWrite,RdNCheck,me->mBAR);
  }
  if (doWrite) { // doing a write
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = (1 << i) & me->adrMask;
      (*me->SetRandomPattern_p)(me,me->mBAR | curAdr); // use curAdr as seed
      // walk a 1 thru adr
      errCnt += (*me->WriteEntry_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  if (RdNCheck) {
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = (1 << i) & me->adrMask;
      (*me->SetRandomPattern_p)(me,me->mBAR | curAdr); // use curAdr as seed
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  return errCnt;
}
int memBaseTest_walk_0_thruAdrTest(memBaseTest_t *me, int doWrite, int RdNCheck) {
  int errCnt = 0;
  int i;
  uint64_t curAdr;
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_walk_0_thruAdrTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  if (doWrite) { // doing a write
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = ~(1 << i) & me->adrMask;
      (*me->SetRandomPattern_p)(me,me->mBAR | curAdr); // use curAdr as seed
      // walk a 1 thru adr
      errCnt += (*me->WriteEntry_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  if (RdNCheck) {
    for (i=0;i<me->mAdrSize;i++) {
      // first set random address base on adr
      curAdr = ~(1 << i) & me->adrMask;
      (*me->SetRandomPattern_p)(me,me->mBAR | curAdr); // use curAdr as seed
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  return errCnt;
}
int memBaseTest_GetMaxDataSize(memBaseTest_t *me) {
  if ((1 << me->mAdrSize) >= (me->mDatSize * 6)) return me->mDatSize;
  else return ((1 << me->mAdrSize)/6);
}

int memBaseTest_walk_1_thruDatTest(memBaseTest_t *me, int doWrite, int RdNCheck) {
  int errCnt = 0;
  int i;
  uint64_t curAdr;
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_walk_1_thruDatTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  if (doWrite) { // doing a write
    for (i=0;i<(*me->GetMaxDataSize_p)(me);i++) {
      // first set random address base on adr
      curAdr = me->adrMask - (i * 3);
      (*me->Set1Pattern_p)(me,i);
      // walk a 1 thru data
      errCnt += (*me->WriteEntry_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  if (RdNCheck) {
    for (i=0;i<(*me->GetMaxDataSize_p)(me);i++) {
      // first set random address base on adr
      curAdr = me->adrMask - (i *3);
      (*me->Set1Pattern_p)(me,i);
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  return errCnt;
}
int memBaseTest_walk_0_thruDatTest(memBaseTest_t *me, int doWrite, int RdNCheck) {
  int errCnt = 0;
  int i;
  uint64_t curAdr;
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_walk_0_thruDatTest (wr=%d rdNchk=%d)\n",doWrite,RdNCheck);
  }
  if (doWrite) { // doing a write
    for (i=0;i<(*me->GetMaxDataSize_p)(me);i++) {
      // first set random address base on adr
      curAdr = (i * 3) & me->adrMask;
      (*me->Set0Pattern_p)(me,i);
      // walk a 1 thru data
      errCnt += (*me->WriteEntry_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  if (RdNCheck) {
    for (i=0;i<(*me->GetMaxDataSize_p)(me);i++) {
      // first set random address base on adr
      curAdr = (i *3) & me->adrMask;
      (*me->Set0Pattern_p)(me,i);
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  thread_timed_yield();
  return errCnt;
}

int memBaseTest_checkerBoardTest(memBaseTest_t *me) {
  int errCnt = 0;
  int i;
  int lastLoc=0;
  int p = 0; // pattern count
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_checkerBoardTest (Step=%x)\n", me->mStep);
  }
  //
  // Write pattern adr=dat
  //
  errCnt += (*me->incPatRangeTest_p)(me,0,(1<<me->mAdrSize)-1,1,0);
  // readback and compare
  errCnt += (*me->incPatRangeTest_p)(me,0,(1<<me->mAdrSize)-1,0,1);
  if (errCnt >= me->mStopOnMaxErr) return errCnt;
  thread_timed_yield();
  p++;
  //
  // Write 0x55555555,0x55555555 from top -> bottom
  for (i= 0;i< (1<<me->mAdrSize);i +=me->mStep) {
    (*me->RepPattern_p)(me,0xAAAAAAAA);  if (i & 0x1) (*me->InvPattern_p)(me);
    lastLoc  = i;
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }

  // Read back to check and write 0xAA...AA into it (bottom -> top)
  for (i= lastLoc;i >= 0;i -=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0xAAAAAAAA);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
    (*me->RepPattern_p)(me,0x55555555);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << (p+1)) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }
  p++;
  // start from top-> bottom with 0xFF..FF
  for (i= 0;i< (1<<me->mAdrSize);i +=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0x55555555);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
    (*me->RepPattern_p)(me,0xFFFFFFFF);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << (p+1)) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }  
  p++;
  // start from bottom -> top with 0x33..33
  for (i= lastLoc;i >= 0;i -=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0xFFFFFFFF);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
    (*me->RepPattern_p)(me,0x33333333);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << (p+1)) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }  
  p++;
  // start from top-> bottom with 0xCC..CC
  for (i= 0;i< (1<<me->mAdrSize);i +=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0x33333333);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
    (*me->RepPattern_p)(me,0xCCCCCCCC);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << (p+1)) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }  
  p++;
  // start from bottom -> top with 0x00..00
  for (i= lastLoc;i >= 0;i -=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0xCCCCCCCC);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
    (*me->RepPattern_p)(me,0x00000000);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << (p+1)) & me->mCheckerMask) errCnt += (*me->WriteEntry_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }    
  p++;
  // start from top-> bottom and check for 0x00
  for (i= 0;i< (1<<me->mAdrSize);i +=me->mStep) {
    // check and write back
    (*me->RepPattern_p)(me,0x00000000);  if (i & 0x1) (*me->InvPattern_p)(me);
    if ((1 << p) & me->mCheckerMask) errCnt += (*me->ReadEntryNCompare_p)(me,i);
    if (errCnt >= me->mStopOnMaxErr) return errCnt;
  }   
  return errCnt;
}
//
// Mother of all tests
//
int memBaseTest_doMemTest(memBaseTest_t *me) {
  int errCnt = 0;
  if(GET_VERBOSE(me)) {
    LOGI("== Starting memBaseTest_doMemTest\n");
  }
  //
  // Walking 1 & 0 tests (Write Only)
  //
  errCnt += (*me->walk_1_thruAdrTest_p)(me,1,0);
  errCnt += (*me->walk_1_thruDatTest_p)(me,1,0);
  errCnt += (*me->walk_0_thruAdrTest_p)(me,1,0);
  errCnt += (*me->walk_0_thruDatTest_p)(me,1,0);
  //
  // Read & Check 1 & 0 tests
  //
  errCnt += (*me->walk_1_thruAdrTest_p)(me,0,1);
  if (errCnt >= me->mStopOnMaxErr) return errCnt;
  errCnt += (*me->walk_1_thruDatTest_p)(me,0,1);
  if (errCnt >= me->mStopOnMaxErr) return errCnt;
  errCnt += (*me->walk_0_thruAdrTest_p)(me,0,1);
  if (errCnt >= me->mStopOnMaxErr) return errCnt;
  errCnt += (*me->walk_0_thruDatTest_p)(me,0,1);
  if (errCnt >= me->mStopOnMaxErr) return errCnt;
  //
  // Finally checker board test
  //
  errCnt += (*me->checkerBoardTest_p)(me);
  if(GET_VERBOSE(me)) {
    LOGI("== Done memBaseTest_doMemTest\n");
  }
  return errCnt;

}
//
// For debug
//

int memBaseTest_incPatRangeTest(memBaseTest_t *me, uint64_t startAdr, uint64_t endAdr,int doWrite, int RdNCheck) {
  int errCnt = 0;
  uint64_t i;
  uint64_t curAdr;
  //uint64_t startPat = me->mSeed;
  int wrdSize = (me->mDatSize >> 5) + (((me->mDatSize & 0x1f) != 0)  ? 1 : 0);
  if(GET_VERBOSE(me)) {
    LOGI("== memBaseTest_incPatRangeTest (starting seed=%08x) (wr=%d rdNchk=%d) range=%016lx - %016lx step=%d\n",
	 me->mSeed,doWrite,RdNCheck,startAdr,endAdr,me->mStep);
  }
  if (doWrite) { // doing a write
    for (i=startAdr;i<=endAdr;i=i+me->mStep) {
      // first set random address base on adr
      curAdr = i;
      if (i & 0x1) {
	(void)(*me->SetIncPattern_p)(me,~(curAdr*wrdSize));
      } else {
	(void)(*me->SetIncPattern_p)(me,curAdr*wrdSize);
      }
      // walk a 1 thru adr
      errCnt += (*me->WriteEntry_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  if (RdNCheck) {
    for (i=startAdr;i<=endAdr;i=i+me->mStep) {
      // first set random address base on adr
      curAdr = i;
      if (i & 0x1) {
	(void)(*me->SetIncPattern_p)(me,~(curAdr*wrdSize));
      } else {
	(void) (*me->SetIncPattern_p)(me,curAdr*wrdSize);
      }
      // walk a 1 thru adr
      errCnt += (*me->ReadEntryNCompare_p)(me,curAdr);
      if (errCnt >= me->mStopOnMaxErr) return errCnt;
    }
  }
  
  return errCnt;
}
// dump
int memBaseTest_dump(memBaseTest_t *me, uint64_t startAdr, uint64_t endAdr) {
  int errCnt = 0;
  uint64_t i;
  for (i=startAdr;i<=endAdr;i++) {
    errCnt += (*me->CleanReadBuf_p)(me);
    errCnt = (*me->ReadEntry_p)(me,i);
    // print
    (*me->Print_p)(me,"",i,2, 1);    
  }
  return errCnt;
}


