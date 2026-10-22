// #include "UART.h"

// VISION_DATA Vision;

// typedef enum
// {
//     WAIT_HEAD1 = 0,
//     WAIT_HEAD2,
//     WAIT_DATA

// }UART_STATE;

// static UART_STATE State = WAIT_HEAD1;

// static uint8_t RxData[7];

// static uint8_t cnt = 0;



// void UART0_Init(void)
// {
//     NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
//     NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
// }



// void UART0_IRQHandler(void)
// {
//     uint8_t ch;

//     switch(DL_UART_Main_getPendingInterrupt(UART_0_INST))
//     {

//     case DL_UART_MAIN_IIDX_RX:

//         ch = DL_UART_Main_receiveData(UART_0_INST);

//         switch(State)
//         {

//         case WAIT_HEAD1:

//             if(ch == 0xAA)
//             {
//                 RxData[0]=ch;
//                 State=WAIT_HEAD2;
//             }

//             break;


//         case WAIT_HEAD2:

//             if(ch==0x55)
//             {
//                 RxData[1]=ch;

//                 cnt=2;

//                 State=WAIT_DATA;
//             }
//             else
//             {
//                 State=WAIT_HEAD1;
//             }

//             break;


//         case WAIT_DATA:

//             RxData[cnt++] = ch;

//             if(cnt>=7)
//             {

//                 State=WAIT_HEAD1;

//                 uint8_t checksum;

//                 checksum =
//                         RxData[0]^
//                         RxData[1]^
//                         RxData[2]^
//                         RxData[3]^
//                         RxData[4]^
//                         RxData[5];

//                 if(checksum==RxData[6])
//                 {

//                     Vision.err_x =
//                             (int16_t)((RxData[3]<<8)|RxData[2]);

//                     Vision.err_y =
//                             (int16_t)((RxData[5]<<8)|RxData[4]);

//                     Vision.receive_ok=1;
//                 }

//             }

//             break;


//         default:

//             State=WAIT_HEAD1;

//             break;

//         }

//         break;

//     default:

//         break;

//     }

// }