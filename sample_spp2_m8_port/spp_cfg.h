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


#ifndef __SPP_CFG_H__
#define __SPP_CFG_H__

#include "spp.h"
#include "spp_cmdid.h"
#include "board.h"
#include "delay.h"


#define SPP_DELAY_MS(a)     delay_ms(a)

#define SPP_RX_PIN_IS_HIGH (PIND & (1<<PD0))

#define SPP_BACK_OFF_MAX  12

#define SPP_DELAY_AFTER_COLLISION  52

#define SPP_PAYLOAD_LEN   40

#define SPP_ADDRESS       0x40

//ignore wrong CRC sums
#define SPP_IGNORE_CRC            0

//ignore destination address
#define SPP_SNIFF_MODE            0

#define SPP_LED_BUSY_ON   LED_3_ON;
#define SPP_LED_BUSY_OFF  LED_3_OFF;

#define SPP_LED_TX_ON     SPP_LED_BUSY_ON
#define SPP_LED_TX_OFF    SPP_LED_BUSY_OFF


#define SPP_MSG           





#endif


