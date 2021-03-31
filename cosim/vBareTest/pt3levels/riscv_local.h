//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#ifndef riscv_local_H
#define riscv_local_H

//
// How the page table should be setup for this test (3 levels)
//
#define SETUP_PAGE_TABLE  do {					\
for (int i=0;i<512;i++) {							\
  vpn2pt[i] = ((pte_t)vpn1pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;	\
  vpn1pt[i] = ((pte_t)vpn0pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;	\
  ker1pt[i] = (((DRAM_BASE+(i*MEGAPAGE_SIZE)) >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D; \
 }									\
vpn2pt[PTES_PER_PT-1] = ((pte_t)ker1pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V; \
} while (0)

//
// How to get the virtual -> PA 
//
//use default

#endif
