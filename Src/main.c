/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "logger.h"
#include "usb_framework.h"
#include "usb_device.h"

UsbDevice usb_device;
uint32_t buffer[8];

int main(void)
{
	log_info("Program Entry-Point");
	usb_device.ptr_out_buffer = &buffer;

	usbd_initialize(&usb_device);
    /* Loop forever */
	for(;;)
	{
		usbd_poll();
	}
}
