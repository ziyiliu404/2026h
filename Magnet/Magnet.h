#ifndef MAGNET_H
#define MAGNET_H

#include <stdint.h>

#include "ti_msp_dl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set to 1U if the electromagnet is enabled by a high level.
 * Set to 0U if the electromagnet is enabled by a low level.
 *
 * Your current SysConfig default output is high, so 0U is safer
 * if you want the electromagnet to stay off after power-up.
 */
#ifndef MAGNET_ACTIVE_LEVEL
#define MAGNET_ACTIVE_LEVEL    0U
#endif

typedef enum
{
    MAGNET_ID_0 = 0,
    MAGNET_ID_1 = 1,
    MAGNET_ID_2 = 2
} Magnet_Id;

void Magnet_Init(void);
void Magnet_On(Magnet_Id magnet);
void Magnet_Off(Magnet_Id magnet);
void Magnet_Set(Magnet_Id magnet, uint8_t isOn);
void Magnet_Toggle(Magnet_Id magnet);
void Magnet_AllOn(void);
void Magnet_AllOff(void);

#ifdef __cplusplus
}
#endif

#endif /* MAGNET_H */
