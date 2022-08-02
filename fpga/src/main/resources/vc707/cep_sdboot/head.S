// See LICENSE.Sifive for license details.
#include <platform.h>
#include <smp.h>

  .section .text.init
  .option norvc
  .globl _prog_start
_prog_start:
  smp_pause(s1, s2)
  li sp, (MEMORY_MEM_ADDR + 0xffff000)
  call main
  smp_resume(s1, s2)
  csrr a0, mhartid // hartid for next level bootloader
  la a1, dtb // dtb address for next level bootloader
  li s1, MEMORY_MEM_ADDR
  jr s1

  .section .dtb
  .align 3
dtb:
