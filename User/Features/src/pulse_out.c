#include "Features/inc/pulse_out.h"

void pulseio_out_send(uint8_t pin_number, uint8_t duty, uint16_t* pulses, uint32_t pulse_count) {
    for (int i = 0; i < pulse_count; i++) {
        pulses[i] = (uint16_t)usbSerial_blocking_read_u32();
    }

    for (int i = 0; i < pulse_count; i++) {
        if (i & 1) {
            //Odd index => duty = 0
            pwm_set_duty(pin_number, 0);
        } else {
            //Even index => duty = duty
            pwm_set_duty(pin_number, duty);
        }
        Delay_Us(pulses[i]);
    }

    pwm_set_duty(pin_number, 0);
}