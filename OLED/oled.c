#include "oled.h"
#include "oledfont.h"
#include "ti/driverlib/m0p/dl_core.h"

enum I2cControllerStatus {
    I2C_STATUS_IDLE = 0,
    I2C_STATUS_TX_STARTED,
    I2C_STATUS_TX_INPROGRESS,
    I2C_STATUS_TX_COMPLETE,
    I2C_STATUS_RX_STARTED,
    I2C_STATUS_RX_INPROGRESS,
    I2C_STATUS_RX_COMPLETE,
    I2C_STATUS_ERROR,
} gI2cControllerStatus;

uint32_t gTxLen, gTxCount, gRxCount, gRxLen;
uint8_t gTxPacket[16];
uint8_t gRxPacket[16];

void delay_ms(unsigned long ms)
{
    while(ms--)
        delay_cycles(CPUCLK_FREQ/1000);
}

void OLED_ColorTurn(uint8_t i)
{
    if(i==0)
    {
        OLED_WR_Byte(0xA6,OLED_CMD);//正常显示
    }
    if(i==1)
    {
        OLED_WR_Byte(0xA7,OLED_CMD);//反色显示
    }
}

void OLED_DisplayTurn(uint8_t i)
{
    if(i==0)
    {
        OLED_WR_Byte(0xC8,OLED_CMD);//正常显示
        OLED_WR_Byte(0xA1,OLED_CMD);
    }
    if(i==1)
    {
        OLED_WR_Byte(0xC0,OLED_CMD);//反转显示
        OLED_WR_Byte(0xA0,OLED_CMD);
    }
}

void OLED_WR_Byte(uint8_t dat,uint8_t mode)
{
    uint16_t i;

    gI2cControllerStatus = I2C_STATUS_IDLE;
    gTxLen = 2;

    if(mode)
        gTxPacket[0] = 0x40;
    else
        gTxPacket[0] = 0x00;
    gTxPacket[1] = dat;

    gTxCount = DL_I2C_fillControllerTXFIFO(I2C_OLED_INST, &gTxPacket[0], gTxLen);

    DL_I2C_disableInterrupt(I2C_OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);

    gI2cControllerStatus = I2C_STATUS_TX_STARTED;
    while (!(DL_I2C_getControllerStatus(I2C_OLED_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C_OLED_INST, 0x3C, DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);

    while ((gI2cControllerStatus != I2C_STATUS_TX_COMPLETE) && (gI2cControllerStatus != I2C_STATUS_ERROR));
    while (DL_I2C_getControllerStatus(I2C_OLED_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(I2C_OLED_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
}

void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f),OLED_CMD);
}

void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}

void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}

void OLED_Clear(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    }
}

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t sizey)
{
    uint8_t c=0,sizex=sizey/2;
    uint16_t i=0,size1;
    if(sizey==8)size1=6;
    else size1=(sizey/8+((sizey%8)?1:0))*(sizey/2);
    c=chr-' ';
    OLED_Set_Pos(x,y);
    for(i=0;i<size1;i++)
    {
        if(i%sizex==0&&sizey!=8) OLED_Set_Pos(x,y++);
        if(sizey==8) OLED_WR_Byte(asc2_0806[c][i],OLED_DATA);//6X8字号
        else if(sizey==16) OLED_WR_Byte(asc2_1608[c][i],OLED_DATA);//8x16字号
        else return;
    }
}

uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}

void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t sizey)
{
    uint8_t t,temp,m=0;
    uint8_t enshow=0;
    if(sizey==8)m=2;
    for(t=0;t<len;t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(sizey/2+m)*t,y,' ',sizey);
                continue;
            }else enshow=1;
        }
        OLED_ShowChar(x+(sizey/2+m)*t,y,temp+'0',sizey);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t sizey)
{
    uint8_t j=0;
    while (chr[j]!='\0')
    {
        OLED_ShowChar(x,y,chr[j++],sizey);
        if(sizey==8)x+=6;
        else x+=sizey/2;
    }
}

void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t no,uint8_t sizey)
{
    uint16_t i,size1=(sizey/8+((sizey%8)?1:0))*sizey;
    for(i=0;i<size1;i++)
    {
        if(i%sizey==0) OLED_Set_Pos(x,y++);
        if(sizey==16) OLED_WR_Byte(Hzk[no][i],OLED_DATA);//16x16字号
        else return;
    }
}

void OLED_DrawBMP(uint8_t x,uint8_t y,uint8_t sizex, uint8_t sizey,uint8_t BMP[])
{
    uint16_t j=0;
    uint8_t i,m;
    sizey=sizey/8+((sizey%8)?1:0);
    for(i=0;i<sizey;i++)
    {
        OLED_Set_Pos(x,i+y);
        for(m=0;m<sizex;m++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}

void OLED_Init(void)
{
    delay_ms(200);
    NVIC_EnableIRQ(I2C_OLED_INST_INT_IRQN);
    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD);//--set SEG output current brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--set segment re-map 0xA0左右反置 0xA1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//set COM scan direction 0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//set pre-charge as 15 clocks & discharge as 1 clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//set VCOM deselect level
    OLED_WR_Byte(0x20,OLED_CMD);//-set page addressing mode
    OLED_WR_Byte(0x02,OLED_CMD);
    OLED_WR_Byte(0x8D,OLED_CMD);//--set charge pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--enable
    OLED_WR_Byte(0xA4,OLED_CMD);//disable entire display on
    OLED_WR_Byte(0xA6,OLED_CMD);//disable inverse display on
    OLED_Clear();
    OLED_WR_Byte(0xAF,OLED_CMD);//display ON
}

void I2C_OLED_INST_IRQHandler(void)
{
    switch (DL_I2C_getPendingInterrupt(I2C_OLED_INST)) {
        case DL_I2C_IIDX_CONTROLLER_RX_DONE:
            gI2cControllerStatus = I2C_STATUS_RX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_TX_DONE:
            DL_I2C_disableInterrupt(
                I2C_OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
            gI2cControllerStatus = I2C_STATUS_TX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_RX_INPROGRESS;
            while (DL_I2C_isControllerRXFIFOEmpty(I2C_OLED_INST) != true) {
                if (gRxCount < gRxLen) {
                    gRxPacket[gRxCount++] =
                        DL_I2C_receiveControllerData(I2C_OLED_INST);
                } else {
                    DL_I2C_receiveControllerData(I2C_OLED_INST);
                }
            }
            break;
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_TX_INPROGRESS;
            if (gTxCount < gTxLen) {
                gTxCount += DL_I2C_fillControllerTXFIFO(
                    I2C_OLED_INST, &gTxPacket[gTxCount], gTxLen - gTxCount);
            }
            break;
        case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
        case DL_I2C_IIDX_CONTROLLER_NACK:
            if ((gI2cControllerStatus == I2C_STATUS_RX_STARTED) ||
                (gI2cControllerStatus == I2C_STATUS_TX_STARTED)) {
                gI2cControllerStatus = I2C_STATUS_ERROR;
            }
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_EMPTY:
        case DL_I2C_IIDX_CONTROLLER_START:
        case DL_I2C_IIDX_CONTROLLER_STOP:
        case DL_I2C_IIDX_CONTROLLER_EVENT1_DMA_DONE:
        case DL_I2C_IIDX_CONTROLLER_EVENT2_DMA_DONE:
        default:
            break;
    }
}

void OLED_SignedShowNum(uint8_t x,uint8_t y,int32_t num,uint8_t len,uint8_t sizey)
{
    if(num == 0 && sizey == 16)
    {
        OLED_ShowChar(x, y, '+', sizey);
        OLED_ShowNum(x + 8, y, 0, len, sizey);
    }
    else if(num == 0 && sizey == 8)
    {
        OLED_ShowChar(x, y, '+', sizey);
        OLED_ShowNum(x + 6, y, 0, len, sizey);
    }
    else if(num > 0 && sizey == 16 )
    {
        OLED_ShowChar(x , y, '+', sizey);
        OLED_ShowNum(x + 8, y, num,len,sizey);
    }
    else if(num > 0 && sizey == 8 )
    {
        OLED_ShowChar(x , y, '+', sizey);
        OLED_ShowNum(x + 6, y, num,len,sizey);
    }
    else if(num < 0 && sizey == 16 )
    {
        OLED_ShowChar(x , y, '-', sizey);
        OLED_ShowNum(x + 8, y, -num,len, sizey);
    }
    else if(num < 0 && sizey == 8 )
    {
        OLED_ShowChar(x , y, '-', sizey);
        OLED_ShowNum(x + 6, y, -num,len,sizey);
    }
}

void OLED_ShowANGLE(float pitch,float roll,float yaw)
{
    OLED_ShowString(0,0,"yaw:",16);
    OLED_SignedShowNum(56,0,(int)yaw,3,16);
}
