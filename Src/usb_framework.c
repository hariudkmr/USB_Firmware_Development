/*
 * usb_framework.c
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#include "usb_framework.h"

void usbd_initialize(){

	initialize_gpio_pins();
	initialize_core();
	connect();


}
