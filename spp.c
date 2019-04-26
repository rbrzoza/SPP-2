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
 * ====            Simple Packet Protocol version 2                 ====
 * =====================================================================
 * */


#include "spp.h"

extern void sppTxByte(uint8_t b);
extern void sppIsr(void);

#define SPP_START_DELIMITER         0x00
#define SPP_START_DISPATCH          0x80
#define SPP_REPLACE_START_DEL       0x81
#define SPP_REPLACE_START_DISPATCH  0x82

enum{
  SPP_STATE_IDLE = 0,
  SPP_STATE_DSTADDR = 1,
  SPP_STATE_SRCADDR = 2,
  SPP_STATE_LEN = 3,
  SPP_STATE_CMDID = 4,
  SPP_STATE_PAYLOAD = 5,
  SPP_STATE_CRC = 6
};


static volatile uint8_t sppRxState = SPP_STATE_IDLE;
void sppRxedPacket_callback(T_sppPacket* pPacket) __attribute__((weak));
static uint8_t crcByte(uint8_t crc, uint8_t data);
static uint8_t busIsFree(void);
static void txwd(uint8_t b);
static uint8_t checkDstAddress(uint8_t addr);
void sppRxFsm(uint8_t rx);



void sppRxedPacket_callback(T_sppPacket* pPacket)
{
  SPP_MSG("internal callback\n");
}


//the receiver is in idle/ready state: 1=yes, 0=no
static uint8_t busIsFree(void)
{
  if((sppRxState == SPP_STATE_IDLE) && SPP_RX_PIN_IS_HIGH) return 1; else return 0;
}



//tx with dispatch
static void txwd(uint8_t b)
{
  switch(b)
  {
    case SPP_START_DELIMITER:
      sppTxByte(SPP_START_DISPATCH);
      sppTxByte(SPP_REPLACE_START_DEL);
      return;
    
    case SPP_START_DISPATCH:
      sppTxByte(SPP_START_DISPATCH);
      sppTxByte(SPP_REPLACE_START_DISPATCH);
      return;
    
    default:
      sppTxByte(b);
      return;
  }
} 


//returns zero if address is matched
static uint8_t checkDstAddress(uint8_t addr)
{
  //broadcast to all instancess of a class (addr[3:0]==0xF and addr[7:4] matches SPP_ADDRESS[7:4])
  if( ((addr&0x0F)==0x0F) && ((addr&0xF0)==(SPP_ADDRESS&0xF0)) ) return 0;
  
  //broadcast to all devices in the network
  if(addr==0xFF) return 0;
  
  //1:1 match
  if( addr==SPP_ADDRESS ) return 0;
  
  //mismatch
  return 1;
}


static uint8_t crcByte(uint8_t crc, uint8_t data)
{
  uint8_t i;
  crc = crc ^ data;
  for(i=0;i<8;i++)
  {
    if(crc & 0x01)
      crc = (crc >> 1) ^ 0x8C;
    else
      crc >>= 1;
  }
  return(crc);
}



/*
 * sppTx - send packet. The following packet contents should be prepared before call:
 *    dstAddr, len, cmdid, payload (only if len>0)
 * returns:
 *   =0 if the tx failed due to high bus load, RX FSM gets reset then
 *   <0 if the tx failed because a collision has been detected during tx, the retured value tells at which stage the collision might occur
 *   >0 if the tx was successfull, the returned value contains the number of back-off iterations
 * */
int8_t sppTx(T_sppPacket* pPacket)
{
  SPP_LED_TX_ON;
  int i;
  uint8_t crc = 0;
  
  for(i=1;i<SPP_BACK_OFF_MAX;i++)
  {
    if(busIsFree())
    {
      //take over the bus
      sppTxByte(SPP_START_DELIMITER);
      break;
    }
    else
    {
      SPP_DELAY_MS(1<<i);
    }
  }
  
  if(i==SPP_BACK_OFF_MAX)
  {
    SPP_LED_TX_OFF;
    SPP_MSG("TX timeout!\n");
    
    //force FSM reset
    sppRxState = SPP_STATE_IDLE;
    return 0;
  }
  
  
  if(!busIsFree()) {i = -1; goto COLLISION_DETECTED;}
  txwd(pPacket->dstAddr);
  crc = crcByte(crc,pPacket->dstAddr);
  
  if(!busIsFree()) {i = -2; goto COLLISION_DETECTED;}
  txwd(SPP_ADDRESS);
  crc = crcByte(crc,SPP_ADDRESS);
  
  if(!busIsFree()) {i = -3; goto COLLISION_DETECTED;}
  txwd(~(pPacket->len));
  crc = crcByte(crc,pPacket->len);
  
  if(!busIsFree()) {i = -4; goto COLLISION_DETECTED;}
  txwd(~(pPacket->cmdid));
  crc = crcByte(crc,pPacket->cmdid);
  
  for(uint8_t i=0;i<pPacket->len;i++)
  {
    if(!busIsFree()) {i = -5; goto COLLISION_DETECTED;}
    txwd(~pPacket->payload[i]);
    crc = crcByte(crc,pPacket->payload[i]);
  }
  
  if(!busIsFree()) {i = -6; goto COLLISION_DETECTED;}
  txwd(crc);
  
  SPP_LED_TX_OFF;
  return i;
  
  COLLISION_DETECTED:
  SPP_MSG("SPP coll. res=%d\n",i);
  sppRxState = SPP_STATE_IDLE;
  SPP_LED_BUSY_ON;  //both LEDs on (if present) after collision
  SPP_DELAY_MS( SPP_DELAY_AFTER_COLLISION );
  SPP_LED_TX_OFF;
  SPP_LED_BUSY_OFF;
  return i;
}



void sppRxFsm(uint8_t rx)
{
  //SPP_MSG("%02X",rx);
  
  static const uint8_t DISPATCH_BIT = 0;
  static const uint8_t MISMATCH_BIT = 1;
  
  //status bit 0: dispatch
  //status bit 1: mismatch
  static uint8_t status;

  static uint8_t crc;
  static uint8_t payloadIdx;
  static T_sppPacket rxPacket;  //rx buffer
  
  
  switch(rx)
  {
    case SPP_START_DELIMITER:  //rxed start byte
      sppRxState = SPP_STATE_DSTADDR;
      SPP_LED_BUSY_ON;
      status &= ~(1<<DISPATCH_BIT);
      status &= ~(1<<MISMATCH_BIT);
      
      return; //exit!
    
    case SPP_START_DISPATCH:
      status |= (1<<DISPATCH_BIT);
      return; //exit!
    
    case SPP_REPLACE_START_DEL:
      if( status & (1<<DISPATCH_BIT) ) rx = SPP_START_DELIMITER;
      break;
      
    case SPP_REPLACE_START_DISPATCH:
      if( status & (1<<DISPATCH_BIT) ) rx = SPP_START_DISPATCH;
      break;
  }
  
  status &= ~(1<<DISPATCH_BIT);
  
  switch(sppRxState)
  {
    case SPP_STATE_DSTADDR:
      #if SPP_SNIFF_MODE
        status &= ~(1<<MISMATCH_BIT);
      #else
        if(checkDstAddress(rx))
          status |= (1<<MISMATCH_BIT);
        else
          status &= ~(1<<MISMATCH_BIT);
      #endif
      rxPacket.dstAddr=rx;
      crc = crcByte(0,rx);  //start computing crc
      sppRxState = SPP_STATE_SRCADDR;
      break;
    
    case SPP_STATE_SRCADDR:
      rxPacket.srcAddr=rx;
      crc = crcByte(crc,rx);
      sppRxState = SPP_STATE_LEN;
      break;
    
    case SPP_STATE_LEN:
      rx = ~rx;
      if(rx > (SPP_PAYLOAD_LEN-1))
      {
        status |= (1<<MISMATCH_BIT);
      }
      rxPacket.len = rx;
      sppRxState = SPP_STATE_CMDID;
      crc = crcByte(crc,rx);
      break;

    case SPP_STATE_CMDID:
      rx = ~rx;
      rxPacket.cmdid = rx;
      payloadIdx = 0;
      sppRxState = SPP_STATE_PAYLOAD;
      crc = crcByte(crc,rx);
      break;
    
    case SPP_STATE_PAYLOAD:
      if(payloadIdx>=rxPacket.len)  //end of payload?
      {
        #if !SPP_IGNORE_CRC
        if(crc != rx)
        {
          //Discard packet and go to IDLE state
          sppRxState = SPP_STATE_IDLE;
          SPP_LED_BUSY_OFF;
          return;
        }
        #endif
        
        if( (status & (1<<MISMATCH_BIT)) == 0 )
        {
          sppRxedPacket_callback(&rxPacket);
        }
        
        sppRxState = SPP_STATE_IDLE;
        SPP_LED_BUSY_OFF;
        
        return;
        
      }
      
      //is the packet for me?
      if( (status & (1<<MISMATCH_BIT)) == 0 )
      {
        //write rxed bytes to the buffer and advance the pointer
        rx = ~rx;
        if(payloadIdx < (SPP_PAYLOAD_LEN-1))
          rxPacket.payload[payloadIdx++] = rx;
      }
      else
      {
        //keep track of a number of bytes rxed
        payloadIdx++;
      }
      
      crc = crcByte(crc,rx);
      break;
  }
  

}

