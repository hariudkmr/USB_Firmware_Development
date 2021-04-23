/*
 * usb_driver.c
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#include "usb_driver.h"

void initialize_gpio_pins()
{
	// Enables the clock for GPIOA.
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

	// Sets alternate function 10 for: PA11 (-), and PA12 (+).
	MODIFY_REG(GPIOA->AFR[1],
		GPIO_AFRH_AFSEL11 | GPIO_AFRH_AFSEL12,
		_VAL2FLD(GPIO_AFRH_AFSEL11, 0xA) | _VAL2FLD(GPIO_AFRH_AFSEL12, 0xA)
	);

	// Configures USB pins (in GPIOA) to work in alternate function mode.
	MODIFY_REG(GPIOA->MODER,
		GPIO_MODER_MODER11 | GPIO_MODER_MODER12,
		_VAL2FLD(GPIO_MODER_MODER11, 2) | _VAL2FLD(GPIO_MODER_MODER12, 2)
	);
}

void initialize_core()
{
	// Enables the clock for USB core FullSpeed
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);

	// Configures the USB core to run in device mode, and to use the embedded full-speed PHY.
	MODIFY_REG(USB_OTG_FS->GUSBCFG,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_TRDT,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | _VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x09)
	);

	// Configures the device to run in full speed mode.
	MODIFY_REG(USB_OTG_FS_DEVICE->DCFG,
		USB_OTG_DCFG_DSPD,
		_VAL2FLD(USB_OTG_DCFG_DSPD, 0x03)
	);

	// Enables VBUS sensing device.
	SET_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_VBUSBSEN);

	// Unmasks the main USB core interrupts.
	SET_BIT(USB_OTG_FS->GINTMSK,
		USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_SOFM |
		USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_IEPINT |
		USB_OTG_GINTSTS_OEPINT | USB_OTG_GINTMSK_RXFLVLM
	);

	// Clears all pending core interrupts.
	WRITE_REG(USB_OTG_FS->GINTSTS, 0xFFFFFFFF);

	// Unmasks USB global interrupt.
	SET_BIT(USB_OTG_FS->GAHBCFG, USB_OTG_GAHBCFG_GINT);

}

/** \brief Connects the USB device to the bus.
 */
void connect()
{
	// Powers the transceivers on.
    SET_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_PWRDWN);

	// Connects the device to the bus.
    CLEAR_BIT(USB_OTG_FS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
}

/** \brief Disconnects the USB device from the bus.
 */
void disconnect()
{
	// Disconnects the device from the bus.
	SET_BIT(USB_OTG_FS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);

	// Powers the transceivers off.
	CLEAR_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_PWRDWN);
}

