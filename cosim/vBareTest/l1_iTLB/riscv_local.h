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

/*
#define l1pt        pt[0]
#define user_l2pt   pt[1]
#define kernel_l2pt pt[2]
#define user_llpt   pt[3]

#define vpn2pt pt[cid][0]
#define vpn1pt pt[cid][1]
#define ker1pt pt[cid][2]
#define vpn0pt pt[cid][3]
*/
//
// How the page table should be setup for this test (3 levels)
//
#define SETUP_PAGE_TABLE  do {					\
for (int i=0;i<512;i++) {							\
  vpn2pt[i] = ((pte_t)vpn0pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;	\
  ker1pt[i] = (((DRAM_BASE+(i*MEGAPAGE_SIZE)) >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D; \
 }									\
 vpn2pt[PTES_PER_PT-1] = ((pte_t)ker1pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V; \
} while (0)

//
// How to get the virtual -> PA 
//
//use default
#define NO_PMP
#define SETUP_FREE_LIST 0
#define FAULT_MGR handle_fault_giga
void handle_fault_giga(uintptr_t addr, uintptr_t cause)
{
    //
    // local 
    // see entry.S under drivers/virtual directory
    // 
    // mhartid is NOT readable under supervisor mode!!!
    //
    int cid = read_csr(scounteren) & 3;	
    
    //addr = addr/PGSIZE*PGSIZE;
    // align to 4K page
    addr = (addr >> PGSHIFT) << PGSHIFT;
    //
    //
    // check which level to add this new_pte
    //
    uintptr_t level2   = (addr >> (9+9+PGSHIFT)) & (PTES_PER_PT-1); // gigapage
    uintptr_t level1   = (addr >> (9+PGSHIFT))   & (PTES_PER_PT-1); // mega page
    uintptr_t page_adr = (addr >> PGSHIFT)       & (PTES_PER_PT-1); // level0

    //
    // a hits on user space?
    //
    if (vpn0pt[level1]) { // assuming they are all unique!!!
        // page_adr = 0x1FF will cause bad virtual address !!!
        if (level2 != 0) {
            vpn0pt[level1] &= ~(0x1FF << PTE_PPN_SHIFT); // zero out page_address
        }
        //
        if (!(vpn0pt[level1] & PTE_A)) {
            //CLR_UNUSED_PPN;
            vpn0pt[level1] |= PTE_A;
        } else {
            //FIXME!!! assert(!(vpn0pt[index] & PTE_D) && cause == CAUSE_STORE_PAGE_FAULT);
            vpn0pt[level1] |= PTE_D;
            // mis-align store?
            // misalign load? 
        }
        flush_page(addr);
        return;
    }
    //
    // fill the next one with same page
    //
    //
    uintptr_t ppn_addr = 0xBFFFF000 ; // | ((level2 << 12) & 0xf); // data 
    //
    if (level2 == 0) { // code
        ppn_addr = DRAM_BASE + (level1 << (9+PGSHIFT)) ; // + ( page_adr << PGSHIFT);
    }
    uintptr_t new_pte = ((ppn_addr >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_U | PTE_R | PTE_W | PTE_X ;
    //
    vpn0pt[level1] = new_pte | PTE_A | PTE_D;
    flush_page(addr);
    
    uintptr_t sstatus = set_csr(sstatus, SSTATUS_SUM);

#if 0
    if (level2 == 0) { // within 1 G
        memcpy((void*)addr, userVA2kernelVA(addr), 128); // dont need to copy the whole page!!!! since most of them are NOPs
    } 
#endif
    //
    write_csr(sstatus, sstatus);
    //
    //vpn0pt[level1] = new_pte; 
    //flush_page(addr);
    //
    __builtin___clear_cache(0,0);
}


#endif
