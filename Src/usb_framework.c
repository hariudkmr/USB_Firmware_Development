/*
 * usb_framework.c
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#include "usb_framework.h"
#include "usb_driver.h"
#include "usb_device.h"


static UsbDevice *usbd_handle;

void usbd_initialize(UsbDevice *usb_device){

	usbd_handle = usb_device;
	usb_driver.initialize_gpio_pins();
	usb_driver.initialize_core();
	usb_driver.connect();


}

static void process_request(){

}

void usbd_poll()
{
	usb_driver.poll();
}

static void usb_reset_received_handler()
{
	usbd_handle->in_data_size = 0;
	usbd_handle->out_data_size = 0;
	usbd_handle->configuration_value = 0;
	usbd_handle->device_state = USB_DEVICE_STATE_DEFAULT;
	usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_SETUP;
	usb_driver.set_device_address(0);
}

static void setup_data_received_handler(uint8_t endpointnumber, uint16_t byte_count){

	usb_driver.read_packet(usbd_handle->ptr_out_buffer, byte_count);
	process_request();

}

const UsbEvents usb_events = {
		.on_usb_reset_received = &usb_reset_received_handler,
		.on_setup_data_received = &setup_data_received_handler
};

