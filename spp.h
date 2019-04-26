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

#ifndef __SPP_H__
#define __SPP_H__

#include <inttypes.h>
#include <string.h>

#include "spp_cfg.h"

#define SPP_BCAST_IN_CLASS      0x0F
#define SPP_BCAST_ALL           0xFF

typedef struct __attribute__((packed))
{
  uint8_t dstAddr;
  uint8_t srcAddr;
  uint8_t len;
  uint8_t cmdid;
  uint8_t payload[SPP_PAYLOAD_LEN];
}T_sppPacket;


void sppInit(void* params);
void sppIsr(void);
uint8_t sppRx(T_sppPacket* pPacket);
int8_t sppTx(T_sppPacket* pPacket);


#endif

