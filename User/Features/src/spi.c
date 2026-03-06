#include "Features/inc/spi.h"


static inline void spi_delay_nops(uint32_t n)
{
    while (n--) __NOP();
}

static inline void gpio_write_fast(uint16_t mask, uint8_t level)
{
    if (level) GPIOA->BSHR = mask;
    else       GPIOA->BCR  = mask;
}

static inline uint8_t gpio_read_fast(uint16_t mask)
{
    return (GPIOA->INDR & mask) ? 1u : 0u;
}

uint8_t spi_xfer_byte_bb_ch32(uint8_t sck_pin,
                              uint8_t mosi_pin,
                              uint8_t miso_pin,
                              uint8_t out,
                              uint8_t mode,
                              uint32_t edge_delay_nops)
{

    uint8_t cpha = mode & 1;
    uint8_t cpol = mode >> 1;

    const uint16_t sck_mask  = (uint16_t)(1u << sck_pin);
    const uint16_t mosi_mask = (uint16_t)(1u << mosi_pin);
    const uint16_t miso_mask = (uint16_t)(1u << miso_pin);

    const uint8_t idle   = (cpol != 0u) ? 1u : 0u;
    const uint8_t active = (uint8_t)(idle ^ 1u);

    uint8_t in = 0;

    // Start at idle and hold for half-cycle (helps consistent first pulse width)
    gpio_write_fast(sck_mask, idle);
    spi_delay_nops(edge_delay_nops);

    if (cpha == 0u) {
        // CPHA=0: set MOSI before leading edge, sample on leading edge
        for (uint8_t i = 0u; i < 8u; i++) {
            // Drive MOSI bit
            gpio_write_fast(mosi_mask, (out & 0x80u) ? 1u : 0u);
            out <<= 1;

            // Leading edge -> active
            gpio_write_fast(sck_mask, active);
            spi_delay_nops(edge_delay_nops);

            // Sample MISO
            in = (uint8_t)((in << 1) | gpio_read_fast(miso_mask));

            // Trailing edge -> idle
            gpio_write_fast(sck_mask, idle);
            spi_delay_nops(edge_delay_nops);
        }
    } else {
        // CPHA=1: leading edge first, sample on trailing edge
        for (uint8_t i = 0u; i < 8u; i++) {
            // Leading edge -> active
            gpio_write_fast(sck_mask, active);
            spi_delay_nops(edge_delay_nops);

            // Drive MOSI bit (changes during active phase)
            gpio_write_fast(mosi_mask, (out & 0x80u) ? 1u : 0u);
            out <<= 1;

            // Trailing edge -> idle (sample edge)
            gpio_write_fast(sck_mask, idle);
            spi_delay_nops(edge_delay_nops);

            // Sample MISO
            in = (uint8_t)((in << 1) | gpio_read_fast(miso_mask));
        }
    }

    return in;
}

void spi_init(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin) {
    gpio_init_ad_pins(sck_pin, GPIO_Mode_Out_PP);
    gpio_init_ad_pins(mosi_pin, GPIO_Mode_Out_PP);
    gpio_init_ad_pins(miso_pin, GPIO_Mode_IN_FLOATING);
}

void spi_read(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint8_t write_value, uint32_t delay, uint8_t* buffer, uint32_t len) {
    for (int i = 0; i < len; i++) {
        buffer[i] = spi_xfer_byte_bb_ch32(sck_pin, mosi_pin, miso_pin, write_value, mode, delay);
    }
}

void spi_write(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint32_t delay, uint8_t* buffer, uint32_t len) {
    for (int i = 0; i < len; i++) {
        spi_xfer_byte_bb_ch32(sck_pin, mosi_pin, miso_pin, buffer[i], mode, delay);
    }
}

void spi_write_read(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint32_t delay, uint8_t* out_buffer, uint8_t* in_buffer, uint32_t len) {
    for (int i = 0; i < len; i++) {
        in_buffer[i] = spi_xfer_byte_bb_ch32(sck_pin, mosi_pin, miso_pin, out_buffer[i], mode, delay);
    }
}