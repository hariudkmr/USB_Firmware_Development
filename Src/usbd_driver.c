/*
 * usb_driver.c
 *
 *  Created on: 23 Apr 2021
 *      Author: hari
 */

#include <usbd_driver.h>

#include "logger.h"
#include "string.h"
#include "usb_standards.h"

static void initialize_gpio_pins() {
  // Enables the clock for GPIOA.
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

  // Sets alternate function 10 for: PA11 (-), and PA12 (+).
  MODIFY_REG(
      GPIOA->AFR[1], GPIO_AFRH_AFSEL11 | GPIO_AFRH_AFSEL12,
      _VAL2FLD(GPIO_AFRH_AFSEL11, 0xA) | _VAL2FLD(GPIO_AFRH_AFSEL12, 0xA));

  // Configures USB pins (in GPIOA) to work in alternate function mode.
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER11 | GPIO_MODER_MODER12,
             _VAL2FLD(GPIO_MODER_MODER11, 2) | _VAL2FLD(GPIO_MODER_MODER12, 2));
}

static void initialize_core() {
  // Enables the clock for USB core FullSpeed
  SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);

  // Configures the USB core to run in device mode, and to use the embedded
  // full-speed PHY.
  MODIFY_REG(
      USB_OTG_FS->GUSBCFG,
      USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_TRDT,
      USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL |
          _VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x09));

  // Configures the device to run in full speed mode.
  MODIFY_REG(USB_OTG_FS_DEVICE->DCFG, USB_OTG_DCFG_DSPD,
             _VAL2FLD(USB_OTG_DCFG_DSPD, 0x03));

  // Enables VBUS sensing device.
  SET_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_VBUSBSEN);

  // Unmasks the main USB core interrupts.
  SET_BIT(USB_OTG_FS->GINTMSK,
          USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM |
              USB_OTG_GINTMSK_SOFM | USB_OTG_GINTMSK_USBSUSPM |
              USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_IEPINT |
              USB_OTG_GINTSTS_OEPINT | USB_OTG_GINTMSK_RXFLVLM);

  // Clears all pending core interrupts.
  WRITE_REG(USB_OTG_FS->GINTSTS, 0xFFFFFFFF);

  // Unmasks USB global interrupt.
  SET_BIT(USB_OTG_FS->GAHBCFG, USB_OTG_GAHBCFG_GINT);

  // Unmasks transfer completed interrupts for all endpoints.
  SET_BIT(USB_OTG_FS_DEVICE->DOEPMSK, USB_OTG_DOEPMSK_XFRCM);
  SET_BIT(USB_OTG_FS_DEVICE->DIEPMSK, USB_OTG_DIEPMSK_XFRCM);
}

static void set_device_address(uint8_t address) {
  MODIFY_REG(USB_OTG_FS_DEVICE->DCFG, USB_OTG_DCFG_DAD,
             _VAL2FLD(USB_OTG_DCFG_DAD, address));
}

/** \brief Connects the USB device to the bus.
 */
static void connect() {
  // Powers the transceivers on.
  SET_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_PWRDWN);

  // Connects the device to the bus.
  CLEAR_BIT(USB_OTG_FS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
}

/** \brief Disconnects the USB device from the bus.
 */
static void disconnect() {
  // Disconnects the device from the bus.
  SET_BIT(USB_OTG_FS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);

  // Powers the transceivers off.
  CLEAR_BIT(USB_OTG_FS->GCCFG, USB_OTG_GCCFG_PWRDWN);
}

/** \brief Pops data from the RxFIFO and stores it in the buffer.
 * \param buffer Pointer to the buffer, in which the popped data will be stored.
 * \param size Count of bytes to be popped from the dedicated RxFIFO memory.
 */
static void read_packet(void *buffer, uint16_t size) {
  // Note: There is only one RxFIFO.
  uint32_t *fifo = FIFO(0);

  for (; size >= 4; size -= 4, buffer += 4) {
    // Pops one 32-bit word of data (until there is less than one word
    // remaining).
    uint32_t data = *fifo;
    // Stores the data in the buffer.
    *((uint32_t *)buffer) = data;
  }

  if (size > 0) {
    // Pops the last remaining bytes (which are less than one word).
    uint32_t data = *fifo;

    for (; size > 0; size--, buffer++, data >>= 8) {
      // Stores the data in the buffer with the correct alignment.
      *((uint8_t *)buffer) = 0xFF & data;
    }
  }
}

/** \brief Pushes a packet into the TxFIFO of an IN endpoint.
 * \param endpoint_number The number of the endpoint, to which the data will be
 * written. \param buffer Pointer to the buffer contains the data to be written
 * to the endpoint. \param size The size of data to be written in bytes.
 */
static void write_packet(uint8_t endpoint_number, void const *buffer,
                         uint16_t size) {
  uint32_t *fifo = FIFO(endpoint_number);
  USB_OTG_INEndpointTypeDef *in_endpoint = IN_ENDPOINT(endpoint_number);

  // Configures the transmission (1 packet that has `size` bytes).
  MODIFY_REG(in_endpoint->DIEPTSIZ,
             USB_OTG_DIEPTSIZ_PKTCNT | USB_OTG_DIEPTSIZ_XFRSIZ,
             _VAL2FLD(USB_OTG_DIEPTSIZ_PKTCNT, 1) |
                 _VAL2FLD(USB_OTG_DIEPTSIZ_XFRSIZ, size));

  // Enables the transmission after clearing both STALL and NAK of the endpoint.
  MODIFY_REG(in_endpoint->DIEPCTL, USB_OTG_DIEPCTL_STALL,
             USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

  // Gets the size in term of 32-bit words (to avoid integer overflow in the
  // loop).
  size = (size + 3) / 4;

  for (; size > 0; size--, buffer += 4) {
    // Pushes the data to the TxFIFO.
    *fifo = *((uint32_t *)buffer);
  }
}

/** \brief Updates the start addresses of all FIFOs according to the size of
 * each FIFO.
 */
static void refresh_fifo_start_addresses() {
  // The first changeable start address begins after the region of RxFIFO.
  uint16_t start_address =
      _FLD2VAL(USB_OTG_GRXFSIZ_RXFD, USB_OTG_FS->GRXFSIZ) * 4;

  // Updates the start address of the TxFIFO0.
  MODIFY_REG(USB_OTG_FS->DIEPTXF0_HNPTXFSIZ, USB_OTG_TX0FSA,
             _VAL2FLD(USB_OTG_TX0FSA, start_address));

  // The next start address is after where the last TxFIFO ends.
  start_address += _FLD2VAL(USB_OTG_TX0FD, USB_OTG_FS->DIEPTXF0_HNPTXFSIZ) * 4;

  // Updates the start addresses of the rest TxFIFOs.
  for (uint8_t txfifo_number = 0; txfifo_number < ENDPOINT_COUNT - 1;
       txfifo_number++) {
    MODIFY_REG(USB_OTG_FS->DIEPTXF[txfifo_number], USB_OTG_NPTXFSA,
               _VAL2FLD(USB_OTG_NPTXFSA, start_address));

    start_address +=
        _FLD2VAL(USB_OTG_NPTXFD, USB_OTG_FS->DIEPTXF[txfifo_number]) * 4;
  }
}

/** \brief Configures the RxFIFO of all OUT endpoints.
 * \param size The size of the largest OUT endpoint in bytes.
 * \note The RxFIFO is shared between all OUT endpoints.
 */
static void configure_rxfifo_size(uint16_t size) {
  // Considers the space required to save status packets in RxFIFO and gets the
  // size in term of 32-bit words.
  size = 10 + (2 * ((size / 4) + 1));

  // Configures the depth of the FIFO.
  MODIFY_REG(USB_OTG_FS->GRXFSIZ, USB_OTG_GRXFSIZ_RXFD,
             _VAL2FLD(USB_OTG_GRXFSIZ_RXFD, size));

  refresh_fifo_start_addresses();
}

/** \brief Configures the TxFIFO of an IN endpoint.
 * \param endpoint_number The number of the IN endpoint we want to configure its
 * TxFIFO \param size The size of the IN endpoint in bytes. \note Any change on
 * any FIFO will update the the registers of all TxFIFOs to adapt the start
 * offsets in the FIFO dedicated memory.
 */
static void configure_txfifo_size(uint8_t endpoint_number, uint16_t size) {
  // Gets the FIFO size in term of 32-bit words.
  size = (size + 3) / 4;

  // Configures the depth of the TxFIFO.
  if (endpoint_number == 0) {
    MODIFY_REG(USB_OTG_FS->DIEPTXF0_HNPTXFSIZ, USB_OTG_TX0FD,
               _VAL2FLD(USB_OTG_TX0FD, size));
  } else {
    MODIFY_REG(USB_OTG_FS->DIEPTXF[endpoint_number - 1], USB_OTG_NPTXFD,
               _VAL2FLD(USB_OTG_NPTXFD, size));
  }

  refresh_fifo_start_addresses();
}

/** \brief Flushes the RxFIFO of all OUT endpoints.
 */
static void flush_rxfifo() {
  SET_BIT(USB_OTG_FS->GRSTCTL, USB_OTG_GRSTCTL_RXFFLSH);
}

/** \brief Flushes the TxFIFO of an IN endpoint.
 * \param endpoint_number The number of an IN endpoint to flush its TxFIFO.
 */
static void flush_txfifo(uint8_t endpoint_number) {
  // Sets the number of the TxFIFO to be flushed and then triggers the flush.
  MODIFY_REG(USB_OTG_FS->GRSTCTL, USB_OTG_GRSTCTL_TXFNUM,
             _VAL2FLD(USB_OTG_GRSTCTL_TXFNUM, endpoint_number) |
                 USB_OTG_GRSTCTL_TXFFLSH);
}

static void configure_endpoint0(uint8_t endpoint_size) {
  // Unmasks all interrupts of IN and OUT endpoint0.
  SET_BIT(USB_OTG_FS_DEVICE->DAINTMSK, 1 << 0 | 1 << 16);

  // Configures the maximum packet size, activates the endpoint, and NAK the
  // endpoint (cannot send data yet).
  MODIFY_REG(IN_ENDPOINT(0)->DIEPCTL, USB_OTG_DIEPCTL_MPSIZ,
             USB_OTG_DIEPCTL_USBAEP |
                 _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, endpoint_size) |
                 USB_OTG_DIEPCTL_SNAK);

  // Clears NAK, and enables endpoint data transmission.
  SET_BIT(OUT_ENDPOINT(0)->DOEPCTL,
          USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK);

  // Note: 64 bytes is the maximum packet size for full speed USB devices.
  configure_rxfifo_size(64);
  configure_txfifo_size(0, endpoint_size);
}

static void configure_in_endpoint(uint8_t endpoint_number,
                                  UsbEndpointType endpoint_type,
                                  uint16_t endpoint_size) {
  // Unmasks all interrupts of the targeted IN endpoint.
  SET_BIT(USB_OTG_FS_DEVICE->DAINTMSK, 1 << endpoint_number);

  // Activates the endpoint, sets endpoint handshake to NAK (not ready to send
  // data), sets DATA0 packet identifier, configures its type, its maximum
  // packet size, and assigns it a TxFIFO.
  MODIFY_REG(
      IN_ENDPOINT(endpoint_number)->DIEPCTL,
      USB_OTG_DIEPCTL_MPSIZ | USB_OTG_DIEPCTL_EPTYP | USB_OTG_DIEPCTL_TXFNUM,
      USB_OTG_DIEPCTL_USBAEP | _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, endpoint_size) |
          USB_OTG_DIEPCTL_SNAK |
          _VAL2FLD(USB_OTG_DIEPCTL_EPTYP, endpoint_type) |
          _VAL2FLD(USB_OTG_DIEPCTL_TXFNUM, endpoint_number) |
          USB_OTG_DIEPCTL_SD0PID_SEVNFRM);

  configure_txfifo_size(endpoint_number, endpoint_size);
}

/** \brief Deconfigures IN and OUT endpoints of a specific endpoint number.
 * \param endpoint_number The number of the IN and OUT endpoints to deconfigure.
 */
static void deconfigure_endpoint(uint8_t endpoint_number) {
  USB_OTG_INEndpointTypeDef *in_endpoint = IN_ENDPOINT(endpoint_number);
  USB_OTG_OUTEndpointTypeDef *out_endpoint = OUT_ENDPOINT(endpoint_number);

  // Disable all interrupts of the targeted IN and OUT endpoints.
  CLEAR_BIT(USB_OTG_FS_DEVICE->DAINTMSK,
            (1 << endpoint_number) | (1 << 16 << endpoint_number));

  // Clears all interrupts of the endpoint.
  SET_BIT(in_endpoint->DIEPINT, 0x29FF);
  SET_BIT(out_endpoint->DOEPINT, 0x71FF);

  // Disables the endpoints if possible.
  if (in_endpoint->DIEPCTL & USB_OTG_DIEPCTL_EPENA) {
    // Disables endpoint transmission.
    SET_BIT(in_endpoint->DIEPCTL, USB_OTG_DIEPCTL_EPDIS);
  }

  // Deactivates the endpoint.
  CLEAR_BIT(in_endpoint->DIEPCTL, USB_OTG_DIEPCTL_USBAEP);

  if (endpoint_number != 0) {
    if (out_endpoint->DOEPCTL & USB_OTG_DOEPCTL_EPENA) {
      // Disables endpoint transmission.
      SET_BIT(out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_EPDIS);
    }

    // Deactivates the endpoint.
    CLEAR_BIT(out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_USBAEP);
  }

  // Flushes the FIFOs.
  flush_txfifo(endpoint_number);
  flush_rxfifo();
}

static void usbrst_handler() {
  log_info("USB reset signal was detected");
  for (uint8_t i = 0; i <= ENDPOINT_COUNT; i++) {
    deconfigure_endpoint(i);
  }
  usb_events.on_usb_reset_received();
}

static void enumdne_handler() {
  log_info("USB device speed enumeration done.");
  configure_endpoint0(8);
}

static void rxflvl_handler() {
  // Pops the status information word from the RxFIFO.
  uint32_t receive_status = USB_OTG_FS_GLOBAL->GRXSTSP;

  // The endpoint that received the data.
  uint8_t endpoint_number = _FLD2VAL(USB_OTG_GRXSTSP_EPNUM, receive_status);
  // The count of bytes in the received packet.
  uint16_t bcnt = _FLD2VAL(USB_OTG_GRXSTSP_BCNT, receive_status);
  // The status of the received packet.
  uint16_t pktsts = _FLD2VAL(USB_OTG_GRXSTSP_PKTSTS, receive_status);

  switch (pktsts) {
    case 0x06:  // SETUP packet (includes data).
      usb_events.on_setup_data_received(endpoint_number, bcnt);
      break;
    case 0x02:  // OUT packet (includes data).
                // ToDo
      break;

    case 0x04:  // SETUP stage has completed.
                // Re-enables the transmission on the endpoint.
      SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPCTL,
              USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
      break;

    case 0x03:  // OUT transfer has completed.
      SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPCTL,
              USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
      break;
  }
}

static void inepint_handler() {
  uint8_t endpoint_number = ffs(USB_OTG_FS_DEVICE->DAINT) - 1;
  if (IN_ENDPOINT(endpoint_number)->DIEPINT & USB_OTG_DIEPINT_XFRC) {
    log_info("Transfer completed on IN Endpoint-%d", endpoint_number);
    usb_events.on_in_transfer_completed(endpoint_number);

    // Clear Interrupt Flag
    SET_BIT(IN_ENDPOINT(endpoint_number)->DIEPINT, USB_OTG_DIEPINT_XFRC);
  }
}

static void outepint_handler() {
  uint8_t endpoint_number = ffs(USB_OTG_FS_DEVICE->DAINT >> 16) - 1;
  if (OUT_ENDPOINT(endpoint_number)->DOEPINT & USB_OTG_DOEPINT_XFRC) {
    log_info("Transfer completed on out Endpoint-%d", endpoint_number);
    usb_events.on_out_transfer_completed(endpoint_number);

    // Clear Interrupt Flag
    SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPINT, USB_OTG_DOEPINT_XFRC);
  }
}

static void gintsts_handler() {
  volatile uint32_t gintsts = USB_OTG_FS_GLOBAL->GINTSTS;

  if (gintsts & USB_OTG_GINTSTS_USBRST) {
    usbrst_handler();
    // Clears the interrupt.
    SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_USBRST);
  } else if (gintsts & USB_OTG_GINTSTS_ENUMDNE) {
    enumdne_handler();
    // Clears the interrupt.
    SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_ENUMDNE);
  } else if (gintsts & USB_OTG_GINTSTS_RXFLVL) {
    rxflvl_handler();
    // Clears the interrupt.
    SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_RXFLVL);

  } else if (gintsts & USB_OTG_GINTSTS_IEPINT) {
    inepint_handler();
    // Clears the interrupt.
    SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_IEPINT);
  } else if (gintsts & USB_OTG_GINTSTS_OEPINT) {
    outepint_handler();
    // Clears the interrupt.
    SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_OEPINT);
  }

  usb_events.on_usb_polled();
}

const UsbDriver usb_driver = {.initialize_core = &initialize_core,
                              .initialize_gpio_pins = &initialize_gpio_pins,
                              .set_device_address = &set_device_address,
                              .connect = &connect,
                              .disconnect = &disconnect,
                              .flush_rxfifo = &flush_rxfifo,
                              .flush_txfifo = &flush_txfifo,
                              .configure_in_endpoint = &configure_in_endpoint,
                              .read_packet = &read_packet,
                              .write_packet = &write_packet,
                              .poll = &gintsts_handler};
