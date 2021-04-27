/*
 * usb_framework.h
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#ifndef USB_FRAMEWORK_H_
#define USB_FRAMEWORK_H_

#include "usb_driver.h"
#include "usb_standards.h"

void usbd_initialize();
void usbd_poll();

#endif /* USB_FRAMEWORK_H_ */
