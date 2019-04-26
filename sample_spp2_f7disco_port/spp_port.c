/*
* The MIT License (MIT)
* Copyright (c) 2019 Robert Brzoza-Woch
* Permission is hereby granted, free of charge, to any person obtaining 
* a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * =====================================================================
 * ====            Simple Packet Protocol version 2.0               ====
 * =====================================================================
 * */


#include "spp.h"


extern void sppRxFsm(uint8_t rx);

UART_HandleTypeDef* pSpp_Uart;


typedef enum {
  SPP_DIR_PWRDOWN,
  SPP_DIR_TX,
  SPP_DIR_RX
} T_Spp_dir;

static void sppDir(T_Spp_dir dir);

//required
void sppInit(void* params)
{
  pSpp_Uart = (UART_HandleTypeDef*)params;
  sppDir(SPP_DIR_RX);
  __HAL_UART_DISABLE_IT(pSpp_Uart,UART_IT_CTS | UART_IT_LBD | UART_IT_TXE | UART_IT_TC | UART_IT_RXNE | UART_IT_IDLE | UART_IT_PE | UART_IT_ERR | UART_IT_ORE);
  __HAL_UART_ENABLE_IT(pSpp_Uart,UART_IT_RXNE);
}


//required
void sppTxByte(uint8_t b)
{
  sppDir(SPP_DIR_TX);
  HAL_UART_Transmit(pSpp_Uart,
  (uint8_t*)&b, 1, 0xFFFF);
  sppDir(SPP_DIR_RX);
}


//required
void sppIsr(void)
{
  char b = pSpp_Uart->Instance->RDR;
  sppRxFsm(b);
  __HAL_UART_CLEAR_OREFLAG(pSpp_Uart);
}


static void sppDir(T_Spp_dir dir)
{
  switch(dir)
  {
    case SPP_DIR_TX:
    {
      #if (defined(SPP_RE_GPIO) && defined(SPP_RE_PIN))
        HAL_GPIO_WritePin(SPP_RE_GPIO, SPP_RE_PIN, GPIO_PIN_SET);
      #endif
      HAL_GPIO_WritePin(SPP_DE_GPIO, SPP_DE_PIN, GPIO_PIN_SET);
      break;
    }
    case SPP_DIR_RX:
    {
      #if (defined(SPP_RE_GPIO) && defined(SPP_RE_PIN))
        HAL_GPIO_WritePin(SPP_RE_GPIO, SPP_RE_PIN, GPIO_PIN_RESET);
      #endif
      HAL_GPIO_WritePin(SPP_DE_GPIO, SPP_DE_PIN, GPIO_PIN_RESET);
      break;
    }
    default:
    {
      #if (defined(SPP_RE_GPIO) && defined(SPP_RE_PIN))
        HAL_GPIO_WritePin(SPP_RE_GPIO, SPP_RE_PIN, GPIO_PIN_SET);
      #endif
      HAL_GPIO_WritePin(SPP_DE_GPIO, SPP_DE_PIN, GPIO_PIN_RESET);
      break;
    }
  }
}


