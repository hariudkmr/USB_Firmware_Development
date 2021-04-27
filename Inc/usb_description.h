/*
 * usb_description.h
 *
 *  Created on: Apr 27, 2021
 *      Author: uidn9556
 */

#ifndef USB_DESCRIPTION_H_
#define USB_DESCRIPTION_H_

#include "usb_standards.h"

const UsbDeviceDescriptor device_descriptor = {
		.blength 			= sizeof(UsbDeviceDescriptor),
		.bDescription 		= USB_DESCRIPTOR_TYPE_DEVICE,
		.bcdDevice 			= 0x200,
		.bDeviceClass 		= USB_CLASS_PER_INTERFACE,
		.bDeviceSubClass	= USB_SUBCLASS_NONE,
		.bDeviceProtocol	= USB_PROTOCOL_NONE,
		.bMaxPacketSize0	= 8,
		.idVendor			= 0x6666,
		.idProduct			= 0x13AA,
		.bcdDevice			= 0x0100,
		.iManufacturer		= 0,
		.iProduct			= 0,
		.iSerialNumber		= 0,
		.bNumConfigurations	= 1
};

#endif /* USB_DESCRIPTION_H_ */
