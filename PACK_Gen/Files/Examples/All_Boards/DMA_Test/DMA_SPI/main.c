#include <MDR_RST_Clock.h>
#include <MDR_PER_Clock.h>
#include <MDRB_Buttons.h>
#include <MDRB_LEDs.h>
#include <MDR_Funcs.h>
#include <MDR_DMA.h>
#include <MDR_SSP.h>

#include <MDRB_SPI_SelectPins.h>

#define BTN_DEBOUNCE_MS 10

#define LED_PERIOD      100000
#define LED_CYCLE       MDRB_LED_1
#define LED_ERR         MDRB_LED_2

//  ----------- Массивы для хранение данных от АЦП  ----------
#define DATA_LEN        1024
#define AVER_DIV_RSH    10

#ifdef DMA_NEED_EXECUTABLE_MEM_RANGE
  //  Для 1986ВЕ1 и 1986ВЕ3Т массивы должны лежать в области памяти доступной для DMA - с адресов 0x2010_0000
  //  Необходимо задать это в скаттер файле, или в настройках модуля *.с
  //  В закладке линкера подключен готовый скаттер файл MDR1986VE1T_sctr.txt.
  uint16_t DataTX[DATA_LEN] __RAM_EXEC;
  uint16_t DataRX[DATA_LEN] __RAM_EXEC;

#else
  // Для остальных МК этого не нужно
  uint16_t DataTX[DATA_LEN];
  uint16_t DataRX[DATA_LEN];
#endif

//  ----------- Настройка каналов DMA  ----------
#ifdef MDR_DMA_CH_REQ_SSP2_RX
  #define DMA_Chan_TX            MDR_DMA_CH_SREQ_SSP1_TX
  #define DMA_Chan_RX            MDR_DMA_CH_SREQ_SSP2_RX
  
  #define SSP_MASTER             MDR_SSP1ex
  #define SSP_SLAVE              MDR_SSP2ex

  #define SSP_MASTER_PINS        MDRB_PinsSSP1
  #define SSP_SLAVE_PINS         MDRB_PinsSSP2

#else
  #define SINGLE_SPI_MODE

  #define DMA_Chan_TX            MDR_DMA_CH_SREQ_SSP1_TX
  #define DMA_Chan_RX            MDR_DMA_CH_SREQ_SSP1_RX
    
  #define SSP_MASTER             MDR_SSP1ex
  #define SSP_SLAVE              MDR_SSP1ex
  
  #define SSP_MASTER_PINS        MDRB_PinsSSP1
#endif


#define DMA_IRQ_PRIORITY    0

const MDR_DMA_CfgTransf  cfgDMA_TX = { 
  //  DMA
  .CfgFileds.Mode         = DMA_MODE_Base,
  .CfgFileds.UseBurst     = MDR_Off,
  .CfgFileds.ArbitrCount  = DMA_Arbitr_1,  
  .CfgFileds.DataSize     = MDR_DMA_Data16,
  .CfgFileds.Src_AddrInc  = MDR_DMA_AddrInc16,
  .CfgFileds.Dest_AddrInc = MDR_DMA_AddrIncOff,
  //  NULL for default init
  .Src_ProtAHB  = NULL,
  .Dest_ProtAHB = NULL,
};

const MDR_DMA_CfgTransf  cfgDMA_RX = { 
  //  DMA
  .CfgFileds.Mode         = DMA_MODE_Base,
  .CfgFileds.UseBurst     = MDR_Off,
  .CfgFileds.ArbitrCount  = DMA_Arbitr_1,  
  .CfgFileds.DataSize     = MDR_DMA_Data16,
  .CfgFileds.Src_AddrInc  = MDR_DMA_AddrIncOff,
  .CfgFileds.Dest_AddrInc = MDR_DMA_AddrInc16,
  //  NULL for default init
  .Src_ProtAHB  = NULL,
  .Dest_ProtAHB = NULL,
};

//  ----------- Настройки SPI  ----------
MDR_SSP_Config cfgSSP = {
  //  Config
  .DataBits = SSP_DataBits_16,
  .FrameFormat = SSP_Frame_SPI,
  .SPI_Mode = SPI_CPHA0_CPOL0,
  // BitRate                        // BitRate = SSP_Clock / (PSR * (1 + SCR))
  .DivSCR_0_255 = 0,                // 0 - 255, Serial Clock Rate
  .DivPSR_2_254 = 14,               // 2 - 254, EVEN ONLY! Clock prescaller                                    
};


//  ----------- Application  ----------
MDR_DMA_ChCtrl RestartCtrl_TX;
MDR_DMA_ChCtrl RestartCtrl_RX;

bool DMA_CompletedTX;
bool DMA_CompletedRX;
bool DMA_DoCheckData = false;

void ClearData(void);
bool CheckData(void);
void CyclicLedFlash(void);


int main(void)
{
  uint32_t freqCPU_Hz;
  bool      firstStart = true;

  //  Максимальная скорость тактирования
  MDR_CPU_SetClock_HSE_Max(MDR_Off);
  
  //  Инициализация кнопок и LCD экрана
  freqCPU_Hz = MDR_CPU_GetFreqHz(true);
  MDRB_Buttons_Init(BTN_DEBOUNCE_MS, freqCPU_Hz);
  MDRB_LED_Init(LED_CYCLE | LED_ERR);

//  Для 1986VE4,VE214,VE234 частота SSP_Clock формируется мультиплексорами
  //  В остальных МК SSP_Clock формируется только из HCLK (равной CPU_Clock) - выбор источника не требуется
#ifdef MDR_SSP_CLOCK_FROM_PER_CLOCK  
  MDR_SetClock_UartTimSSP(MDR_PER_PLLCPUo);
#endif  

  //  Init  Master SPI
  MDR_SSPex_Init(SSP_MASTER, &cfgSSP, MDR_BRG_div1);
  MDR_SSPex_EnableMaster(SSP_MASTER, false);  
  MDR_SSP_InitPinsGPIO(&SSP_MASTER_PINS, MDR_PIN_MAXFAST);
  
#ifndef  SINGLE_SPI_MODE
  //  Init  Slave SPI
  MDR_SSPex_Init(SSP_SLAVE, &cfgSSP, MDR_BRG_div1);
  MDR_SSPex_EnableSlave(SSP_SLAVE, false);   
  MDR_SSP_InitPinsGPIO(&SSP_SLAVE_PINS, MDR_PIN_MAXFAST);  
#endif

  //  SPI Enable DMA call
  MDR_SSP_TX_DMA_Enable(SSP_MASTER->SSPx);  
  MDR_SSP_RX_DMA_Enable(SSP_SLAVE->SSPx);

  //  DMA
  MDR_DMA_Init();
  RestartCtrl_TX = MDR_DMA_InitTransfPri(DMA_Chan_TX, (uint32_t)DataTX, (uint32_t)&(SSP_MASTER->SSPx)->DR, DATA_LEN, &cfgDMA_TX);
  RestartCtrl_RX = MDR_DMA_InitTransfPri(DMA_Chan_RX, (uint32_t)&(SSP_SLAVE->SSPx)->DR, (uint32_t)DataRX,  DATA_LEN, &cfgDMA_RX);
  //  Start IRQ and RX channed
  MDR_DMA_EnableIRQ(DMA_IRQ_PRIORITY);
  MDR_DMA_StartChannel(DMA_Chan_RX, false, false, true);

  while (1)
  {
    if (MDRB_BntClicked_Up(true))
    {
      MDRB_LED_Set(LED_CYCLE | LED_ERR, 1);
      
      //  Restart new transfer
      ClearData();      
      DMA_CompletedTX = false;
      DMA_CompletedRX = false;
      
      //  Run DMA Transfer
      if (firstStart)
      {
        MDR_DMA_StartChannel(DMA_Chan_TX, false, false, true);  //  Start SPI transfer
        firstStart = false;
      }
      else
      {
        MDR_DMA_InitNextCyclePri(DMA_Chan_RX, RestartCtrl_RX);
        MDR_DMA_InitNextCyclePri(DMA_Chan_TX, RestartCtrl_TX);
        MDR_DMA_ReStartChannel(DMA_Chan_RX);
        MDR_DMA_ReStartChannel(DMA_Chan_TX);                    //  Start SPI transfer
      }
    }
    
    if (DMA_DoCheckData)
    {      
      if (CheckData())
      {  
        // Задержка чтобы увидеть на светодиоде, что он загорался
        //MDR_Delay_ms(500, freqCPU_Hz);        
        MDRB_LED_Set(LED_ERR, 0);
      }
      DMA_DoCheckData = false;
    }
    
    CyclicLedFlash();
  }
}

void DMA_IRQHandler(void)
{
  if (!DMA_CompletedTX)
    if (MDR_DMA_GetChannelCompleted(DMA_Chan_TX))
    {
      MDR_DMA_StopChannel(DMA_Chan_TX);
      DMA_CompletedTX = true;
    }

  if (!DMA_CompletedRX)
    if (MDR_DMA_GetChannelCompleted(DMA_Chan_RX))
    {
      MDR_DMA_StopChannel(DMA_Chan_RX);
      DMA_CompletedRX = true;
    }
    
  DMA_DoCheckData = DMA_CompletedTX && DMA_CompletedRX;
}



void ClearData(void)
{
  static uint32_t nextData = 0;
  uint32_t i;
  
  for (i = 0; i < DATA_LEN; ++i)
  {
    DataTX[i] = MDR_ToPseudoRand(nextData++);
    DataRX[i] = 0;
  }
}

bool CheckData(void)
{
  uint32_t i;
  uint32_t cntErr = 0;
  
  for (i = 0; i < DATA_LEN; ++i)
    if (DataTX[i] != DataRX[i])
      ++cntErr;

  return cntErr == 0;
}

void CyclicLedFlash(void)
{
  static uint32_t cnt = 0;
  
  ++cnt;
  if (cnt > LED_PERIOD)
  {
    cnt = 0;
    MDRB_LED_Switch(LED_CYCLE);
  }
}