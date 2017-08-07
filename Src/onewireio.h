#ifndef _ONEWIREIO_H
#define _ONEWIREIO_H

#include <stdint.h>
#include "common.h"
/*Global variables*/



void Read();
void Write(unsigned char byte);
void Write_SendArray(uint8_t* data, int length);
void owSetUpRxIT();
void owUartTxDma(uint8_t data);
void owUartTx(uint8_t data);
uint8_t OW_UartRx();
int isUartFrameError();
void setUartBaudRate(int baudRate);
void resetUart(int baudRate);
#endif // _ONEWIREIO_H
