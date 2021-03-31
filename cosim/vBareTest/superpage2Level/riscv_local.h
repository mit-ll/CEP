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
// How the page table should be setup for this test (2 levels)
//
// 1 level to vpn0 = level2 instead
//
// Direct map vpn = ppn upt 1G
//
#define IDX_VPN0        level1
// should never need to do this
#define BYTE2COPY       128

// only 1 levels
#define LEAF_ENTRY_PTR vpn1pt[level1]
// no need to use free list
#define SETUP_FREE_LIST 0

// See PTW.scala
/*
     // for superpage mappings, make sure PPN LSBs are zero
     for (i <- 0 until pgLevels-1)
        when (count <= i && tmp.ppn((pgLevels-1-i)*pgLevelBits-1, (pgLevels-2-i)*pgLevelBits) =/= 0) { res.v := false }
    }

    This means, there unused PPN[x] are UN-touchable because they need to be zeros!!!! Comment these lines out, we can load any values in those unsued PPN[x]
    field of the PTE entries.
 */

//
#define NO_PMP
#define UNUSED_PPN_BITS (0x0<<12)

// only 37:0 of VPN are tested..vpn2[3:0]
// for 2 level ppn[0] = vpn[0] so e can full gargage with pte's ppn[0] = (0x1ff<<12)
//
#define SETUP_PAGE_TABLE  do {					\
    for (int i=0;i<16;i++) {						\
      vpn2pt[i] = ((pte_t)vpn1pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;	\
    } \
    for (int i=0;i<PTES_PER_PT;i++) {						\
      ker1pt[i] = (((DRAM_BASE | (i*MEGAPAGE_SIZE)) >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D; \
      vpn1pt[i] = (((DRAM_BASE | (i*MEGAPAGE_SIZE) | UNUSED_PPN_BITS) >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_U; \
    }	\
    vpn2pt[PTES_PER_PT-1] = ((pte_t)ker1pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V; \
    __builtin___clear_cache(0,0); \
  } while (0)



#endif
