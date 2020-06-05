//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef REGBASETEST_H
#define REGBASETEST_H
// ---------------------------

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

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
typedef struct regBaseTest_ regBaseTest_t;
//
// Constructor/Destructor
//
#define REGBASETEST_CREATE(me,target,accessSize,seed,verbose)		\
  { me=(regBaseTest_t *)my_malloc(sizeof(regBaseTest_t));		\
    if (me==NULL) return(1);						\
    me->Construct_p = regBaseTest_Construct;				\
    (*me->Construct_p)(me,target,accessSize,seed,verbose);  }

#define REGBASETEST_DELETE(me) my_free(me)


struct regBaseTest_ {
  //
  // Constructor/Destructors
  //
  void (*Construct_p)         (regBaseTest_t *me,
			       int target,
			       int accessSize,
			       int seed,
			       int verbose);
  int (*SetSeed_p)              (regBaseTest_t *, int);
  //
  // Function Pointers/ Methods
  //
  void (*ClearAll_p)             (regBaseTest_t *);
  int (*AddAReg_p)              (regBaseTest_t *, uint32_t, uint64_t);
  int (*AddROReg_p)             (regBaseTest_t *,uint32_t, uint64_t, uint64_t );
  int (*GetMaxBits_p)           (regBaseTest_t *, uint64_t);
  int (*WriteReg_p)             (regBaseTest_t *,uint32_t,uint64_t);
  uint64_t (*ReadReg_p)         (regBaseTest_t *,uint32_t);
  int (*ReadRegNCompare_p)      (regBaseTest_t *,uint32_t, uint64_t, uint64_t);
  int (*uniquifyTest_p)         (regBaseTest_t *,int, int);
  int (*walk_1_thruDatTest_p)   (regBaseTest_t *,int, int);
  int (*walk_0_thruDatTest_p)   (regBaseTest_t *,int, int);
  int (*checkerBoardTest_p)     (regBaseTest_t *);
  int (*doRORegTest_p)          (regBaseTest_t *);
  int (*doRegTest_p)            (regBaseTest_t *);
  void (*SetNoWr2ROreg_p)        (regBaseTest_t *,int);
  void (*SetAccessSize_p)        (regBaseTest_t *,int);
  void (*Srand48_p)              (regBaseTest_t *,uint64_t);

  //
  // Member
  //
  int mTarget;
  // members
  int mVerbose;
  int mAccessSize;
  int mRegCnt;
  int mStep;
  int mSeed;
  int mMaxBits;
  uint64_t mRegList[0x1000*2]; 
  int mROcnt;
  int mNoWr2ROreg;
  uint64_t mROList[0x1000*3]; 
  int mLogLevelMask;
  // random
  uint64_t customDiagRandomNumber;
  
};

//
#endif
