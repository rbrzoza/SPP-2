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


typedef enum {
  SPP_DIR_PWRDOWN,
  SPP_DIR_TX,
  SPP_DIR_RX
} T_Spp_dir;

static void sppDir(T_Spp_dir dir);

//compatibility defs
#define SPP_UCSRA           UCSR0A
#define SPP_UCSRB           UCSR0B
#define SPP_UCSRC           UCSR0C
#define SPP_UBRRH           UBRR0H
#define SPP_UBRRL           UBRR0L
#define SPP_UDR             UDR0
#define SPP_PE              PE0
#define SPP_RXEN            RXEN0
#define SPP_TXEN            TXEN0
#define SPP_UDRE            UDRE0
#define SPP_RXC             RXC0
#define SPP_TXC             TXC0
#define SPP_RXCIE           RXCIE0
#define SPP_TXCIE           TXCIE0
#define SPP_USART_RXC_vect  USART_RX_vect
#define SPP_USART_TXC_vect  USART_TX_vect
#define SPP_UBRR_VALUE      23


//required
void sppInit(void* params)
{
  params = params;  //not used here
  
  SPP_UCSRA = (1<<SPP_TXC);
  SPP_UCSRB = (1<<SPP_RXCIE)|(1<<SPP_RXEN)|(1<<SPP_TXEN);
  SPP_UBRRL = SPP_UBRR_VALUE;
  SPP_UBRRH = 0;
  SPP_UCSRC = (1<<7)|(1<<2)|(1<<1);
  DDRD |= (1<<PD1) | (1<<PD2);
  SPP_UCSRB |= (1<<SPP_RXCIE);
}


//required
void sppTxByte(uint8_t b)
{
  loop_until_bit_is_set(SPP_UCSRA,SPP_UDRE);
  sppDir(SPP_DIR_TX);
  SPP_UDR = b;
  loop_until_bit_is_set(SPP_UCSRA,SPP_TXC);
  SPP_UCSRA |= (1<<SPP_TXC);
  sppDir(SPP_DIR_RX);
}



//required (call to sppRxFsm)
ISR(SPP_USART_RXC_vect)
{
  uint8_t b = SPP_UDR;
  sppRxFsm(b);
}

static void sppDir(T_Spp_dir dir)
{
  switch(dir)
  {
    case SPP_DIR_TX:
    {
      PORTD |= (1<<PD2);
      break;
    }
    case SPP_DIR_RX:
    {
      PORTD &= ~(1<<PD2);
      break;
    }
    default:
    {
      break;
    }
  }
}



