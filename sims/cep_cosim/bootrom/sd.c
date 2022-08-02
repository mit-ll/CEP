//-------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:     sd.c
// Program:       Common Evaluation Platform (CEP)
// Description:   SPI and UART initialization code for the CEP Bootrom
// Notes:         Specification referenced is:
//                "SD Specifications Part 1 Physical Layer Simplified Specification 8.00, September 23, 2020"
//
//                - Updated ACMD41 processing to read all five bytes of the R3 response and check
//                  the busy bit in the response per specification Figure 4-4 (Response bit 39)
//                - Removed 34-byte BBL offset in sd_copy (now set to 0)
//                - Increased default payload size to 35MB to accomodate larger Linux images
//--------------------------------------------------------------------------------------

// See LICENSE.Sifive for license details.
#include <stdio.h>
#include <stdint.h>
#include <platform.h>

// Payload size in bytes
#define PAYLOAD_SIZE_B (35 << 20) // 35 MB

// A sector is 512 bytes, so (1 << 11) * 512B = 1 MiB
#define SECTOR_SIZE_B 512

// Payload size in # of sectors
#define PAYLOAD_SIZE (PAYLOAD_SIZE_B / SECTOR_SIZE_B)

// The sector at which the BBL partition starts
#define BBL_PARTITION_START_SECTOR 34

#ifndef TL_CLK
#error Must define TL_CLK
#endif

#define F_CLK TL_CLK

#define REG64(p, i) ((p)[(i) >> 3])
#define REG32(p, i) ((p)[(i) >> 2])
#define REG16(p, i) ((p)[(i) >> 1])

static volatile uint32_t * const spi = (void *)(SPI_CTRL_ADDR);
static volatile uint32_t * const gpio = (void *)(GPIO_CTRL_ADDR);
static volatile uint64_t * const cepregs = (void *)(CEPREGS_ADDR);
static volatile uint32_t * const uart = (void *)(UART_CTRL_ADDR);

// Button used to allow for a "fast boot"
#ifdef VCU118
  // GPIO Button N on VCU118
  #define BTN0_MASK       (0x00000010)
#elif VC707
  // GPIO Button N on VC707
  #define BTN0_MASK       (0x00000100)
#else
  // BTN0 on Arty100T
  #define BTN0_MASK       (0x00001000)
#endif

static inline uint8_t spi_xfer(uint8_t d)
{
  int32_t r;

  REG32(spi, SPI_REG_TXFIFO) = d;
  do {
    r = REG32(spi, SPI_REG_RXFIFO);
  } while (r < 0);
  return r;
}

static inline uint8_t sd_dummy(void)
{
  return spi_xfer(0xFF);
}

static uint8_t sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  unsigned long n;
  uint8_t r;

  REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_HOLD;
  sd_dummy();
  spi_xfer(cmd);
  spi_xfer(arg >> 24);
  spi_xfer(arg >> 16);
  spi_xfer(arg >> 8);
  spi_xfer(arg);
  spi_xfer(crc);

  n = 1000;
  do {
    r = sd_dummy();
    if (!(r & 0x80)) {
      goto done;
    }
  } while (--n > 0);
  puts("sd_cmd: timeout");
done:
  return r;
}

static inline void sd_cmd_end(void)
{
  sd_dummy();
  REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_AUTO;
}


static void sd_poweron(void)
{
  long i;
  REG32(spi, SPI_REG_SCKDIV) = (F_CLK / 300000UL);
  REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_OFF;
  for (i = 10; i > 0; i--) {
    sd_dummy();
  }
  REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_AUTO;
}

static int sd_cmd0(void)
{
  int rc;
  puts("CMD0");
  rc = (sd_cmd(0x40, 0, 0x95) != 0x01);
  sd_cmd_end();
  return rc;
}

static int sd_cmd8(void)
{
  int rc;
  puts("CMD8");
  // Per section 7.3.2.6 of the specification, the card should be in the IDLE state and
  // running the initialization process
  rc = (sd_cmd(0x48, 0x000001AA, 0x87) != 0x01);
  sd_dummy();                         /* command version; reserved  */
  sd_dummy();                         /* reserved                   */
  rc |= ((sd_dummy() & 0xF) != 0x1);  /* voltage                    */
  rc |= (sd_dummy() != 0xAA);         /* check pattern              */
  sd_cmd_end();
  return rc;
}

static void sd_cmd55(void)
{
  sd_cmd(0x77, 0, 0x65);
  sd_cmd_end();
}

static int sd_acmd41(void)
{
  uint8_t r;
  puts("ACMD41");
  do {
    sd_cmd55();
    r = sd_cmd(0x69, 0x40000000, 0x77); /* HCS = 1 */
  } while (r == 0x01);
  return (r != 0x00);
}

static int sd_cmd58(void)
{
  int rc;
  puts("CMD58");
  rc = (sd_cmd(0x7A, 0, 0xFD) != 0x00);
  rc |= ((sd_dummy() & 0x80) != 0x80); /* Power up status */
  sd_dummy();
  sd_dummy();
  sd_dummy();
  sd_cmd_end();
  return rc;
}

// Set block length set to 512 bytes
static int sd_cmd16(void)
{
  int rc;
  puts("CMD16");
  rc = (sd_cmd(0x50, 0x200, 0x15) != 0x00);
  sd_cmd_end();
  return rc;
}

static uint16_t crc16_round(uint16_t crc, uint8_t data) {
  crc = (uint8_t)(crc >> 8) | (crc << 8);
  crc ^= data;
  crc ^= (uint8_t)(crc >> 4) & 0xf;
  crc ^= crc << 12;
  crc ^= (crc & 0xff) << 5;
  return crc;
}

#define STATUS_SHIFT  6
#define STATUS_NEWLINE_SHIFT 12
#define STATUS_UPDATE(i)  (!((i) & ((1 << STATUS_SHIFT)-1)))
#define STATUS_NEWLINE(i) (!((i) & ((1 << STATUS_NEWLINE_SHIFT)-1)))

// Copy SD contents to main memory
static int sd_copy(void)
{
  volatile uint8_t *p = (void *)(MEMORY_MEM_ADDR);
  long i = PAYLOAD_SIZE;
  int rc = 0;
  uint32_t fast_boot = 0;

  puts("CMD18");

  // Read the state of button 0 on the arty100t board to determine if a fast boot is requested
  REG32(gpio, GPIO_INPUT_EN) = (uint32_t)(BTN0_MASK);
  fast_boot = REG32(gpio, GPIO_INPUT_VAL) & BTN0_MASK;
  REG32(gpio, GPIO_INPUT_EN) = (uint32_t)(0);
  
  // The following logic allows for overiding of the default payload size by either holding button zero upon
  // release from reset OR being forced by the simulation.
  // Given that the simulation has it's own override, the "fast boot" is targetted to bare metal boots
  // on the FPGA dev boards. 
  if (fast_boot) {
    i = 0x100;  // 256 x 512B = 132kB
  } else {
    REG64(cepregs, CEPREGS_SCRATCH_W7) = i;
    i = REG64(cepregs, CEPREGS_SCRATCH_W7);
  }

  // Performing multiplication here in the event that PAYLOAD_SIZE is
  // overriden in simulation
  printf("LOADING %ldkB PAYLOAD\n", (i * SECTOR_SIZE_B)/1024);

  // Begin a multi-cycle read.  Divider taked from default VCU118 Bootroml
  REG32(spi, SPI_REG_SCKDIV) = (F_CLK / 5000000UL);
  
  if (sd_cmd(0x52, BBL_PARTITION_START_SECTOR, 0xE1) != 0x00) {
    sd_cmd_end();
    return 1;
  }

  do {
    uint16_t crc, crc_exp;
    long n;

    crc   = 0;
    n     = SECTOR_SIZE_B;

    // Wait for the start token
    while (sd_dummy() != 0xFE);

    // Copy a block/sector of data
    do {
      uint8_t x = sd_dummy();
      *p++ = x;
      crc = crc16_round(crc, x);
    } while (--n > 0);

    crc_exp = ((uint16_t)sd_dummy() << 8);
    crc_exp |= sd_dummy();

    if (crc != crc_exp) {
      puts("CRC mismatch");
      rc = 1;
      break;
    }

    if (STATUS_UPDATE(i)) {
      putchar('.');
      if (STATUS_NEWLINE(i)) {
        printf("%ldkB\n", (i * SECTOR_SIZE_B)/1024);
      }
    }

  } while (--i > 0);

  sd_cmd_end();
  sd_cmd(0x4C, 0, 0x01);
  sd_cmd_end();
  printf("\n");

  return rc;
}

// Main Function
int main(void)
{

  // The default Chipyard VCU118/Arty100t bootrom has been modified
  // to read the CEP W0 Scratch Register to alter functionality.  By default,
  // the following is enabled:
  // - Print Welcome message out the UART
  // - Enable SD Boot
  //
  // If bits 1 and 0 are set, the UART Welcome message will be disabled
  // If bits 2 and 3 are set, SD Boot will be disabled
  //
  uint64_t scratch_reg = 0;
  uint64_t version_reg = 0;
  uint8_t  major_version = 0;
  uint8_t  minor_version = 0;

  scratch_reg = REG64(cepregs, CEPREGS_SCRATCH_W0);
  version_reg = REG64(cepregs, CEPREGS_VERSION);
  major_version = (version_reg >> 48) & 0xFF;
  minor_version = (version_reg >> 56) & 0xFF;

  // Enable the UART
  REG32(uart, UART_REG_TXCTRL)  = UART_TXEN;

  // Enable the welcome message if the two LSBits in CEP Scratch Register are NOT set
  if ((scratch_reg & 0x3) != 0x3) {
    printf("\n");
    printf("---          Common Evaluation Platform v%x.%x            ---\n", major_version, minor_version);
    printf("---         Based on the UCB Chipyard Framework          ---\n");
    printf("--- Copyright 2022 Massachusetts Institute of Technology ---\n");
    printf("---     BootRom Image built on %s %s      ---\n",__DATE__,__TIME__);
    printf("\n");
  } // if ((scratch_reg & 0x3) != 0x3)

  // Enable SD Boot if bits 3 & 2 of the CEP Scratch register are NOT set
  if ((scratch_reg & 0xC) != 0xC) {
    puts("INIT");
  
    sd_poweron();

    if (sd_cmd0()) {
      puts("CMD0 ERROR");
      return 1;     
    }

    if (sd_cmd8()) {
      puts("CMD8 ERROR");
      return 1;     
    }

    if (sd_acmd41()) {
      puts("ACMD41 ERROR");
      return 1;     
    }

    if (sd_cmd58()) {
      puts("CMD58 ERROR");
      return 1;     
    }

    if (sd_cmd16()) {
      puts("CMD16 ERROR");
      return 1;     
    }

    if (sd_copy()) {
      puts("SDCOPY ERROR");
      return 1;     
    }

    puts("BOOT");
  } // if ((scratch_reg & 0xC) != 0xC)

  // Force instruction and data stream synchronization
  __asm__ __volatile__ ("fence.i" : : : "memory");

  return 0;
}
