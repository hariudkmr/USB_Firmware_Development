/*
 * usb_framework.c
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#include "usb_framework.h"
#include "usb_driver.h"

void usbd_initialize(){

	usb_driver.initialize_gpio_pins();
	usb_driver.initialize_core();
	usb_driver.connect();


}

void usbd_poll()
{
	usb_driver.poll();
}
