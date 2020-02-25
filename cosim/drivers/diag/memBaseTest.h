//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef memBaseTest_H
#define memBaseTest_H
// ---------------------------

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
//#include <sys/types.h>

#ifdef BARE_MODE
#include "bare_malloc.h"
#define my_malloc bare_malloc
#define my_free   bare_free

#else
#define my_malloc malloc
#define my_free   free
#endif


//
// ==================================
// Register base test converted from C++ class
// ==================================
//
typedef struct memBaseTest_ memBaseTest_t;
//
// Constructor/Destructor
//
#define MEMBASETEST_CREATE(me,baseAdr,adrSize,datSize,step,seed)	\
  { me=(memBaseTest_t *)my_malloc(sizeof(memBaseTest_t));		 \
    if (me==NULL) return(1);					 \
    me->Construct_p = memBaseTest_Construct;			 \
    (*me->Construct_p)(me,baseAdr,adrSize,datSize,step,seed);}

#define MEMBASETEST_DELETE(me) my_free(me)

 
struct memBaseTest_ {
  //
  // Constructor/Destructors
  //
  void (*Construct_p)         (memBaseTest_t *me,
			       uint64_t baseAdr,
			       int adrSize,
			       int datSize,
			       int step,
			       int seed); 
  //
  // Function Pointers/ Methods
  //
  int (*CleanReadBuf_p)		(memBaseTest_t *) ;
  int (*WriteEntry_p)		(memBaseTest_t *, uint64_t ) ;
  int (*ReadEntry_p)		(memBaseTest_t *, uint64_t ) ;
  int (*ReadEntryNCompare_p)	(memBaseTest_t *, uint64_t ) ;
  int (*notEqual_p)		(memBaseTest_t *) ;
  void (*Set1Pattern_p)		(memBaseTest_t *, int ) ;
  void (*Set0Pattern_p)		(memBaseTest_t *, int ) ;
  void (*SetPattern_p)		(memBaseTest_t *, uint64_t *) ;
  uint64_t (*SetIncPattern_p)	(memBaseTest_t *, uint64_t ) ;
  void (*RepPattern_p)		(memBaseTest_t *, uint64_t ) ;
  void (*InvPattern_p)		(memBaseTest_t *) ;
  void (*XORPattern_p)		(memBaseTest_t *, int ) ;
  void (*SetRandomPattern_p)	(memBaseTest_t *, uint64_t ) ;
  void (*Print_p)		(memBaseTest_t *, const char *, uint64_t , int, int ) ;
  void (*setDatMsk_p)		(memBaseTest_t *) ;
  void (*setAdrMsk_p)		(memBaseTest_t *) ;
  void (*setCheckerMsk_p)	(memBaseTest_t *, int) ;
  int (*incPatTest_p)		(memBaseTest_t *, int , int ) ;
  int (*incPatRangeTest_p)	(memBaseTest_t *, uint64_t , uint64_t, int , int ) ;
  int (*walk_1_thruAdrTest_p)	(memBaseTest_t *, int , int ) ;
  int (*walk_0_thruAdrTest_p)	(memBaseTest_t *, int , int ) ;
  int (*GetMaxDataSize_p)	(memBaseTest_t *) ;
  int (*walk_1_thruDatTest_p)	(memBaseTest_t *, int , int ) ;
  int (*walk_0_thruDatTest_p)	(memBaseTest_t *, int , int ) ;
  int (*checkerBoardTest_p)	(memBaseTest_t *) ;
  int (*doMemTest_p)		(memBaseTest_t *) ;
  int (*dump_p)	                (memBaseTest_t *, uint64_t , uint64_t);
  //
  void memBaseTest_Random48_srand48(memBaseTest_t *me, uint64_t seed);
  //
  // members
  //
  int mBrdId;
  int mId;
  int mTarget;
  int mInterface;
  int mPri;
  int mSubTarget;
  uint64_t mBAR; // base address to access MEM IF register
  int mAdrSize;
  int mDatSize;
  int mStep;
  int mSeed;
  int mStopOnMaxErr;
  int mCheckerMask;
  int mCleanRdBuf;
  int mRetryCnt;
  uint64_t wrPat[16]; // upto 512 bit wide
  uint64_t rdPat[16];
  //
  uint64_t mskPat[16];
  uint64_t adrMask;
  // private functions
  int mLogLevelMask;

  // need it own random since this can be run under  multicore bare metal
  uint64_t Random48_customDiagRandomNumber;
};



//
#endif
