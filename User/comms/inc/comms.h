#ifndef __RECV_H
#define __RECV_H

#include "stdint.h"
#include "USB_Serial/inc/usb_serial.h"
#include "debug.h"
#include "VMIOX/inc/execute.h"
#include "VMIOX/inc/opcodes.h"
#include "ch32v20x.h"
#include "constants.h"

#include "Features/inc/i2c.h"
#include "Features/inc/neopixel.h"
#include "Features/inc/one_wire.h"
#include "Features/inc/spi.h"
#include "Features/inc/pwm.h"
#include "Features/inc/pulse_in.h"
#include "Features/inc/adc.h"
#include "Features/inc/pulse_out.h"
#include "Features/inc/touchkey.h"

#define REGISTER_COUNT          (32)
#define PILE_SIZE               (256)

#define REGISTER_SIZE           (1)
#define PILE_COUNT              (2)

#define MAX_INSTRUCTION_COUNT   (2048)
#define BYTES_PER_INSTRUCTION   (4)
#define MAX_PROGRAM_SIZE_BYTES  (MAX_INSTRUCTION_COUNT * BYTES_PER_INSTRUCTION)

#define I2C_WRITE_BUFFER_MAX    (1024)
#define I2C_READ_BUFFER_MAX     (1024)


#define ESIG_REGISTER_BASE      ((uint32_t*)0x1FFFF7E8)

#define RECV_PING               ((uint8_t)0)
#define RECV_IDENTIFY           ((uint8_t)1)
#define RECV_RUN                ((uint8_t)2)

#define RECV_OUT_PP             ((uint8_t)20)
#define RECV_OUT_OD             ((uint8_t)21)
#define RECV_IN_FLOATING        ((uint8_t)22)
#define RECV_IN_PU              ((uint8_t)23)
#define RECV_IN_PD              ((uint8_t)24)

#define RECV_SET_PIN            ((uint8_t)30)
#define RECV_CLEAR_PIN          ((uint8_t)31)
#define RECV_READ_PIN           ((uint8_t)32)

#define RECV_PWM_INIT           ((uint8_t)119)
#define RECV_PWM_DUTY           ((uint8_t)120)

#define RECV_ADC_INIT           ((uint8_t)121)
#define RECV_ADC_READ           ((uint8_t)122)
#define RECV_ADC_TKEY_READ      ((uint8_t)123)

#define RECV_I2C_INIT           ((uint8_t)140)
#define RECV_I2C_SCAN           ((uint8_t)141)
#define RECV_I2C_READ           ((uint8_t)142)
#define RECV_I2C_WRITE          ((uint8_t)143)
#define RECV_I2C_WRITE_READ     ((uint8_t)144)

#define RECV_PULSEIO_IN_CLEAR      ((uint8_t)151)
#define RECV_PULSEIO_IN_STOP       ((uint8_t)152)
#define RECV_PULSEIO_IN_RESUME     ((uint8_t)153)
#define RECV_PULSEIO_IN_READ       ((uint8_t)154)
#define RECV_PULSEIO_IN_POPLEFT    ((uint8_t)155)
#define RECV_PULSEIO_IN_LENGTH     ((uint8_t)156)

#define RECV_PULSEIO_OUT_SEND      ((uint8_t)160)

#define RECV_NEOPIXEL_WRITE     ((uint8_t)170)

#define RECV_OW_RESET           ((uint8_t)180)
#define RECV_OW_WRITE_0         ((uint8_t)181)
#define RECV_OW_WRITE_1         ((uint8_t)182)
#define RECV_OW_READ            ((uint8_t)183)

#define RECV_SPI_INIT           ((uint8_t)190)
#define RECV_SPI_READ           ((uint8_t)191)
#define RECV_SPI_WRITE          ((uint8_t)192)
#define RECV_SPI_WRITE_READ     ((uint8_t)193)

#define SEND_OK                 (0)
#define SEND_IDENTITY           (1)
#define SEND_MACHINE_STATE      (2)
#define SEND_ADC_READ           (5)
#define SEND_FAILURE            (0xFFFFFFFF)

void get_packet();

#endif