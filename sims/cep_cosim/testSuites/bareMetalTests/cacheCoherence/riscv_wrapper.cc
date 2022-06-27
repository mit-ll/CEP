//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      riscv_wrapper.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------

// For bareMetal mode ONLY
#ifdef BARE_MODE
  #include <stdlib.h>
  #include "cep_adrMap.h"
  #include "cep_apis.h"
  #include "portable_io.h"
  
  #ifdef __cplusplus
  extern "C" {
  #endif

  #define flush_page(addr) asm volatile ("sfence.vma %0" : : "r" (addr) : "memory")

  // cache size is 64 bytes 
  #define BLOCK_SIZE_BITS 6
  #define BLOCK_SIZE (1 << BLOCK_SIZE_BITS)

  //int main(void)
  void thread_entry(int cid, int nc) {
    int errCnt = 0,i,to,b;
    int testId[4] = {0x00,0x11,0x22,0x33};
    int coreId = read_csr(mhartid);
    int partnerId = (coreId + 1) % MAX_CORES;
  
    // upto 4 for now
    uint64_t pattern[MAX_CORES] = {
      0x0102030405060708LL,   
      0x1112131415161718LL,   
      0x2122232425262728LL,
      0x3132333435363738LL
    };
    uint64_t volatile rd64;

    // pick a cache block, each one read/write to it with different 64-bits and offset
    // Also verify the pattern do the same.
    // this cache block should be bounced around the active cores a lot
    //
    // ON 8-bytes boundary (data bus width)
    //
    int max_blocks = 8;
    uint32_t my_base = 0x800E0000 + (coreId*8);
    uint32_t pn_base = 0x800E0000 + (partnerId*8);  
    uint32_t stat=0  ;
    
    set_printf(0);

    // Set the current core's status to running
    set_cur_status(CEP_RUNNING_STATUS);  

    // Write and read back (8-bytes aligned)
    for (i = 0; i < 8; i++) {

      // write to different block
      for (b = 0; b < max_blocks; b++) {
        DUT_WRITE32_64(my_base | (1 << (BLOCK_SIZE_BITS+b)) , pattern[coreId] + b);
        // Add this line to get test to pass on RHEL7/gcc-7 but may be we are hiding a bug???
        // flush_page(my_base | (1 << (6+b)));
      }
    
      // now read and wait we see the partners do the same
      for (b = 0; b < max_blocks; b++) {
        if (errCnt) { break; }      
        to = 1000;
        while (to > 0) {
          // check the pattern to make sure it get here
          DUT_READ32_64(pn_base + (1 << (BLOCK_SIZE_BITS+b)),rd64);

          if (rd64 != (pattern[partnerId]+b)) {
            to--;
          } else {
            break;
          }
        } // end while (to > 0)
      
        if (to <= 0) {
          errCnt++;
        }
      } // for (b = 0; b < max_blocks; b++)
    
      // Now check mine
      for (b = 0; b < max_blocks; b++) {
        if (errCnt) { break; }  
        DUT_READ32_64(my_base + (1 << (BLOCK_SIZE_BITS+b)),rd64);      
        if ((pattern[coreId]+b) != rd64) {
          stat = testId[coreId] | (i << 16);
          errCnt++;
        }
      } // end for (b = 0; b < max_blocks; b++)
    
      // next
      pattern[coreId]    += 0x0101010101010101LLU;
      pattern[partnerId] += 0x0101010101010101LLU;
      if (errCnt) break;
  
    } // end for (i = 0; i < 8; i++)
  
    stat = testId[coreId] | (i << 8)  | (to << 16) | (b << 24);  
  
    // Done
    set_status(errCnt,stat);
  
    // Exit with the error count
    exit(errCnt);
  }

  #ifdef __cplusplus
  }
  #endif
  
#endif // #ifdef BARE_MODE
