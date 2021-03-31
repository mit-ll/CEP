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

#define EXTRA_CAUSE_TRAP 0
#define SETUP_PAGE_TABLE 0

//
//
//
#define VM_BOOT_MGR vm_boot_giga
void vm_boot_giga(uintptr_t test_addr)
{
  int cid = read_csr(mhartid); // ok to read mhartid here
  //
  _Static_assert(SIZEOF_TRAPFRAME_T == sizeof(trapframe_t), "???");

  //
  // user code
  //

  //
  //
  //
  uintptr_t vm_choice = SATP_MODE_CHOICE;
  uintptr_t sptbr_value = ((uintptr_t)vpn2pt >> PGSHIFT)
                        | (vm_choice * (SATP_MODE & ~(SATP_MODE<<1)));
  write_csr(sptbr, sptbr_value);
  if (read_csr(sptbr) != sptbr_value)
    assert(!"unsupported satp mode");

  // Set up PMPs if present, ignoring illegal instruction trap if not.
  // setup all 8 PMP within 4G memory space: each is 512MG
  // load CSR_PMPCFG0 with
  // bit[7] = Lock
  // bit[6:5] = -NA-
  // [4:3] = A =
  //     0 : disable
  //     1 : TOR: top-of-range
  //     2 : NA4
  //     3 : NAPOT
  // [2] = X
  // [1] = W
  // [0] = R
  uintptr_t pmpc = 0x7F7F7F7F7F7F7F7FLL; // all 8 with all access except not locked
  uintptr_t pmpa[8];
  //      3          2         1
  //   3210987654321098765432109876543210
  //   xxx0111111111111111111111111111111
  // xxx = i
  // bit 33:0 of the virtual address!! not physical!!
  //
  for (int i=0;i<8;i++) {
    //    pmpa[i] = ((i << 29) | ((1<<28)-1)) >> 2; // bit[31:2] only
    pmpa[i] = ((i << 31) | ((1<<30)-1)) >> 2; // bit[31:2] only
  }
  //
  asm volatile ("la t0, 1f\n\t"
                "csrrw t0, mtvec, t0\n\t"
                "csrw pmpaddr0, %1\n\t"
                "csrw pmpaddr1, %2\n\t"
                "csrw pmpaddr2, %3\n\t"
                "csrw pmpaddr3, %4\n\t"
                "csrw pmpaddr4, %5\n\t"
                "csrw pmpaddr5, %6\n\t"
                "csrw pmpaddr6, %7\n\t"
                "csrw pmpaddr7, %8\n\t"
                "csrw pmpcfg0,  %0\n\t"
                ".align 2\n\t"
                "1: csrw mtvec, t0"
                : : "r" (pmpc), 
		  "r" (pmpa[0]),  
		  "r" (pmpa[1]),  
		  "r" (pmpa[2]),  
		  "r" (pmpa[3]),  
		  "r" (pmpa[4]), 
		  "r" (pmpa[5]),  
		  "r" (pmpa[6]),  
		  "r" (pmpa[7]) : 
		  "t0");
  //
  // set up supervisor trap handling
  //
  write_csr(stvec, userPA2KVA(trap_entry));
  write_csr(sscratch, userPA2KVA(read_csr(mscratch)));
  write_csr(medeleg,
	    EXTRA_CAUSE_TRAP |
	    (1 << CAUSE_USER_ECALL) |
	    (1 << CAUSE_FETCH_PAGE_FAULT) |
	    (1 << CAUSE_LOAD_PAGE_FAULT) |
	    (1 << CAUSE_STORE_PAGE_FAULT));
  // FPU on; accelerator on; vector unit on
  write_csr(mstatus, MSTATUS_FS | MSTATUS_XS | MSTATUS_VS);
  write_csr(mie, 0);
  //
  // NO free_list
  //

  //
  //
  trapframe_t tf[MAX_CORES];
  memset(&tf[cid], 0, sizeof(tf[0]));
  tf[cid].epc = test_addr - DRAM_BASE;
  pop_tf(&tf[cid]);
}


#endif
