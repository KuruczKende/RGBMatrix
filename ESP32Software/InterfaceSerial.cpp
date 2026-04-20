#include "InterfaceSerial.h"
#include "Disp.h"
#include "USBCDC.h"

// Current logic only works if 1 packet is 1 byte, thats limiting a color to 2 bit
#if BR_LVL == 1
  // I'm not doing this logic
#endif
#if (BR_LVL == 2) || (BR_LVL == 3)
  #define BIT_FOR_CHECK (2)
  #define CHECK_MASK (0b00000011)
  #define COLOR_MASK (0b00000011)
  #define BIT_PER_COLOR (2)
  #define BIT_PER_PIX (3*BIT_PER_COLOR+BIT_FOR_CHECK)
#endif

#define Serial USBSerial

// datatype for a frame packet
#define FRAMESIZE (BIT_PER_PIX*COL_NUM*ROW_NUM/8)
//*
// Where the transfer is currently

USBCDC USBSerial = USBCDC();
static uint8_t u8PackIdx;
static uint8_t u8NextCheckVal;

static bool boITFSER_ReceivePack(uint8_t u8Pack, uint8_t* pu8ErrorIdx);

bool ITFSER_Init(){
  Serial.begin(115200);

  u8PackIdx = 0;
  u8NextCheckVal = 0;

}

void vITFSER_Task(){
  uint8_t u8ErrorByte;
  if(Serial.available()){
    if(boITFSER_ReceivePack(Serial.read(),&u8ErrorByte) == false){
      // error handling the incomming byte
      // send the requested index
      Serial.write(u8ErrorByte);
      // trash the input
      while(Serial.available()){
        (void)Serial.read();
      }
      // Send signal to ready to receive
      Serial.write(u8ErrorByte);
    }
  }
}

static bool boITFSER_ReceivePack(uint8_t u8Pack, uint8_t* pu8ErrorIdx){
  *pu8ErrorIdx = u8PackIdx; // Index of expected Byte
  if((u8Pack&CHECK_MASK) != u8NextCheckVal){
    return false;
  }
  bool boRet = boDISP_SetColor(u8PackIdx%ROW_NUM,u8PackIdx/COL_NUM,
                (u8Pack>>(2*BIT_PER_COLOR+BIT_FOR_CHECK))&COLOR_MASK,
                (u8Pack>>(1*BIT_PER_COLOR+BIT_FOR_CHECK))&COLOR_MASK,
                (u8Pack>>(0*BIT_PER_COLOR+BIT_FOR_CHECK))&COLOR_MASK);
  if(boRet == false){
    return false;
  }
  u8PackIdx++;
  u8NextCheckVal = u8PackIdx&CHECK_MASK;
  return true;
}



//*/