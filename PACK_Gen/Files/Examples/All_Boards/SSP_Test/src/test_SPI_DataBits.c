#include "test_SSP_Interfs.h"
#include "SSP_cfg.h"
#include "SSP_CommonFuncs.h"

//===============   ОПИСАНИЕ  ======================
//  Тест переделан из теста передачи по Master-Slave. Делает все тоже самое, только вместо скорости меняется количество бит в слове.
//  Рабочая скорость должна быть выставлена в тесте Master-Slave.
//  Кнопки:
//    - Up    - Смена теста
//    - Right - Смена количество бит в слове
//    - Down  - Запуск передачи-приема слов, количество 65535.
//  Принимаемые данные сравниваются с отправляемыми, количество ошибок выводится на экран LCD.
//  Если ошибок нет выводится Success Transfer


//  LBM Test Interface functions
static void TestSSP_Init(void);
static void TestSSP_Finit(void);
static void TestSSP_ChangeRate(void);
static void TestSSP_Exec(void);
static void TestSSP_HandleIRQ(void);


TestInterface TI_SSP_DataBits = {
  .funcInit       = TestSSP_Init,
  .funcFinit      = TestSSP_Finit,
  .funcChange     = TestSSP_ChangeRate,
  .funcExec       = TestSSP_Exec,
  .funcHandlerIrqMaster = TestSSP_HandleIRQ,
  .funcHandlerIrqSlave  = TestSSP_HandleIRQ
};

static void TestSSP_Init(void)
{
  //  Ограничение на скорость ведомого
  cfgSSPex.cfgSSP->DivSCR_0_255 = 0;
  if (cfgSSPex.cfgSSP->DivPSR_2_254 < 12) 
    cfgSSPex.cfgSSP->DivPSR_2_254 = 12;  
    
  //  Init  Master
  MDR_SSPex_InitEx(SSP_MASTER, &cfgSSPex);
  MDR_SSPex_EnableMaster(SSP_MASTER, false);
  
  //  Init  Slave
  MDR_SSPex_InitEx(SSP_SLAVE, &cfgSSPex);
  MDR_SSPex_EnableSlave(SSP_SLAVE, false);

  LCD_ShowInit(SSP_MASTER, "Ms-Sl Bits");
}

static void TestSSP_Finit(void)
{
  MDR_SSPex_DeInit(SSP_MASTER);
  MDR_SSPex_DeInit(SSP_SLAVE);
}

static void TestSSP_ChangeRate(void)
{
  Cfg_NextDataBits();
  MDR_SSPex_ChangeDataBits(SSP_MASTER, cfgSSPex.cfgSSP->DataBits);
  MDR_SSPex_ChangeDataBits(SSP_SLAVE,  cfgSSPex.cfgSSP->DataBits);
  
  LCD_ShowInit(SSP_MASTER, "Ms-Sl Bits");
}

static void TestSSP_Exec(void)
{
  uint32_t errCount = 0;
  
  LCD_ShowStarted();  
  errCount = MasterSlave_TestTransfer();
  LCD_ShowResult(errCount);    
}

static void TestSSP_HandleIRQ(void)
{
  // Not Used
}


