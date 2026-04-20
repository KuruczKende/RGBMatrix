extern "C" {
  #include "Disp.h"
}
// Sem18
uint8_t const aaCRed[ROW_NUM][COL_NUM] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
  {0,1,1,1,0,1,1,1,1,0,1,0,0,1,0,1},
  {0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,1},
  {1,1,0,1,0,1,1,1,1,0,1,0,0,1,0,1},
  {0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};
uint8_t const aaCGreen[ROW_NUM][COL_NUM] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1},
  {0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1},
  {1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,1},
  {0,0,0,0,1,1,0,1,1,0,1,1,0,0,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,0,0,0,0,1},
  {1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,1},
};
uint8_t const aaCBlue[ROW_NUM][COL_NUM] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
  {0,1,1,1,0,1,1,1,1,0,1,0,0,1,0,1},
  {0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,1},
  {1,1,0,1,0,1,1,1,1,0,1,0,0,1,0,1},
  {0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,1},
  {0,0,0,0,1,1,0,1,1,0,1,1,0,0,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,0,0,0,0,1},
  {1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1},
  {1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,1},
};
// Sem18 END

// Global variables
extern bool gboBufferReady; // Set by CPU indicating the ChangeBuffer is ready to send out
extern bool gboBufferSwitched; // Set by SPI+DMA indicating the ChangeBuffer is dirty, clean it before use

// Main
uint32_t last;
void setup() {
  boDISP_Init();
  last=millis();
  for(uint8_t i=0;i<COL_NUM;i++){
    for(uint8_t j=0;j<ROW_NUM;j++){
      uint8_t pwr = 3;
      if(i<16) pwr = 1;
      boDISP_SetColor(j, i, pwr * aaCRed[j][i],pwr * aaCGreen[j][i],pwr * aaCBlue[j][i]);
    }
  }
  delay(100);
  gboBufferReady = true;
  boDISP_SendBuffer();
}
uint16_t n=0;
void loop() {
  delay(1000);

  // Clear buffer before use
  if(gboBufferSwitched == true){
    gboBufferSwitched = false;
    vClearBuffer();
  }
  // Check if the buffer isn't the one ready to be sent
  if(gboBufferReady == false){
    // Buffer can be modified here

    // Trigger the switch if the buffer is ready to be sent out
    // gboBufferReady = true;
  }

  /*if(millis()>last+1000){
    last=millis();
    n++;
    if(n>=COL_NUM)n=0;
  }*/
}
