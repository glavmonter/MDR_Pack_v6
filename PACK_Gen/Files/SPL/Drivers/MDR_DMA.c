#include <MDR_DMA.h>

//  Настройка прав доступа шины AHB по умолчанию
static const MDR_DMA_ProtAHB _DMA_ProtAHB_Def = {
  .Fields.Privileged = MDR_On,
  .Fields.Bufferable = MDR_On,
  .Fields.Cachable   = MDR_On,
};

#define _DMA_CH_ProtAHB_Def_Msk   ( MDR_DMA_ChCtrl_SrcPrivileged_Msk  | MDR_DMA_ChCtrl_SrcBufferable_Msk  | MDR_DMA_ChCtrl_SrcCachable_Msk    \
                                  | MDR_DMA_ChCtrl_DestPrivileged_Msk | MDR_DMA_ChCtrl_DestBufferable_Msk | MDR_DMA_ChCtrl_DestCachable_Msk)


//============    Глобальная структура каналов DMA ================
#if !USE_DMA_USER_CNTLS_CTRL_TABLE
  //  Table align for 32 DMA channels
  #define DATA_ALIGN 1024

  #if defined ( __ICCARM__ ) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
    //  For IAR Compiler 
    #ifdef DMA_NEED_EXECUTABLE_MEM_RANGE
      MDR_DMA_ChCfg _DMA_CHNLS_CtrlTable[MDR_DMA_CHANNELS_COUNT * (1 + USE_DMA_ALTER_STRUCT)] __RAM_EXEC;
    #else
      MDR_DMA_ChCfg _DMA_CHNLS_CtrlTable[MDR_DMA_CHANNELS_COUNT * (1 + USE_DMA_ALTER_STRUCT)];
    #endif
    
  #elif defined ( __CC_ARM )
    //  For Keil Compiler
    #ifdef DMA_NEED_EXECUTABLE_MEM_RANGE
      MDR_DMA_ChCfg _DMA_CHNLS_CtrlTable[MDR_DMA_CHANNELS_COUNT * (1 + USE_DMA_ALTER_STRUCT)] __RAM_EXEC __attribute__ ((aligned (DATA_ALIGN)));
    #else
      MDR_DMA_ChCfg _DMA_CHNLS_CtrlTable[MDR_DMA_CHANNELS_COUNT * (1 + USE_DMA_ALTER_STRUCT)] __attribute__ ((aligned (DATA_ALIGN)));
    #endif
  #endif
  
  
  //  Функции Init сами включают тактирование!
  //  Инициализация DMA с правами доступа к таблице управляющих структур каналов по умолчанию
  void MDR_DMA_Init(void)
  {
    MDR_DMA_InitProtEx(_DMA_CHNLS_CtrlTable, _DMA_ProtAHB_Def);
  }

  //  Инициализация DMA с заданием прав доступа к таблице управляющих структур каналов
  void MDR_DMA_InitProtAHB(MDR_DMA_ProtAHB chnlsTableAccess)
  {
    MDR_DMA_InitProtEx(_DMA_CHNLS_CtrlTable, chnlsTableAccess);
  }  
  
#endif

void DMA_ClearRegs(void)
{
  MDR_DMA->CFG = 0;                           // Выключение контроддера DMA
  MDR_DMA->CTRL_BASE_PTR = 0;                 // Обнуление указателя на управляющие структуры каналов
  MDR_DMA->CHNL_SW_REQUEST = 0;               // Обнуление программный запросов на запуск работы каналов
  MDR_DMA->CHNL_USEBURST_SET  = 0xFFFFFFFF;   // Запрет обработки одиночных запросов SReq
  MDR_DMA->CHNL_REQ_MASK_SET  = 0xFFFFFFFF;   // Игнорирование запросов SReq и Req  
  
  MDR_DMA->CHNL_ENABLE_SET    = 0xFFFFFFFF;   // Разрешение работы каналов. Итого: каналы разрешены, но запросы игнорируются = не работают!
  // С этим прерывание зависает!
  //MDR_DMA->CHNL_ENABLE_CLR    = 0xFFFFFFFF;
  
  MDR_DMA->CHNL_PRI_ALT_CLR   = 0xFFFFFFFF;   // Активны основные управляющие структуры
  MDR_DMA->CHNL_PRIORITY_CLR  = 0xFFFFFFFF;   // Отключение повышенного приоритета
  MDR_DMA->ERR_CLR = 0x01;                    // Стирание ошибок
}

//  Инициализация DMA с пользовательским расположением управляющи структур каналов и привилегиями доступа
void MDR_DMA_InitProtEx(const MDR_DMA_ChCfg *channelsCtrlTable,  MDR_DMA_ProtAHB chnlsTableAccess)
{
  //	Clock Enable
  REG32(MDR_DMA_CLK_EN_ADDR) |= MDR_DMA_CLK_EN_MSK;

//  ВЕ4, ВК234  
//#ifdef MDR_DMA_CLKex_EN_MSK  
//  REG32(MDR_DMA_CLKex_EN_ADDR) |= MDR_DMA_CLKex_EN_MSK;
//#endif
  
  //  Сброс регистров в состояне по умолчанию
  DMA_ClearRegs();
  
  //  Основная конфигурация и включение блока DMA
  MDR_DMA->CTRL_BASE_PTR = (uint32_t)channelsCtrlTable; 
  MDR_DMA->CFG = MDR_DMA_CFG_Enable_Msk | VAL2FLD_Pos(chnlsTableAccess.Value, MDR_DMA_CFG_Privileged_Pos);
}

//  Деинициализация DMA, выключает тактирование блока
void MDR_DMA_DeInit(void)
{
  //  Сброс регистров в состояне по умолчанию
  DMA_ClearRegs();
  
  //	Clock Disable
  REG32(MDR_DMA_CLK_EN_ADDR) &= ~MDR_DMA_CLK_EN_MSK;
}

//==================   Вспомогательные структуры для настройки каналов ===================
uint32_t MDR_DMA_Calc_SrcEndAddr(uint32_t startAddr, MDR_DMA_AddrInc AddrInc, uint32_t dataCount)
{
  if (AddrInc == MDR_DMA_AddrIncOff)
    return startAddr;
  else
    return startAddr + ((dataCount - 1) << AddrInc);
}

uint32_t MDR_DMA_Calc_DestEndAddr(uint32_t startAddr, MDR_DMA_AddrInc AddrInc, uint32_t dataCount, MDR_DMA_Mode mode)
{ 
  if (mode == DMA_MODE_MemScatterPri)
    return startAddr + 12;
  else
    return MDR_DMA_Calc_SrcEndAddr(startAddr, AddrInc, dataCount);
}

static void DMA_Fill_InitChCfg(const MDR_DMA_Init_ChStruct *initChStruct, MDR_DMA_Init_ChCfg *initChCfg)
{
  //  Src and Dest EndAddr
  initChCfg->Src_EndAddr  = MDR_DMA_Calc_SrcEndAddr (initChStruct->SrcAddr, initChStruct->Src_AddrInc, initChStruct->Count);
  initChCfg->Dest_EndAddr = MDR_DMA_Calc_DestEndAddr(initChStruct->DestAddr, initChStruct->Dest_AddrInc, initChStruct->Count, initChStruct->Mode);
    
  //  Control
  initChCfg->Control.Value =  VAL2FLD_Pos(initChStruct->Count - 1,    MDR_DMA_ChCtrl_N_minus1_Pos) 
                            | VAL2FLD_Pos(initChStruct->DataSize,     MDR_DMA_ChCtrl_SrcDataSize_Pos)
                            | VAL2FLD_Pos(initChStruct->DataSize,     MDR_DMA_ChCtrl_DestDataSize_Pos)
                            | VAL2FLD_Pos(initChStruct->Src_AddrInc,  MDR_DMA_ChCtrl_SrcAddrInc_Pos)
                            | VAL2FLD_Pos(initChStruct->Dest_AddrInc, MDR_DMA_ChCtrl_DestAddrInc_Pos)  
                            | VAL2FLD_Pos(initChStruct->Mode,         MDR_DMA_ChCtrl_Mode_Pos)  
                            | VAL2FLD_Pos(initChStruct->UseBurst,     MDR_DMA_ChCtrl_UseBurst_Pos)  
                            | VAL2FLD_Pos(initChStruct->ArbitrCount,  MDR_DMA_ChCtrl_ArbitrCount_Pos);
  
  if (initChStruct->Src_ProtAHB == NULL)
    initChCfg->Control.Value |= VAL2FLD_Pos(_DMA_ProtAHB_Def.Value,            MDR_DMA_ChCtrl_SrcPrivileged_Pos);
  else
    initChCfg->Control.Value |= VAL2FLD_Pos(initChStruct->Src_ProtAHB->Value,  MDR_DMA_ChCtrl_SrcPrivileged_Pos);
  
  if (initChStruct->Dest_ProtAHB == NULL)
    initChCfg->Control.Value |= VAL2FLD_Pos(_DMA_ProtAHB_Def.Value,            MDR_DMA_ChCtrl_DestPrivileged_Pos);
  else
    initChCfg->Control.Value |= VAL2FLD_Pos(initChStruct->Dest_ProtAHB->Value, MDR_DMA_ChCtrl_DestPrivileged_Pos);    
}


static void DMA_Apply_ChCfg(MDR_DMA_ChCfg *DMA_chCfgRegs, const MDR_DMA_Init_ChCfg *initChCfg)
{
  //  CtrlTable
  DMA_chCfgRegs->Src_EndAddr  = initChCfg->Src_EndAddr;
  DMA_chCfgRegs->Dest_EndAddr = initChCfg->Dest_EndAddr;  
  DMA_chCfgRegs->Control      = initChCfg->Control;
   
  if (initChCfg->activateNVIC)
  {
    NVIC_ClearPendingIRQ(DMA_IRQn);
    NVIC_EnableIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, initChCfg->priority);
  }
}

//==================   Настройка каналов DMA ===================
MDR_DMA_ChCtrl  MDR_DMA_InitChannelPri(uint32_t chIndex, const MDR_DMA_Init_ChStruct *initCh)
{
  MDR_DMA_Init_ChCfg initChCfg;
  
  DMA_Fill_InitChCfg(initCh, &initChCfg);
  return MDR_DMA_InitChannelCfgPri(chIndex, &initChCfg);
}

MDR_DMA_ChCtrl  MDR_DMA_InitChannelAlt(uint32_t chIndex, const MDR_DMA_Init_ChStruct *initCh)
{
  MDR_DMA_Init_ChCfg initChCfg;
  
  DMA_Fill_InitChCfg(initCh, &initChCfg);
  return MDR_DMA_InitChannelCfgAlt(chIndex, &initChCfg);  
}

MDR_DMA_ChCtrl  MDR_DMA_InitChannelCfgPri(uint32_t chIndex, const MDR_DMA_Init_ChCfg *initChCfg)
{
  MDR_DMA_ChCfg *cfgTable = (MDR_DMA_ChCfg *)MDR_DMA->CTRL_BASE_PTR;  
  DMA_Apply_ChCfg(&cfgTable[chIndex], initChCfg);
  
  return initChCfg->Control;
}

MDR_DMA_ChCtrl  MDR_DMA_InitChannelCfgAlt(uint32_t chIndex, const MDR_DMA_Init_ChCfg *initChCfg)
{
  MDR_DMA_ChCfg *cfgTable = (MDR_DMA_ChCfg *)MDR_DMA->ALT_CTRL_BASE_PTR;  
  DMA_Apply_ChCfg(&cfgTable[chIndex], initChCfg);
  
  return initChCfg->Control;
}


//  Запуск работы канала и остановка
void  MDR_DMA_StartChannel(uint32_t chIndex, MDR_OnOff IgnoreSReq, MDR_OnOff HighPriority, bool startWithPrimary)
{
  uint32_t chSel;
  chSel = VAL2FLD_Pos(1, chIndex);

  if (IgnoreSReq)
    MDR_DMA->CHNL_USEBURST_SET = chSel;
  else
    MDR_DMA->CHNL_USEBURST_CLR = chSel;
  
  if (HighPriority)
    MDR_DMA->CHNL_PRIORITY_SET = chSel;
  else
    MDR_DMA->CHNL_PRIORITY_CLR = chSel;   
  
  if (startWithPrimary)
    MDR_DMA->CHNL_PRI_ALT_CLR = chSel;
  else
    MDR_DMA->CHNL_PRI_ALT_SET = chSel;  
  
  //  Start process Reqs
  MDR_DMA->CHNL_REQ_MASK_CLR = chSel;
  MDR_DMA->CHNL_ENABLE_SET   = chSel;
}

void  MDR_DMA_StartChannelSoft(uint32_t chIndex, MDR_OnOff HighPriority, bool startWithPrimary)
{
  uint32_t chSel;
  chSel = VAL2FLD_Pos(1, chIndex);

  if (HighPriority)
    MDR_DMA->CHNL_PRIORITY_SET = chSel;
  else
    MDR_DMA->CHNL_PRIORITY_CLR = chSel;   
  
  if (startWithPrimary)
    MDR_DMA->CHNL_PRI_ALT_CLR = chSel;
  else
    MDR_DMA->CHNL_PRI_ALT_SET = chSel;  
  
  //  Start process Reqs
  MDR_DMA->CHNL_ENABLE_SET   = chSel;
  MDR_DMA->CHNL_SW_REQUEST   = chSel;
}

void  MDR_DMA_StopChannel (uint32_t chIndex)
{
  uint32_t chSel = VAL2FLD_Pos(1, chIndex);
  
  MDR_DMA->CHNL_REQ_MASK_SET = chSel;
  MDR_DMA->CHNL_ENABLE_CLR = chSel;
}

//  Перезапуск следующего цикла DMA
//  Обновляет контрольное слово в упр. структуре и выставляет Enable!
void  MDR_DMA_RunNextCyclePri(uint32_t chIndex, MDR_DMA_ChCtrl chCtrl)
{
  MDR_DMA_InitNextCyclePri(chIndex, chCtrl);
  MDR_DMA->CHNL_ENABLE_SET = VAL2FLD_Pos(1, chIndex);
}

void  MDR_DMA_RunNextCycleAlt(uint32_t chIndex, MDR_DMA_ChCtrl chCtrl)
{
  MDR_DMA_InitNextCycleAlt(chIndex, chCtrl);
  MDR_DMA->CHNL_ENABLE_SET = VAL2FLD_Pos(1, chIndex);
}

//  Готовые функции копирования массивов с использованием DMA
//  Необходима предварительная инициализация блока - MDR_DMA_Init()

#define _DMA_CtrlCopy8   ( VAL2FLD_Pos(DMA_MODE_AutoRequest,  MDR_DMA_ChCtrl_Mode_Pos)         \
                         | VAL2FLD_Pos(DMA_Arbitr_1024,       MDR_DMA_ChCtrl_ArbitrCount_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data8,         MDR_DMA_ChCtrl_SrcDataSize_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data8,         MDR_DMA_ChCtrl_DestDataSize_Pos) \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc8,      MDR_DMA_ChCtrl_SrcAddrInc_Pos)   \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc8,      MDR_DMA_ChCtrl_DestAddrInc_Pos)  \
                         | _DMA_CH_ProtAHB_Def_Msk )

void MDR_DMA_CopyStart8 (uint32_t chIndex, uint8_t  *src, uint8_t  *dest, uint16_t count)
{
  MDR_DMA_ChCfg chCfg;
  
  chCfg.Src_EndAddr  = (uint32_t)src  + count - 1;
  chCfg.Dest_EndAddr = (uint32_t)dest + count - 1;
  chCfg.Control.Value = VAL2FLD(count - 1, MDR_DMA_ChCtrl_N_minus1) | _DMA_CtrlCopy8;
  
  MDR_DMA_CtrlTablePri(chIndex) = chCfg;
  MDR_DMA_StartChannelSoft(chIndex, MDR_On, true);  
}

#define _DMA_CtrlCopy16  ( VAL2FLD_Pos(DMA_MODE_AutoRequest,  MDR_DMA_ChCtrl_Mode_Pos)         \
                         | VAL2FLD_Pos(DMA_Arbitr_1024,        MDR_DMA_ChCtrl_ArbitrCount_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data16,         MDR_DMA_ChCtrl_SrcDataSize_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data16,         MDR_DMA_ChCtrl_DestDataSize_Pos) \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc16,      MDR_DMA_ChCtrl_SrcAddrInc_Pos)   \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc16,      MDR_DMA_ChCtrl_DestAddrInc_Pos)  \
                         | _DMA_CH_ProtAHB_Def_Msk )

void MDR_DMA_CopyStart16(uint32_t chIndex, uint16_t *src, uint16_t *dest, uint16_t count)
{
  MDR_DMA_ChCfg chCfg;
  
  chCfg.Src_EndAddr  = (uint32_t)src  + ((count - 1) << 1);
  chCfg.Dest_EndAddr = (uint32_t)dest + ((count - 1) << 1);
  chCfg.Control.Value = VAL2FLD(count - 1, MDR_DMA_ChCtrl_N_minus1) | _DMA_CtrlCopy16;
  
  MDR_DMA_CtrlTablePri(chIndex) = chCfg;
  MDR_DMA_StartChannelSoft(chIndex, MDR_On, true);  
}


#define _DMA_CtrlCopy32  ( VAL2FLD_Pos(DMA_MODE_AutoRequest,  MDR_DMA_ChCtrl_Mode_Pos)         \
                         | VAL2FLD_Pos(DMA_Arbitr_1024,        MDR_DMA_ChCtrl_ArbitrCount_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data32,         MDR_DMA_ChCtrl_SrcDataSize_Pos)  \
                         | VAL2FLD_Pos(MDR_DMA_Data32,         MDR_DMA_ChCtrl_DestDataSize_Pos) \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc32,      MDR_DMA_ChCtrl_SrcAddrInc_Pos)   \
                         | VAL2FLD_Pos(MDR_DMA_AddrInc32,      MDR_DMA_ChCtrl_DestAddrInc_Pos)  \
                         | _DMA_CH_ProtAHB_Def_Msk )

void MDR_DMA_CopyStart32(uint32_t chIndex, uint32_t *src, uint32_t *dest, uint16_t count)
{
  MDR_DMA_ChCfg chCfg;
  
  chCfg.Src_EndAddr  = (uint32_t)src  + ((count - 1) << 2);
  chCfg.Dest_EndAddr = (uint32_t)dest + ((count - 1) << 2);
  chCfg.Control.Value = VAL2FLD(count - 1, MDR_DMA_ChCtrl_N_minus1) | _DMA_CtrlCopy32;
  
  MDR_DMA_CtrlTablePri(chIndex) = chCfg;
  MDR_DMA_StartChannelSoft(chIndex, MDR_On, true);
}

void MDR_DMA_Copy8 (uint32_t chIndex, uint8_t  *src, uint8_t  *dest, uint16_t count)
{
  MDR_DMA_CopyStart8(chIndex, src, dest, count);
  while (!MDR_DMA_CopyGetCompleted(chIndex)){};   
}

void MDR_DMA_Copy16(uint32_t chIndex, uint16_t *src, uint16_t *dest, uint16_t count)
{
  MDR_DMA_CopyStart16(chIndex, src, dest, count);
  while (!MDR_DMA_CopyGetCompleted(chIndex)){}; 
}

void MDR_DMA_Copy32(uint32_t chIndex, uint32_t *src, uint32_t *dest, uint16_t count)
{
  MDR_DMA_CopyStart32(chIndex, src, dest, count);
  while (!MDR_DMA_CopyGetCompleted(chIndex)){};   
}


