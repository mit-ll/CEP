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
    if (vpn0pt[page_adr]) { // assuming they are all unique!!!
        if (!(vpn0pt[page_adr] & PTE_A)) {
            //CLR_UNUSED_PPN;
            vpn0pt[page_adr] |= PTE_A;
        } else {
            //FIXME!!! assert(!(vpn0pt[index] & PTE_D) && cause == CAUSE_STORE_PAGE_FAULT);
            vpn0pt[page_adr] |= PTE_D;
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
    uintptr_t ppn_addr = 0xBFFFF000; // data
    //
    if ((level2 == 0) && (level1 == 0)) { // code
         ppn_addr = DRAM_BASE + (level1 << (9+PGSHIFT)) + ( page_adr << PGSHIFT);
    }
    uintptr_t new_pte = ((ppn_addr >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_U | PTE_R | PTE_W | PTE_X;
    //
    vpn0pt[page_adr] = new_pte | PTE_A | PTE_D;
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
    //vpn0pt[page_adr] = new_pte; 
    //flush_page(addr);
    //
    //__builtin___clear_cache(0,0);
}


#endif
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
    if (vpn0pt[page_adr]) { // assuming they are all unique!!!
        if (!(vpn0pt[page_adr] & PTE_A)) {
            //CLR_UNUSED_PPN;
            vpn0pt[page_adr] |= PTE_A;
        } else {
            //FIXME!!! assert(!(vpn0pt[index] & PTE_D) && cause == CAUSE_STORE_PAGE_FAULT);
            vpn0pt[page_adr] |= PTE_D;
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
    uintptr_t ppn_addr = 0xBFFFF000; // data
    //
    if ((level2 == 0) && (level1 == 0)) { // code
         ppn_addr = DRAM_BASE + (level1 << (9+PGSHIFT)) + ( page_adr << PGSHIFT);
    }
    uintptr_t new_pte = ((ppn_addr >> PGSHIFT) << PTE_PPN_SHIFT) | PTE_V | PTE_U | PTE_R | PTE_W | PTE_X;
    //
    vpn0pt[page_adr] = new_pte | PTE_A | PTE_D;
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
    //vpn0pt[page_adr] = new_pte; 
    //flush_page(addr);
    //
    //__builtin___clear_cache(0,0);
}


#endif
