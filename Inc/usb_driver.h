/*
 * usb_driver.h
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#ifndef USB_DRIVER_H_
#define USB_DRIVER_H_

#include "stm32f4xx.h"

#define USB_OTG_FS_GLOBAL ((USB_OTG_GlobalTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_GLOBAL_BASE))
#define USB_OTG_FS_DEVICE ((USB_OTG_DeviceTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USB_OTG_FS_PCGCCTL ((__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)) // Power and clock gating control register

void initialize_gpio_pins();
void initialize_core();
void connect();
void disconnect();

#endif /* USB_DRIVER_H_ */
