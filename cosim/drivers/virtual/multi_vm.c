//
// See LICENSE for license details.
//
//
// Multi-core virtual/physical address mapping manager
// ONLY can be used for bare metal under simulation
//
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "riscv_test.h"

// SV39 ONLY!!!
# define SATP_MODE_CHOICE SATP_MODE_SV39


#define STACK_TOP (_end + 4096)

void trap_entry();
void pop_tf(trapframe_t*);
extern uint64_t vm_flags;

//
// Only tested for SATP_MODE_SV39
//
#define l1pt pt[cid][0]
#define user_l2pt pt[cid][1]
#define NPT 4
#define kernel_l2pt pt[cid][2]
#define user_llpt pt[cid][3]
#define MAX_CORES 4
//         
#define vpn2pt pt[cid][0]
#define vpn1pt pt[cid][1]
#define ker1pt pt[cid][2]
#define vpn0pt pt[cid][3]

pte_t pt[MAX_CORES][NPT][PTES_PER_PT] __attribute__((aligned(PGSIZE)));

typedef struct { pte_t addr; void* next; } freelist_t;

freelist_t freelist_nodes[MAX_CORES][MAX_TEST_PAGES];
freelist_t *freelist_head[MAX_CORES], *freelist_tail[MAX_CORES];

void test_fail(void);
void hangMe(void)
{
  test_fail();
  while (1);
}

#define assert(x) do { \
    if (x) break;      \
    hangMe();	       \
} while(0)

//
#define pa2kva(pa) ((void*)(pa) - DRAM_BASE - MEGAPAGE_SIZE)
#define uva2kva(pa) ((void*)(pa) - MEGAPAGE_SIZE)

//
// For GigaPage Only
//
#define userPA2KVA(pa) ((void*)(pa) - DRAM_BASE - GIGAPAGE_SIZE)
#define userVA2kernelVA(adr) ((void*)((adr & 0x3FFFFFFF) + 0xffffffffc0000000))

#define flush_page(addr) asm volatile ("sfence.vma %0" : : "r" (addr) : "memory")

#ifdef USE_LOCAL_VM
#include "riscv_local.h"
#endif


#ifndef SETUP_PAGE_TABLE
#define SETUP_PAGE_TABLE
#endif

#ifndef EXTRA_CAUSE_TRAP
#define EXTRA_CAUSE_TRAP 0
#endif

//
// Each core will get 256 MB of physical space (1G total)
//      3           2            1 
//     1098 7654 3210 9876 5432 1098 7654 3210
// 32'b10cc_xxxx_xxxM_MMMM_MMMM_PPPP_PPPP_PPPP (c=coreId,P=page adr = 4K, M=MegaPage=2Mbytes))
//
//       cc_10xx_xxxx_xxxx_xxxx : walk 1 14-bit
//       cc_11...               : walk 0
//      ~cc_11xx_xxxx_xxxx_xxxx : walk 1
//      ~cc_10xx_xxxx_xxxx_xxxx : walk 0
#ifndef GET_PA_ADR
#define GET_PA_ADR(i) do {						\
    if (i < 14) {							\
      mask =  (1 << i) & 0x3fff;					\
      pte_entry = DRAM_BASE | (cid << 28)  | (2 << 26) | (mask << 12) ;	\
    } else if (i < 28) {						\
      mask = ~(1 << (i-14)) & 0x3fff;					\
      pte_entry = DRAM_BASE | (cid << 28)  | (3 << 26) | (mask << 12) ;	\
    } else if (i < 42) {						\
      mask =  (1 << (i-28)) & 0x3fff;					\
      pte_entry = DRAM_BASE | (icid << 28) | (3 << 26) | (mask << 12) ;	\
    } else if (i < 56) {						\
      mask = ~(1 << (i-42)) & 0x3fff;					\
      pte_entry = DRAM_BASE | (icid << 28) | (2 << 26) | (mask << 12) ;	\
    } else {								\
      pte_entry = DRAM_BASE | (icid << 28) | (0xA<<21)  | (i << 12) ;   \
    }									\
    if (vm_flags & INVERT_MASK) {					\
      pte_entry = (~pte_entry & 0x3FFFF000) | (pte_entry & ~0x3FFFF000); \
    }									\
} while (0)
#endif

// Other level might override this
#ifndef IDX_VPN0
#define IDX_VPN0 page_adr
#endif

#ifndef BYTE2COPY
#define BYTE2COPY 128
#endif

#ifndef OVERRIDE_INDEX
#define OVERRIDE_INDEX()
#endif

// default is 3 levels look up
#ifndef LEAF_ENTRY_PTR
#define LEAF_ENTRY_PTR      vpn0pt[index]
#define NEXT_LEAF_ENTRY_PTR vpn0pt[index+1]
#endif

// default is setup dynamic freelist for dynamic table setup
#ifndef SETUP_FREE_LIST
#define SETUP_FREE_LIST 1
#endif

#ifndef CLR_UNUSED_PPN
#define CLR_UNUSED_PPN
#endif




//
//
//
//   VPN[2]   VPN[1]      VPN[0]    PageOffset
//         3           2          1
// 876543210_987654321_098765432_109876543210
//
// Kernel VA
//         3           2          1
// 876543210_987654321_098765432_109876543210
// 111111111 11111111x xxxxxxxxx xxxxxxxxxxxx
//
// Kernel PA (megaPage) -> Level1 (VPN[1])
//         3           2          1
//        10_987654321_098765432_109876543210
//        11_xxxxxxxxx MMMMMMMMM pppppppppppp
//
// User PA (regular page) VPN[0])
//
// Mapping conventions:
//  VPN[2] = 0 : user code level 2     -> VPN[1]
//           0x1FF : kernel level 2    -> KVPN[1]
//           0x1F7 : walk-1 next level -> VPN[1]
//           0x1FC : walk-0 next level -> VPN[1]
//           walk-1                    -> VPN[1]
//           walk-0                    -> VPN[1]
//
//  KVPN[1] =  0-0x1ff's kernel -> stop here (mega page)
//
//  VPN[1]  = 0     : user code level 1     -> VPN[0]
//            0x1F7 : walk-1 previous level -> VPN[0]
//            0x1FC : walk-0 previous level -> VPN[0]
//            walk-1                        -> VPN[0]
//            walk-0                        -> VPN[0]
//
//  VPN[0]  = 0 - 0x03F    : user code level 0
//            0x400 - 0x1FF: dynamic user code level 0
//           
//
// Acceptabe User's VA addressing:
//
//  VPN[2]     VPN[1]    VPN[0]   PageOffset
//         3           2          1
// 876543210_987654321_098765432_109876543210
// 111111111 mmmmmmmmm xxxxxxxxx pppppppppppp : Kernel code (megapage)
// 000000000 000000000 000UUUUUU pppppppppppp : user code (restricted to 64 pages = 256Kbytes)
// 000000011 wwwwwwwww 1011xxxxx pppppppppppp : walk-1 VPN[2] = pattern1
// 000000011 WWWWWWWWW 1100xxxxx pppppppppppp : walk-0 VPN[2] = pattern2
// wwwwwwwww 000000011 1101xxxxx pppppppppppp : walk-1 VPN[1] = pattern3
// WWWWWWWWW 000000011 1110xxxxx pppppppppppp : walk-0 VPN[1] = pattern4
//
// Only for simulation!!!
//
#ifndef VM_BOOT_MGR
#define VM_BOOT_MGR vm_boot_giga
void vm_boot_giga(uintptr_t test_addr)
{
  int cid = read_csr(mhartid); // ok to read mhartid here
  //
  _Static_assert(SIZEOF_TRAPFRAME_T == sizeof(trapframe_t), "???");

  //
  // user code
  //
  SETUP_PAGE_TABLE;

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
  //#define NO_PMP
#ifdef NO_PMP
  // Set up PMPs if present, ignoring illegal instruction trap if not.
  //  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  uintptr_t pmpc =  PMP_TOR | PMP_R | PMP_W | PMP_X;
  uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
  asm volatile ("la t0, 1f\n\t"
                "csrrw t0, mtvec, t0\n\t"
                "csrw pmpaddr0, %1\n\t"
                "csrw pmpcfg0, %0\n\t"
                ".align 2\n\t"
                "1: csrw mtvec, t0"
                : : "r" (pmpc), "r" (pmpa) : "t0");

#else

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
#endif

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

#if defined SETUP_FREE_LIST && SETUP_FREE_LIST == 1
  // if page tables are statically setup, should not ever get here!!!
  //
  // Map all pages to uniquely fit into 1G space of physical
  //
  //
  // Each core will get 256 MB of physical space (1G total)
  //      3           2            1 
  //     1098 7654 3210 9876 5432 1098 7654 3210
  // 32'b10cc_xxxx_xxxM_MMMM_MMMM_PPPP_PPPP_PPPP (c=coreId,P=page adr = 4K, M=MegaPage=2Mbytes))
  //
  //       cc_1xxx_xxxx_xxxx_xxxx : walk 1 15-bit
  //       cc_0...                : walk 0
  //      ~cc_1xxx_xxxx_xxxx_xxxx : walk 1
  //      ~cc_0xxx_xxxx_xxxx_xxxx : walk 0
  freelist_head[cid] = userPA2KVA((void*)&freelist_nodes[cid][0]);
  freelist_tail[cid] = userPA2KVA(&freelist_nodes[cid][MAX_TEST_PAGES-1]);
  int mask;
  pte_t pte_entry;
  //
  // Code writter!!! 
  // make sure all cores and possible VA address never use the same page within 1G memory space
  //
  //
  //
  //
  int icid = ~cid & 0x3;
  for (long i = 0; i < MAX_TEST_PAGES; i++)
  {
    GET_PA_ADR(i);
    freelist_nodes[cid][i].addr = pte_entry;
    freelist_nodes[cid][i].next = userPA2KVA(&freelist_nodes[cid][i+1]);
  }
  freelist_nodes[cid][MAX_TEST_PAGES-1].next = 0;
#endif
  //
  //
  trapframe_t tf[MAX_CORES];
  memset(&tf[cid], 0, sizeof(tf[0]));
  tf[cid].epc = test_addr - DRAM_BASE;
  pop_tf(&tf[cid]);
}
#endif


//
// Handle page fault
//
#ifndef FAULT_MGR
#define FAULT_MGR handle_fault_giga
void handle_fault_giga(uintptr_t addr, uintptr_t cause)
{
  //
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

  int index = IDX_VPN0; // default
  // override if define
  OVERRIDE_INDEX();

  //
  // a hits on user space?
  //
  if (LEAF_ENTRY_PTR) { // assuming they are all unique!!!
    if (!(LEAF_ENTRY_PTR & PTE_A)) {
      CLR_UNUSED_PPN;
      LEAF_ENTRY_PTR |= PTE_A;
    } else {
      //FIXME!!! assert(!(vpn0pt[index] & PTE_D) && cause == CAUSE_STORE_PAGE_FAULT);
      LEAF_ENTRY_PTR |= PTE_D;
      // mis-align store?
      // misalign load? 
    }
    flush_page(addr);
    return;
  }
  //
  // walk the list to get the next one
  //
  freelist_t* node = freelist_head[cid];
  assert(node);
  freelist_head[cid] = node->next;
  if (freelist_head[cid] == freelist_tail[cid])
    freelist_tail[cid] = 0; // FIXME!!! TODO: evict the old one to make room for new one!!
  //
  uintptr_t new_pte = (node->addr >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V | PTE_U | PTE_R | PTE_W | PTE_X;
  //
  // add as needed
  //
  //
  LEAF_ENTRY_PTR = new_pte | PTE_A | PTE_D;
  flush_page(addr);

  uintptr_t sstatus = set_csr(sstatus, SSTATUS_SUM);
  //
  // tony duong: this to reduce the execution time, since most of the test just touch couple of intructions at beginning of the page
  // 12/3/20
  // restrict to within 1 G physical space

  //
  // miss on kernel space?
  //
  //
  if (level2 == 0) { // within 1 G
    memcpy((void*)addr, userVA2kernelVA(addr), BYTE2COPY); // dont need to copy the whole page!!!! since most of them are NOPs
  } 
#if 1
  else {
    *(volatile uint64_t *)((uintptr_t)addr)=0;
  }
#endif
  //
  write_csr(sstatus, sstatus);
  //
  LEAF_ENTRY_PTR = new_pte; 
  flush_page(addr);
  //
  __builtin___clear_cache(0,0);
}
#endif

// mis-align
void handle_misalign_load(uintptr_t addr, uintptr_t cause, long *data) {
  uint8_t dat8;
  handle_fault_giga(addr, cause);
  *data = 0;
  for (int i=0;i<8;i++){
    dat8 = *(volatile uint8_t *)(userVA2kernelVA(addr+i));
    *data = (*data << 8) | dat8;
  }
}
void handle_misalign_store(uintptr_t addr, uintptr_t cause, long data) {
  handle_fault_giga(addr,cause);
  for (int i=7;i>=0;i--){
    *(volatile uint8_t *)(userVA2kernelVA(addr+i)) = data & 0xff;
    data = data >> 8;
  }
}

void handle_trap(trapframe_t* tf)
{
  if (tf->cause == CAUSE_USER_ECALL)
  {
    int n = tf->gpr[10];
    hangMe(); // FIXME!!!
  }
  else if (tf->cause == CAUSE_ILLEGAL_INSTRUCTION)
  {
    assert(tf->epc % 4 == 0);

    int* fssr;
    asm ("jal %0, 1f; fssr x0; 1:" : "=r"(fssr));

    if (*(int*)tf->epc == *fssr) {
      hangMe(); // FIXME
    }
    else
      assert(!"illegal instruction");
    tf->epc += 4;
  }
  else if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_LOAD_PAGE_FAULT || tf->cause == CAUSE_STORE_PAGE_FAULT) {
    FAULT_MGR(tf->badvaddr, tf->cause);
  }
  else if (tf->cause == CAUSE_MISALIGNED_STORE) {
    handle_misalign_store(tf->badvaddr, tf->cause, tf->gpr[7]); // t2 = x7 = WRITE !!
    tf->epc += 4;
  }
  else if (tf->cause == CAUSE_MISALIGNED_LOAD) {
    handle_misalign_load(tf->badvaddr, tf->cause, &tf->gpr[28]); // t3 = x28 = READ!!!
    tf->epc += 4;
  }
  else
    assert(!"unexpected exception");

  pop_tf(tf);
}


