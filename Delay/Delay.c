#include "ti_msp_dl_config.h"
#include "Delay.h"

void Delay_ms(uint32_t ms)
{
  while(ms--)
  {
    delay_cycles(CPUCLK_FREQ/1000);
  }   
}

















