#ifndef _MDRB_UART_DEBUG_H
#define _MDRB_UART_DEBUG_H

#include <MDR_UART.h>
#include <MDRB_BoardSelect.h>

extern const MDR_UART_TypeEx * UART_DBG;

void MDR_UART_DBG_Init(void);
void MDR_UART_DBG_InitEx(uint32_t baudRate, bool RX_Enable);
void MDR_UART_DBG_Finit(void);
void MDR_UART_DBG_ChangeRate(uint32_t baudRate);


#endif  //_MDRB_UART_DEBUG_H

