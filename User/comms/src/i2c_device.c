#include "comms/inc/i2c_device.h"



/*void i2c_device_init(uint8_t address) {

    GPIO_InitTypeDef GPIO_InitStructure={0};
    I2C_InitTypeDef I2C_InitTSturcture={0};

    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    I2C_InitTSturcture.I2C_ClockSpeed = 100000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );

    I2C_Cmd( I2C1, ENABLE );
}*/

void i2c_device_init(uint8_t address)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitTSturcture = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTSturcture.I2C_ClockSpeed = 100000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitTSturcture);

    I2C_Cmd(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

void i2c_device_read(uint8_t *buffer, uint32_t size) {
    uint32_t i = 0;

    if (size == 0) {
        return;
    }

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET) {
    }

    /* clear ADDR */
    (void)I2C1->STAR1;
    (void)I2C1->STAR2;

    while (1) {
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) != RESET) {
            uint8_t b = I2C_ReceiveData(I2C1);
            if (i < size) {
                buffer[i++] = b;
            }
        }

        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) != RESET) {
            (void)I2C1->STAR1;
            I2C1->CTLR1 |= I2C_CTLR1_PE;   /* clear STOPF */
            break;
        }
    }
}

void i2c_device_write(uint8_t *buffer, uint32_t size) {
    uint32_t i = 0;

    if (size == 0) {
        return;
    }

    /* Wait for a completely separate READ transaction: SLA+R */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED)) {
    }

    while (1) {
        if ((I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE) != RESET) && (i < size)) {
            I2C_SendData(I2C1, buffer[i]);
            i++;
        }

        /* Master NACKed final byte */
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF) != RESET) {
            I2C_ClearFlag(I2C1, I2C_FLAG_AF);
            break;
        }

        /* Also tolerate STOP */
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) != RESET) {
            (void)I2C1->STAR1;
            I2C_Cmd(I2C1, ENABLE);
            break;
        }
    }
}