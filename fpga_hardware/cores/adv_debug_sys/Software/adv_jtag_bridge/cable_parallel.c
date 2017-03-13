/* cable_parallel.c - Parallel cable drivers (XPC3 and XESS) for the Advanced JTAG Bridge
   Copyright (C) 2001 Marko Mlinar, markom@opencores.org
   Copyright (C) 2004 György Jeney, nog@sdf.lonestar.org
   
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */



#include <stdio.h>
#include <sys/io.h>  // for inb(), outb()
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include "cable_parallel.h"
#include "errcodes.h"

#ifdef __PARALLEL_TIMER_BUSY_WAIT__
 #if ((_POSIX_TIMERS) && (_POSIX_CPUTIME))
 #define PARALLEL_USE_PROCESS_TIMER
 #endif
#endif

jtag_cable_t xpc3_cable_driver = {
    .name = "xpc3",
    .inout_func = cable_xpc3_inout,
    .out_func = cable_xpc3_out,
    .init_func = cable_parallel_init,
    .opt_func = cable_parallel_opt,
    .bit_out_func = cable_common_write_bit,
    .bit_inout_func = cable_common_read_write_bit,
    .stream_out_func = cable_common_write_stream,
    .stream_inout_func = cable_common_read_stream,
    .flush_func = NULL,
    .opts = "p:",
    .help = "-p [port] Which port to use when communicating with the parport hardware (eg. 0x378)\n"
    };

jtag_cable_t xess_cable_driver = {
    .name = "xess",
    .inout_func = cable_xess_inout,
    .out_func = cable_xess_out,
    .init_func = cable_parallel_init,
    .opt_func = cable_parallel_opt,
    .bit_out_func = cable_common_write_bit,
    .bit_inout_func = cable_common_read_write_bit,
    .stream_out_func = cable_common_write_stream,
    .stream_inout_func = cable_common_read_stream,
    .flush_func = NULL,
    .opts = "p:",
    .help = "-p [port] Which port to use when communicating with the parport hardware (eg. 0x378)\n",
    };

#define LPT_READ (base+1)
#define LPT_WRITE base

// Common functions used by both cable types
static int cable_parallel_out(uint8_t value);
static int cable_parallel_inout(uint8_t value, uint8_t *inval);


static int base = 0x378;

#ifdef __PARALLEL_TIMER_BUSY_WAIT__
 #ifndef PARALLEL_USE_PROCESS_TIMER
 struct timeval last_tv;
 #endif
#endif

/////////////////////////////////////////////////////////////////////////////////
/*-------------------------------------[ Parallel port specific functions ]---*/
///////////////////////////////////////////////////////////////////////////////

int cable_parallel_init()
{
  if (ioperm(base, 3, 1)) {
    fprintf(stderr, "Couldn't get the port at %x\n", base);
    perror("Root privileges are required.\n");
    return APP_ERR_INIT_FAILED;
  }
  printf("Connected to parallel port at %x\n", base);
  printf("Dropping root privileges.\n");
  setreuid(getuid(), getuid());

#ifdef __PARALLEL_TIMER_BUSY_WAIT__
 #ifdef PARALLEL_USE_PROCESS_TIMER
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  clock_settime(CLOCK_PROCESS_CPUTIME_ID, &ts);
 #else
  gettimeofday(&last_tv, NULL);
 #endif
#endif

  return APP_ERR_NONE;
}


int cable_parallel_opt(int c, char *str)
{
  switch(c) {
  case 'p':
    if(!sscanf(str, "%x", &base)) {
      fprintf(stderr, "p parameter must have a hex number as parameter\n");
      return APP_ERR_BAD_PARAM;
    }
    break;
  default:
    fprintf(stderr, "Unknown parameter '%c'\n", c);
    return APP_ERR_BAD_PARAM;
  }
  return APP_ERR_NONE;
}

/*-----------------------------------------[ Physical board wait function ]---*/

/* Multiple users have reported poor performance of parallel cables,
 * which has been traced to various sleep functions sleeping much longer than 
 * microseconds.  The same users have reported error-free functionality
 * and an order of magnitude improvement in upload speed.
 * Other users have reported errors when running without a wait.
 * Impact apparently limits the frequency of parallel JTAG cables
 * to 200 kHz, and some clones fail at higher speeds.
 */ 



#ifdef __PARALLEL_SLEEP_WAIT__
void cable_parallel_phys_wait()
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 2500;
  nanosleep(&ts, NULL);
}
#else

 #ifdef __PARALLEL_TIMER_BUSY_WAIT__

  #ifndef PARALLEL_USE_PROCESS_TIMER

/* Helper function needed if process timer isn't implemented */
/* do x-y */
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
  #endif


void cable_parallel_phys_wait()
{
  /* This busy wait attempts to make the frequency exactly 200kHz,
   * including the processing time between ticks.
   * This means a period of 5us, or half a period of 2.5us.
   */
  #ifdef PARALLEL_USE_PROCESS_TIMER
  struct timespec ts;
  do
    {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); 
    } while((ts.tv_sec == 0) && (ts.tv_nsec < 2500));

  /* Doing the set after the check means that processing time
   * is not added to the wait. */
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  clock_settime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  #else
  struct timeval now_tv;
  struct timeval results_tv;
  do
    {
      gettimeofday(&now_tv, NULL);
      timeval_subtract (&results_tv, &now_tv, &last_tv);
    } while((results_tv.tv_sec == 0) && (results_tv.tv_usec < 3));
  last_tv = now_tv;
  #endif
}

 #else  // NO WAIT

void cable_parallel_phys_wait()
{
  // No wait, run max speed
}

 #endif
#endif


/*----------------------------------------------[ xpc3 specific functions ]---*/
jtag_cable_t *cable_xpc3_get_driver(void)
{
  return &xpc3_cable_driver; 
}

int cable_xpc3_out(uint8_t value)
{
  uint8_t out = 0;

  cable_parallel_phys_wait();  // Limit the max clock rate if necessary

  /* First convert the bits in value byte to the ones that the cable wants */
  if(value & TCLK_BIT)
    out |= 0x02; /* D1 pin 3 */
  if(value & TRST_BIT)
    out |= 0x10; /* Not used */
  if(value & TDI_BIT)
    out |= 0x01; /* D0 pin 2 */
  if(value & TMS_BIT)
    out |= 0x04; /* D2 pin 4 */

  return cable_parallel_out(out);
}

int cable_xpc3_inout(uint8_t value, uint8_t *inval)
{
  uint8_t in;
  int retval;
  uint8_t out = 0;

  cable_parallel_phys_wait();  // Limit the max clock rate if necessary

  /* First convert the bits in value byte to the ones that the cable wants */
  if(value & TCLK_BIT)
    out |= 0x02; /* D1 pin 3 */
  if(value & TRST_BIT)
    out |= 0x10; /* Not used */
  if(value & TDI_BIT)
    out |= 0x01; /* D0 pin 2 */
  if(value & TMS_BIT)
    out |= 0x04; /* D2 pin 4 */

  retval = cable_parallel_inout(out, &in);

  if(in & 0x10) /* S6 pin 13 */
    *inval = 1;
  else
    *inval = 0;

  return retval;
}

/*----------------------------------------------[ xess specific functions ]---*/
jtag_cable_t *cable_xess_get_driver(void)
{
  return &xess_cable_driver; 
}

int cable_xess_out(uint8_t value)
{
  uint8_t out = 0;

  cable_parallel_phys_wait();  // Limit the max clock rate if necessary

  /* First convert the bits in value byte to the ones that the cable wants */
  if(value & TCLK_BIT)
    out |= 0x04; /* D2 pin 4 */
  if(value & TRST_BIT)
    out |= 0x08; /* D3 pin 5 */
  if(value & TDI_BIT)
    out |= 0x10; /* D4 pin 6 */
  if(value & TMS_BIT)
    out |= 0x20; /* D3 pin 5 */

  return cable_parallel_out(out);
}

int cable_xess_inout(uint8_t value, uint8_t *inval)
{
  uint8_t in;
  int retval;
  uint8_t out = 0;

  cable_parallel_phys_wait();  // Limit the max clock rate if necessary

  /* First convert the bits in value byte to the ones that the cable wants */
  if(value & TCLK_BIT)
    out |= 0x04; /* D2 pin 4 */
  if(value & TRST_BIT)
    out |= 0x08; /* D3 pin 5 */
  if(value & TDI_BIT)
    out |= 0x10; /* D4 pin 6 */
  if(value & TMS_BIT)
    out |= 0x20; /* D3 pin 5 */

  retval = cable_parallel_inout(out, &in);

  if(in & 0x20) /* S5 pin 12*/
    *inval = 1;
  else
    *inval = 0;

  return retval;
}


/*----------------------------------------------[ common helper functions ]---*/
// 'static' for internal access only

static int cable_parallel_out(uint8_t value)
{
  outb(value, LPT_WRITE);
  return APP_ERR_NONE;
}

static int cable_parallel_inout(uint8_t value, uint8_t *inval)
{
  *inval = inb(LPT_READ);
  outb(value, LPT_WRITE);

  return APP_ERR_NONE;
}
