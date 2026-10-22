#include "Magnet.h"  //电磁铁控制代码，原理与蜂鸣器类似

typedef struct
{
    GPIO_Regs *port;
    uint32_t pin;
} Magnet_Channel;

static const Magnet_Channel gMagnetChannels[] = {
    {GPIO_Magnet_PIN_0_PORT, GPIO_Magnet_PIN_0_PIN},
    {GPIO_Magnet_PIN_1_PORT, GPIO_Magnet_PIN_1_PIN},
    {GPIO_Magnet_PIN_2_PORT, GPIO_Magnet_PIN_2_PIN}
};

static const Magnet_Channel *Magnet_GetChannel(Magnet_Id magnet)
{
    if ((uint32_t) magnet >= (sizeof(gMagnetChannels) / sizeof(gMagnetChannels[0]))) {
        return &gMagnetChannels[0];
    }

    return &gMagnetChannels[(uint32_t) magnet];
}

static void Magnet_WriteRaw(Magnet_Id magnet, uint8_t level)
{
    const Magnet_Channel *channel = Magnet_GetChannel(magnet);

    if (level != 0U) {
        DL_GPIO_setPins(channel->port, channel->pin);
    } else {
        DL_GPIO_clearPins(channel->port, channel->pin);
    }
}

static uint8_t Magnet_ReadRaw(Magnet_Id magnet)
{
    const Magnet_Channel *channel = Magnet_GetChannel(magnet);

    return (DL_GPIO_readPins(channel->port, channel->pin) != 0U) ? 1U : 0U;
}

void Magnet_Init(void)
{
    Magnet_AllOff();
}

void Magnet_On(Magnet_Id magnet)
{
#if (MAGNET_ACTIVE_LEVEL == 1U)
    Magnet_WriteRaw(magnet, 1U);
#else
    Magnet_WriteRaw(magnet, 0U);
#endif
}

void Magnet_Off(Magnet_Id magnet)
{
#if (MAGNET_ACTIVE_LEVEL == 1U)
    Magnet_WriteRaw(magnet, 0U);
#else
    Magnet_WriteRaw(magnet, 1U);
#endif
}

void Magnet_Set(Magnet_Id magnet, uint8_t isOn)
{
    if (isOn != 0U) {
        Magnet_On(magnet);
    } else {
        Magnet_Off(magnet);
    }
}

void Magnet_Toggle(Magnet_Id magnet)
{
    uint8_t rawLevel = Magnet_ReadRaw(magnet);

    Magnet_WriteRaw(magnet, (rawLevel == 0U) ? 1U : 0U);
}

void Magnet_AllOn(void)
{
    Magnet_On(MAGNET_ID_0);
    Magnet_On(MAGNET_ID_1);
    Magnet_On(MAGNET_ID_2);
}

void Magnet_AllOff(void)
{
    Magnet_Off(MAGNET_ID_0);
    Magnet_Off(MAGNET_ID_1);
    Magnet_Off(MAGNET_ID_2);
}
