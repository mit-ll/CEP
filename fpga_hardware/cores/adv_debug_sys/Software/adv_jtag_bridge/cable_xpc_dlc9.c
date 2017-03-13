/* cable_xpc_dlc9.c - Xilinx Platform Cable (DLC9) driver for the Advanced JTAG Bridge
   Copyright (C) 2008 - 2010 Nathan Yawn, nathan.yawn@opencores.org

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
#include <sys/types.h>
#include <unistd.h>  // for usleep()
#include <stdlib.h>  // for sleep()

#include "usb.h"  // libusb header

#include "cable_xpc_dlc9.h"
#include "errcodes.h"

jtag_cable_t dlc9_cable_driver = {
    .name ="xpc_usb" ,
    .inout_func = cable_xpcusb_inout,
    .out_func = cable_xpcusb_out,
    .init_func = cable_xpcusb_init,
    .opt_func = cable_xpcusb_opt,
    .bit_out_func = cable_common_write_bit,
    .bit_inout_func = cable_xpcusb_read_write_bit,
    .stream_out_func = cable_common_write_stream,
    .stream_inout_func = cable_common_read_stream,
    .flush_func = NULL,
    .opts = "",
    .help = "no options\n",
   };

#define USB_TIMEOUT 500

// Note that this is based on the UrJTAG driver for the XPC-USB,
// which was 'experimental' at the time.
// It only supports bit-bang mode, and therefore will not be fast.

// USB constants for the DLC9
#define XPCUSB_VID  0x3fd
#define XPCUSB_PID  0x08

// Bit meanings in the command byte sent to the DLC9
// DLC9 has no TRST bit
#define XPCUSB_CMD_TDI 0x01
#define XPCUSB_CMD_TDO 0x01
#define XPCUSB_CMD_TMS 0x02
#define XPCUSB_CMD_TCK 0x04
#define XPCUSB_CMD_PROG 0x08


static struct usb_device *device;


///////////////////////////////////////////////////////////////////////////////
/*----- Functions for the Xilinx Platform Cable USB (Model DLC9)            */
/////////////////////////////////////////////////////////////////////////////
  

static int xpcu_request_28(struct usb_dev_handle *xpcu, int value)
{
  // Maybe clock speed setting?
  if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0028, value, NULL, 0, USB_TIMEOUT)<0)
    {
      fprintf(stderr, "Error sending usb_control_msg(0x28.x)\n");
      return APP_ERR_USB;
    }
  
  return APP_ERR_NONE;
}


static int xpcu_raise_ioa5(struct usb_dev_handle *xpcu)
{
  if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0018, 0x0000, NULL, 0, USB_TIMEOUT)<0)
    {
      fprintf(stderr, "Error sending usb_control_msg(0x18.0x00) (raise IOA.5{\n");
      return APP_ERR_USB;
    }
  
  return APP_ERR_NONE;
}

static int xpcu_select_gpio(struct usb_dev_handle *xpcu, int chain)
{
  if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0052, chain, NULL, 0, USB_TIMEOUT)<0)
    {
      fprintf(stderr, "Error sending usb_control_msg(0x52.x) (select gpio)\n");
      return APP_ERR_USB;
    }
  
  return APP_ERR_NONE;
}

static int xpcu_read_firmware_version(struct usb_dev_handle *xpcu, uint16_t *buf)
{
  if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0050, 0x0000, (char*)buf, 2, USB_TIMEOUT)<0)
    {
      fprintf(stderr,"Error sending usb_control_msg(0x50.0) (read_firmware_version)\n");
      return APP_ERR_USB;
    }
  return APP_ERR_NONE;
}

static int xpcu_read_cpld_version(struct usb_dev_handle *xpcu, uint16_t *buf)
{
  if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0050, 0x0001, (char*)buf, 2, USB_TIMEOUT)<0)
    {
      fprintf(stderr, "Error sending usb_control_msg(0x50.1) (read_cpld_version)\n");
      return APP_ERR_USB;
    }
  return APP_ERR_NONE;
}


static int xpcusb_enumerate_bus(void)
{
  int             flag;  // for USB bus scanning stop condition
  struct usb_bus *bus;   // pointer on the USB bus
  
  // board detection
  usb_init();
  usb_find_busses();
  usb_find_devices();

  flag = 0;
  
  for (bus = usb_get_busses(); bus; bus = bus->next)
  {
    for (device = bus->devices; device; device = device->next)
    {	
      if (device->descriptor.idVendor  == XPCUSB_VID &&
          device->descriptor.idProduct == XPCUSB_PID) 
      {
	      flag = 1;
	      fprintf(stderr, "Found Xilinx Platform Cable USB (DLC9)\n");
	      return APP_ERR_NONE;
      }
    }
    if (flag)
      break;
  }

  fprintf(stderr, "Failed to find Xilinx Platform Cable USB\n");
  return APP_ERR_CABLENOTFOUND;
}


int cable_xpcusb_init()
{
  int err = APP_ERR_NONE;

  // Process to reset the XPC USB (DLC9)
  if(err |= xpcusb_enumerate_bus()) {
    return err;
  }

  usb_dev_handle *h_device = usb_open(device);

  if(h_device == NULL)
    {
      fprintf(stderr, "Init failed to open USB device for reset\n");
      return APP_ERR_USB;
    }
  
  if(usb_reset(h_device) != APP_ERR_NONE)
    fprintf(stderr, "Failed to reset XPC-USB\n");
  
  usb_close(h_device);

  // Wait for reset!!!
  sleep(1);
  
  // Do device initialization
  if(err |= xpcusb_enumerate_bus())
    return err;

  h_device = usb_open(device);
  if(h_device == NULL)
    {
      fprintf(stderr, "Init failed to open USB device for initialization\n");
      return APP_ERR_USB;
    }

  // set the configuration
  if (usb_set_configuration(h_device, device->config->bConfigurationValue))
    {
      usb_close(h_device);
      fprintf(stderr, "USB-reset failed to set configuration\n");
      return APP_ERR_USB;
    }

  while (usb_claim_interface(h_device, device->config->interface->altsetting->bInterfaceNumber));

  // DO DEVICE-SPECIFIC INIT HERE
  // Don't mess with the order here, it's easy to break.

  // Maybe set the clock speed?
  if(xpcu_request_28(h_device, 0x11) != APP_ERR_NONE)	{
    fprintf(stderr, "Request 28 (set clock speed?) failed.\n");
  }

  // Set internal TCK,TMS,TDO to 0
  if(usb_control_msg(h_device, 0x40, 0xB0, 0x0030, 0x08, NULL, 0, USB_TIMEOUT)!= APP_ERR_NONE) {
    fprintf(stderr, "usb_control_msg(0x30.0x00) (write port E) failed\n");
  }
 
  // Read firmware version (constant embedded in firmware)
  uint16_t buf;

  if(xpcu_read_firmware_version(h_device, &buf) != APP_ERR_NONE)	{
    fprintf(stderr, "Failed to read firmware version.\n");
  }
  else 	{
    printf("firmware version = 0x%04X (%u)\n", buf, buf);
  }

  // Read CPLD version (uses the internal GPIF interface)
  if(xpcu_read_cpld_version(h_device, &buf) != APP_ERR_NONE) {
    fprintf(stderr, "Failed to read CPLD version.\n");
  }
  else
    {
      printf("cable CPLD version = 0x%04X (%u)\n", buf, buf);
      if(buf == 0) 		{
	printf("Warning: version '0' can't be correct. Please try resetting the cable\n");
      }
    }
  
  // Set IOA bit 5, which enables output buffers
  if(xpcu_raise_ioa5(h_device) != APP_ERR_NONE) {
    fprintf(stderr, "Failed to enable XPC output buffers\n");
  }
  
  // access external chain for normal operation
  if(xpcu_select_gpio(h_device, 0) != APP_ERR_NONE) {
    fprintf(stderr, "Failed to select external JTAG chain\n");
  }

  // Init all done, release cable
  if (usb_release_interface(h_device, device->config->interface->altsetting->bInterfaceNumber)){
    usb_close(h_device);
    fprintf(stderr, "USB-out failed to release interface\n");
    return APP_ERR_USB;
  }

  usb_close(h_device);

  return APP_ERR_NONE;
}


int cable_xpcusb_out(uint8_t value)
{
  int             rv;                  // to catch return values of functions
  usb_dev_handle *h_device;            // handle on the ubs device
  uint8_t out;

  // open the device
  h_device = usb_open(device);
  if (h_device == NULL){
    usb_close(h_device);
    fprintf(stderr, "USB-out failed to open device\n");
    return APP_ERR_USB;
  }
 
  // set the configuration
  if (usb_set_configuration(h_device, device->config->bConfigurationValue))
    {
      usb_close(h_device);
      fprintf(stderr, "USB-out failed to set configuration\n");
      return APP_ERR_USB;
    }
  
  // wait until device is ready
  while (usb_claim_interface(h_device, device->config->interface->altsetting->bInterfaceNumber));
  
  // send the buffer
  // Translate to USB blaster protocol
  out = 0;
  if(value & TCLK_BIT)
    out |= XPCUSB_CMD_TCK;
  if(value & TDI_BIT)
    out |= XPCUSB_CMD_TDI;
  if(value & TMS_BIT)
    out |= XPCUSB_CMD_TMS;

  out |= XPCUSB_CMD_PROG;  // Set output PROG (always necessary)

  /* debug
  if(value & TDI_BIT)
    printf("Write 1\n");
  else
    printf("Write 0\n");
  */

  rv = usb_control_msg(h_device, 0x40, 0xB0, 0x0030, out, NULL, 0, USB_TIMEOUT);
  if (rv < 0){
    fprintf(stderr, "\nFailed to send a write control message (rv = %d):\n%s\n", rv, usb_strerror());
  }

  // release the interface cleanly
  if (usb_release_interface(h_device, device->config->interface->altsetting->bInterfaceNumber)){
    fprintf(stderr, "Warning: failed to release usb interface after write\n");
  }
  
  // close the device
  usb_close(h_device);
  return APP_ERR_NONE;
}


int cable_xpcusb_inout(uint8_t value, uint8_t *inval)
{
  int rv;                  // to catch return values of functions
  usb_dev_handle *h_device;            // handle on the usb device
  char ret = 0;
  uint8_t out;

  // Translate to USB blaster protocol
  out = 0;
  if(value & TCLK_BIT)
    out |= XPCUSB_CMD_TCK;
  if(value & TDI_BIT)
    out |= XPCUSB_CMD_TDI;
  if(value & TMS_BIT)
    out |= XPCUSB_CMD_TMS;

  out |= XPCUSB_CMD_PROG;  // Set output PROG (always necessary)

  // open the device
  h_device = usb_open(device);
  if (h_device == NULL){
    usb_close(h_device);
    return APP_ERR_USB;
  }
 
  // set the configuration
  if (usb_set_configuration(h_device, device->config->bConfigurationValue)){
    usb_close(h_device);
    return APP_ERR_USB;
  }

  // wait until device is ready
  while (usb_claim_interface(h_device, device->config->interface->altsetting->bInterfaceNumber));

  // Send the output
  rv = usb_control_msg(h_device, 0x40, 0xB0, 0x0030, out, NULL, 0, USB_TIMEOUT);
  if (rv < 0){
    fprintf(stderr, "\nFailed to send a write control message (rv = %x):\n%s\n", rv, usb_strerror());
    goto usbblaster_in_fail;
  }


  // receive the response
  rv = usb_control_msg(h_device, 0xC0, 0xB0, 0x0038, 0, (char*)&ret, 1, USB_TIMEOUT);
  if (rv < 0){
    fprintf(stderr, "\nFailed to execute a read control message:\n%s\n", usb_strerror());
    goto usbblaster_in_fail;
  }


  // release the interface cleanly
  if (usb_release_interface(h_device, device->config->interface->altsetting->bInterfaceNumber)){
    fprintf(stderr, "Warning: failed to release USB interface after read\n");
    usb_close(h_device);
    return APP_ERR_USB;
  }

  // close the device
  usb_close(h_device);

  /* debug
  if(value & TDI_BIT)
    printf("Write 1, ");
  else
    printf("Write 0, ");
  */

  if(ret & XPCUSB_CMD_TDO)
    *inval = 1;
  else
    *inval = 0;

  //printf("Read 0\n");
  return APP_ERR_NONE;

usbblaster_in_fail:
  usb_release_interface(h_device, device->config->interface->altsetting->bInterfaceNumber);
  usb_close(h_device);
  return APP_ERR_USB;
}


// Xilinx couldn't be like everyone else.  Oh, no.
// For some reason, "set data/drop TCK" then "read data/raise TCK" won't work.
// So we have our very own bit read/write function.  @whee.
int cable_xpcusb_read_write_bit(uint8_t packet_out, uint8_t *bit_in) {
  uint8_t data = TRST_BIT;  //  TRST is active low, don't clear unless /set/ in 'packet'
  int err = APP_ERR_NONE;

  /* Write data, drop clock */
  if(packet_out & TDO) data |= TDI_BIT;
  if(packet_out & TMS) data |= TMS_BIT;
  if(packet_out & TRST) data &= ~TRST_BIT;

  err |= cable_xpcusb_inout(data, bit_in);  // read in bit, set data, drop clock
  err |= cable_xpcusb_out(data|TCLK_BIT);  // clk hi

  return err;
}


int cable_xpcusb_opt(int c, char *str)
{
    fprintf(stderr, "Unknown parameter '%c'\n", c);
    return APP_ERR_BAD_PARAM;
}

jtag_cable_t *cable_xpcusb_get_driver(void)
{
  return &dlc9_cable_driver; 
}
