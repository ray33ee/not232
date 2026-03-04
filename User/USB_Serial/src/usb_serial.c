#include "USB_Serial/inc/usb_serial.h"
#include "USB_Serial/inc/ch32v20x_usbfs_device.h"
#include <string.h>

#include "comms/inc/comms.h"

/* CDC endpoints per usb_desc.c:
   EP2 OUT  (0x02) bulk OUT
   EP3 IN   (0x83) bulk IN
*/

#define RX_RING_SIZE (2048)
#define TX_RING_SIZE (2048)

static volatile uint16_t rx_head = 0, rx_tail = 0;
static volatile uint16_t tx_head = 0, tx_tail = 0;

static uint8_t rx_ring[RX_RING_SIZE];
static uint8_t tx_ring[TX_RING_SIZE];

static volatile uint8_t tx_pending_kick = 0;

/* We keep UEP2 DMA pointed at USBFS_EP2_Buf (already set in USBFS_Device_Endp_Init)
   and copy from USBFS_EP2_Buf when OUT packets arrive. */

static inline uint16_t ring_count(uint16_t head, uint16_t tail, uint16_t size)
{
    return (head >= tail) ? (head - tail) : (uint16_t)(size - (tail - head));
}

static inline uint16_t ring_free(uint16_t head, uint16_t tail, uint16_t size)
{
    return (uint16_t)(size - 1 - ring_count(head, tail, size));
}

static void tx_kick(void)
{
    if(!usbSerial_connected())
        return;

    if(USBFS_Endp_Busy[DEF_UEP3])
        return;

    uint16_t cnt = ring_count(tx_head, tx_tail, TX_RING_SIZE);
    if(cnt == 0)
        return;

    uint8_t pkt[DEF_USBD_FS_PACK_SIZE]; /* 64 */
    uint16_t n = (cnt > sizeof(pkt)) ? (uint16_t)sizeof(pkt) : cnt;

    /* Pop n bytes from tx ring into pkt */
    for(uint16_t i = 0; i < n; i++)
    {
        pkt[i] = tx_ring[tx_tail];
        tx_tail = (uint16_t)((tx_tail + 1) % TX_RING_SIZE);
    }

    /* Send on EP3 IN */
    (void)USBFS_Endp_DataUp(DEF_UEP3, pkt, n, DEF_UEP_CPY_LOAD);
}

void usbSerial_begin(void)
{
    /* Your project already does clock + init similarly in main.c.
       Minimal init needed for USBFS CDC device:
    */
    USBFS_RCC_Init();
    USBFS_Device_Init(ENABLE);

    rx_head = rx_tail = 0;
    tx_head = tx_tail = 0;
}

uint8_t usbSerial_connected(void)
{
    /* This example sets DevEnumStatus=1 on SET_CONFIGURATION */
    return (USBFS_DevEnumStatus == 0x01) && (USBFS_DevConfig != 0);
}

uint16_t usbSerial_available(void)
{
    return ring_count(rx_head, rx_tail, RX_RING_SIZE);
}

uint8_t usbSerial_read(void)
{
    if(rx_head == rx_tail)
        return 0;

    uint8_t b = rx_ring[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1) % RX_RING_SIZE);
    return b;
}

uint16_t usbSerial_writeP(const uint8_t *data, uint16_t len)
{
    uint16_t written = 0;

    /* Enqueue into TX ring */
    while(written < len)
    {
        uint16_t free = ring_free(tx_head, tx_tail, TX_RING_SIZE);
        if(free == 0)
            break;

        tx_ring[tx_head] = data[written];
        tx_head = (uint16_t)((tx_head + 1) % TX_RING_SIZE);
        written++;
    }

    /* Try to start sending immediately */
    tx_kick();
    return written;
}

void usbSerial_flush(void)
{
    /* Non-blocking flush: keep kicking once.
       Your main loop already delays, so this is enough.
    */
    tx_kick();
}

void usbSerial_println_s(const char *s)
{
    usbSerial_writeP((const uint8_t*)s, (uint16_t)strlen(s));
    static const uint8_t nl[2] = {'\r','\n'};
    usbSerial_writeP(nl, 2);
}

static void i32_to_str(int32_t v, char *out, size_t out_sz)
{
    if(out_sz == 0) return;
    char tmp[16];
    int pos = 0;

    uint32_t x;
    if(v < 0) { x = (uint32_t)(-v); }
    else      { x = (uint32_t)v; }

    do {
        tmp[pos++] = (char)('0' + (x % 10u));
        x /= 10u;
    } while(x && pos < (int)sizeof(tmp));

    size_t idx = 0;
    if(v < 0 && idx + 1 < out_sz) out[idx++] = '-';

    while(pos > 0 && idx + 1 < out_sz)
        out[idx++] = tmp[--pos];

    out[idx] = '\0';
}

void usbSerial_println_i(int32_t v)
{
    char buf[20];
    i32_to_str(v, buf, sizeof(buf));
    usbSerial_println_s(buf);
}

/* ====== Hooks called from the USB interrupt ======
   You must call these from USBHD_IRQHandler in ch32v20x_usbfs_device.c
*/
void usbSerial_on_ep2_out(uint16_t len)
{
    /* Copy from USBFS_EP2_Buf into rx ring */
    for(uint16_t i = 0; i < len; i++)
    {
        uint16_t free = ring_free(rx_head, rx_tail, RX_RING_SIZE);
        if(free == 0) break;

        rx_ring[rx_head] = USBFS_EP2_Buf[i];
        rx_head = (uint16_t)((rx_head + 1) % RX_RING_SIZE);
    }
}

void usbSerial_on_ep3_in_done(void)
{
    /* previous IN completed; send next chunk if pending */
    //tx_kick();
    tx_pending_kick  = 1;
}
