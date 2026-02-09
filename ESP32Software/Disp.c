#include "Disp.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_system.h"

// Number of Columns 
#define BUFF_SIZE (COL_NUM * BR_LVL + 1)
// Set color simplify define
#define C_SET(n) case n: psRef[(n-1)*COL_NUM*2] &= ~mask;

// Defines for the Shift register
#define SH_0A 14
#define SH_0B 15
#define SH_0C 12
#define SH_0D 13
#define SH_0E 10
#define SH_0F 11
#define SH_0G 8
#define SH_0H 9
#define SH_1A 6
#define SH_1B 7
#define SH_1C 4
#define SH_1D 5
#define SH_1E 2
#define SH_1F 3
#define SH_1G 0
#define SH_1H 1

// Datatype to hold data of a column
typedef uint32_t colorData[2];
// row index to shift register pin index
uint8_t const colorMap[3][13] = {
  {SH_0B,SH_1G,SH_1F,SH_1E,SH_1D,SH_1C,SH_1B,SH_0H,SH_0G,SH_0F,SH_0E,SH_0D,SH_0C},//r
  {SH_0C,SH_0D,SH_0E,SH_0F,SH_0G,SH_0H,SH_1B,SH_1C,SH_1D,SH_1E,SH_1F,SH_1G,SH_0B},//g
  {SH_0C,SH_0D,SH_0E,SH_0F,SH_0G,SH_0H,SH_1B,SH_1C,SH_1D,SH_1E,SH_1F,SH_1G,SH_0B},//b
};
// Global variables
bool gboBufferReady; // Set by CPU indicating the ChangeBuffer is ready to send out
bool gboBufferSwitched; // Set by SPI+DMA indicating the ChangeBuffer is dirty, clean it before use

// Column Shift register data HIGH mask
static uint8_t const au8ColDataMask[2] = {0x00000040,0x04000000};
// SPI handler variables
spi_device_handle_t spi_hndl;
spi_transaction_t trans_desc;
bool boDisplayRun;

// Buffer memory holder
colorData ledBufferA[BUFF_SIZE]={0};
colorData ledBufferB[BUFF_SIZE]={0};

// Buffer pointers
colorData* ledBufferOut; // Buffer used by the SPI+DMA
colorData* ledBufferChange; // Buffer used by the CPU

// Local functions
static void vDISP_SwitchBuffer();
static bool boDISP_SetColumn(uint8_t u8ColIdx, uint8_t u8BrLvl);
IRAM_ATTR void postCb(spi_transaction_t *trans);

// Function definitions
// Initialize the display module (SPI, Buffers)
bool boDISP_Init(){
  esp_err_t intError;

  // Initialize the bus
  spi_bus_config_t bus_config = { };
  bus_config.sclk_io_num =   0; // CLK
  bus_config.mosi_io_num =   1; // R Data
  bus_config.miso_io_num =   3; // G Data
  bus_config.quadwp_io_num = 4; // B Data
  bus_config.quadhd_io_num = 5; // Clk for Column shift register
  intError = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO) ;
  if(intError!=0)return false;

  // Initialize the device
  spi_device_interface_config_t dev_config = { };
  dev_config.address_bits     = 0;
  dev_config.command_bits     = 0;
  dev_config.dummy_bits       = 0;
  dev_config.mode             = 3;
  dev_config.duty_cycle_pos   = 0;
  dev_config.cs_ena_posttrans = 0;
  dev_config.cs_ena_pretrans  = 0;
  dev_config.clock_speed_hz   = 1000;
  dev_config.spics_io_num     = -1;
  dev_config.flags            = SPI_DEVICE_HALFDUPLEX;
  dev_config.queue_size       = 1;
  dev_config.pre_cb           = NULL;
  dev_config.post_cb          = postCb;
  intError = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_hndl);
  if(intError!=ESP_OK) return false;

  // Initialize the buffer pointers
  ledBufferChange = ledBufferA;
  ledBufferOut = ledBufferB;

  // Initialize the transmision
  trans_desc.addr =  0;
  trans_desc.cmd = 0;
  trans_desc.flags = SPI_TRANS_MODE_QIO;
  trans_desc.length = (BUFF_SIZE*8)*8; // total data bits
  trans_desc.tx_buffer = (uint8_t*)ledBufferOut;
  trans_desc.rxlength = 0 ; // Number of bits NOT number of bytes
  trans_desc.rx_buffer = NULL;

  // Clearing the buffers
  vClearBuffer();
  vDISP_SwitchBuffer();
  vClearBuffer();
  boBufferSwitched = false;
  boBufferReady = false;
  boDisplayRun = false;

  return true;
}

// Set color data indexing from 0,0 left top corner
bool boDISP_SetColor(uint8_t u8RowIdx, uint8_t u8ColIdx, uint8_t u8R, uint8_t u8G, uint8_t u8B){
  uint8_t u8ShIdx=0;
  uint32_t* psRef = NULL;
  uint32_t mask = 0x00000000;
  u8ColIdx = 15 - u8ColIdx;
  //RED
  u8ShIdx = colorMap[0][u8RowIdx];
  psRef = (uint32_t*)&ledBufferChange[u8ColIdx][u8ShIdx/8];
  mask = 0x00000001<<((u8ShIdx&0x07)<<2);
  switch(u8R){
    C_SET(4)
    C_SET(3)
    C_SET(2)
    C_SET(1)
    case 0:
    default:
    break;
  }
  //GREEN
  u8ShIdx = colorMap[1][u8RowIdx];
  psRef = (uint32_t*)&ledBufferChange[u8ColIdx][u8ShIdx/8];
  mask = 0x00000002<<((u8ShIdx&0x07)<<2);
  switch(u8G){
    C_SET(4)
    C_SET(3)
    C_SET(2)
    C_SET(1)
    case 0:
    default:
    break;
  }
  //BLUE
  u8ShIdx = colorMap[2][u8RowIdx];
  psRef = (uint32_t*)&ledBufferChange[u8ColIdx][u8ShIdx/8];
  mask = 0x00000004<<((u8ShIdx&0x07)<<2);
  switch(u8B){
    C_SET(4)
    C_SET(3)
    C_SET(2)
    C_SET(1)
    case 0:
    default:
    break;
  }
}

// Clears the buffer
void vClearBuffer(){
  for(uint8_t j=0;j<BR_LVL;j++){
    for(uint8_t i=0;i<COL_NUM+1;i++) ledBufferChange[i+COL_NUM*j][0]  = 0x777777b7;
    for(uint8_t i=0;i<COL_NUM+1;i++) ledBufferChange[i+COL_NUM*j][1]  = 0x73777777;
    boDISP_SetColumn(COL_NUM+COL_NUM*j);
  }
}
// Sends the first buffer (after that, the sendig is done by Callback)
bool boDISP_SendBuffer(){
  // Dont't send buffer if it's already running
  if(boDisplayRun == true){
    return false;
  }
  bool boRet = (ESP_OK == spi_device_queue_trans(spi_hndl, &trans_desc, portMAX_DELAY));
  boDisplayRun = boRet;
  return boRet;
}
// Stops the display
void boDISP_Stop(){
  boDisplayRun = false;
}
// Returns true if the buffer playes out, false if stopped
bool boDISP_IsRunning(){
  return boDisplayRun;
}

// Switches the two buffer, and sets the indicator flag
static void vDISP_SwitchBuffer(){
  colorData* ledBufferTemp = ledBufferOut;
  ledBufferOut = ledBufferChange;
  ledBufferChange = ledBufferTemp;
  boBufferSwitched = true;
}

// Set column shift register data to HIGH
static bool boDISP_SetColumn(uint8_t u8ColIdx, uint8_t u8BrLvl){
  uint8_t u8Idx = u8ColIdx + u8BrLvl * COL_NUM;
  if(u8Idx>0 && u8Idx<BUFF_SIZE-1) ledBufferChange[u8Idx][0] |= au8ColDataMask[0];
  u8Idx++;
  if(u8Idx>1 && u8Idx<BUFF_SIZE) ledBufferChange[u8Idx][1] |= au8ColDataMask[1];
}

// Callback function to send the next buffer if the spi finished
IRAM_ATTR void postCb(spi_transaction_t *trans){
  // Dont't send buffer if it's stopped
  if(boDisplayRun == false){
    return;
  }
  // If the CPU finished the new buffer, switch between them
  if(boBufferReady == true){
    boBufferReady = false;
    vDISP_SwitchBuffer();
  }
  // Always set the output pointer to the output buffer
  trans_desc.tx_buffer = (uint8_t*)ledBufferOut;
  spi_device_queue_trans(spi_hndl, &trans_desc, portMAX_DELAY);
}
