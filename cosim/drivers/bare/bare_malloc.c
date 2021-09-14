//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// Very simple/premitive maaloc to support malloc in bare metal
//
#include "cep_adrMap.h"
#include "encoding.h"
#include "bare_malloc.h"

//
//
//
bareMalloc_t malloc_st[MAX_CORES] __attribute__((aligned(64))) = {
  {.start_heap=(cep_malloc_heap_core0_start),                            .end_heap=(cep_malloc_heap_core0_start + cep_malloc_heap_core_size)},
  {.start_heap=(cep_malloc_heap_core0_start+cep_malloc_heap_core_size),  .end_heap=(cep_malloc_heap_core0_start + 2*cep_malloc_heap_core_size)},
  {.start_heap=(cep_malloc_heap_core0_start+2*cep_malloc_heap_core_size),.end_heap=(cep_malloc_heap_core0_start + 3*cep_malloc_heap_core_size)},
  {.start_heap=(cep_malloc_heap_core0_start+3*cep_malloc_heap_core_size),.end_heap=(cep_malloc_heap_core0_start + 4*cep_malloc_heap_core_size)},
} ;
				      

void* bare_malloc(uint32_t nbytes) {
  int coreId = read_csr(mhartid);;
  if (malloc_st[coreId].start_heap + nbytes >= malloc_st[coreId].end_heap)
        return 0;
  uint32_t temp = (uint32_t)malloc_st[coreId].start_heap;
  malloc_st[coreId].start_heap += nbytes;
  return (void*)(intptr_t)temp;
}

void bare_free(void *ptr) {
  // TODO: does nothing for now
  // assuming test never maaloc more then allocated
  return;
}
