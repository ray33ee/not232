#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void     usbSerial_begin(void);
uint8_t  usbSerial_connected(void);

uint16_t usbSerial_available(void);
uint8_t  usbSerial_read(void);

uint16_t usbSerial_writeP(const uint8_t *data, uint16_t len);
void     usbSerial_flush(void);

void     usbSerial_println_s(const char *s);
void     usbSerial_println_i(int32_t v);

void usbSerial_on_ep2_out(uint16_t len);
void usbSerial_on_ep3_in_done(void);

#ifdef __cplusplus
}
#endif
