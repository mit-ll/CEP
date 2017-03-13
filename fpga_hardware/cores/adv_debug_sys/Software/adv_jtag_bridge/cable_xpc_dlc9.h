
#ifndef _CABLE_XPC_DLC9_H_
#define _CABLE_XPC_DLC9_H_

#include <stdint.h>
#include "cable_common.h"

jtag_cable_t *cable_xpcusb_get_driver(void);
int cable_xpcusb_init();
int cable_xpcusb_out(uint8_t value);
int cable_xpcusb_inout(uint8_t value, uint8_t *inval);
int cable_xpcusb_opt(int c, char *str);
int cable_xpcusb_read_write_bit(uint8_t packet_out, uint8_t *bit_in);


#endif
