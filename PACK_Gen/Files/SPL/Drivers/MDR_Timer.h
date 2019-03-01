#ifndef _MDR_TIMER_H
#define _MDR_TIMER_H

#include <MDR_Config.h>
#include <MDR_RST_Clock.h>
#include <MDR_PER_Clock.h>
#include <MDR_GPIO.h>

#include <MDR_Timer_CfgRegs.h>

#ifdef MDR_TIMER3_CLK_EN_ADDR
  #define TIMER3_EXIST
#endif
#ifdef MDR_TIMER4_CLK_EN_ADDR
  #define TIMER4_EXIST
#endif

//=====================   TimerEx Definitions ============
typedef struct {
  // TIMER Block
  MDR_TIMER_Type     *TIMERx;
  // Timer_Clock and BRG
  MDR_PerClock_Cfg  CfgClock;
  //  IRQn
  IRQn_Type         TIMERx_IRQn;
} MDR_TIMER_TypeEx;

extern const MDR_TIMER_TypeEx    _MDR_TIMER1ex;
#define MDR_TIMER1ex            &_MDR_TIMER1ex

extern const MDR_TIMER_TypeEx    _MDR_TIMER2ex;
#define MDR_TIMER2ex            &_MDR_TIMER2ex

#ifdef TIMER3_EXIST
  extern const MDR_TIMER_TypeEx  _MDR_TIMER3ex;
  #define MDR_TIMER3ex          &_MDR_TIMER3ex
#endif
#ifdef TIMER4_EXIST
  extern const MDR_TIMER_TypeEx  _MDR_TIMER4ex;
  #define MDR_TIMER4ex          &_MDR_TIMER4ex
#endif

//=========================================================================================================
//================================      Counting TIM_Clock or Events   ====================================
//=========================================================================================================

//=====================   Simplest Timer Period IQR (like SystemTimer) ==================

//  ARR = period - 1, ������� �������� period > 0.
void MDR_Timer_InitPeriod(const MDR_TIMER_TypeEx *TIMERex, MDR_BRG_DIV_128 clockBRG, uint16_t timClockPSG, uint_tim period, bool enaIRQ);

typedef enum {
  TIM_EventTIM1_CNT_ARR = 0,
  TIM_EventTIM2_CNT_ARR = 1,
#ifdef TIMER3_EXIST
  TIM_EventTIM3_CNT_ARR = 2,
#endif
#ifdef TIMER4_EXIST
  TIM_EventTIM4_CNT_ARR = 3
#endif  
} MDR_Timer_EventTimer;

void MDR_Timer_AddCascadePeriod(const MDR_TIMER_TypeEx *TIMERex, MDR_BRG_DIV_128 clockBRG, MDR_Timer_EventTimer countEvent, uint_tim period, bool enaIRQ);


//=====================   Timers Start and most usefull control ==================
  #define TIM1_StartMsk     MDR_RST_TIM__TIM1_CLK_EN_Msk
  #define TIM2_StartMsk     MDR_RST_TIM__TIM2_CLK_EN_Msk  
#ifdef TIMER3_EXIST
  #define TIM3_StartMsk     MDR_RST_TIM__TIM3_CLK_EN_Msk
  
  #define _TIM_CLOCK_GATES_MASK   (TIM1_StartMsk | TIM2_StartMsk | TIM3_StartMsk)
#else
  #define _TIM_CLOCK_GATES_MASK   (TIM1_StartMsk | TIM2_StartMsk)
#endif

//  ������������� ������ �������� ������ ��� 3-� ��������!
__STATIC_INLINE void MDR_Timer_StartSync(uint32_t selTimers) {MDR_CLOCK->TIM_CLOCK |=  (selTimers & _TIM_CLOCK_GATES_MASK);}
__STATIC_INLINE void MDR_Timer_StopSync (uint32_t selTimers) {MDR_CLOCK->TIM_CLOCK &= ~(selTimers & _TIM_CLOCK_GATES_MASK);}

#ifdef TIMER4_EXIST
  __STATIC_INLINE void MDR_Timer4_Start(void) {MDR_CLOCK->UART_CLOCK |= MDR_RST_UART__TIM4_CLK_EN_Msk;}
  __STATIC_INLINE void MDR_Timer4_Stop (void) {MDR_CLOCK->UART_CLOCK &= ~MDR_RST_UART__TIM4_CLK_EN_Msk;}
#endif

//  ������ ���������� �������
__STATIC_INLINE void MDR_Timer_Start(const MDR_TIMER_TypeEx *TIMERex) {MDR_PerClock_SetGateOpen(&TIMERex->CfgClock);}
__STATIC_INLINE void MDR_Timer_Stop(const MDR_TIMER_TypeEx  *TIMERex) {MDR_PerClock_GateClose  (&TIMERex->CfgClock);}

//  ������� ����������
void MDR_Timer_SetCNT   (MDR_TIMER_Type *TIMERx, uint_tim value);
void MDR_Timer_SetPeriod(MDR_TIMER_Type *TIMERx, uint_tim value);
void MDR_Timer_SetPSG   (MDR_TIMER_Type *TIMERx, uint_tim value);

//  ���������������
void MDR_Timer_DeInit(const MDR_TIMER_TypeEx *TIMERex);


//=====================   ���������� ������������ � DMA ==================

#define TIM_FL_CNT_ZERO       MDR_TIM_EVENT_CNT_ZERO_Msk    
#define TIM_FL_CNT_ARR        MDR_TIM_EVENT_CNT_ARR_Msk
#define TIM_FL_ETR_RE         MDR_TIM_EVENT_ETR_RE_Msk
#define TIM_FL_ETR_FE         MDR_TIM_EVENT_ETR_FE_Msk
#define TIM_FL_BRK_Msk        MDR_TIM_EVENT_BRK_Msk
#define TIM_FL_CCR_CAP_CH1    MDR_TIM_EVENT_CCR_CAP_CH1_Msk
#define TIM_FL_CCR_CAP_CH2    MDR_TIM_EVENT_CCR_CAP_CH2_Msk
#define TIM_FL_CCR_CAP_CH3    MDR_TIM_EVENT_CCR_CAP_CH2_Msk
#define TIM_FL_CCR_CAP_CH4    MDR_TIM_EVENT_CCR_CAP_CH4_Msk
#define TIM_FL_CCR_REF_CH1    MDR_TIM_EVENT_CCR_REF_CH1_Msk
#define TIM_FL_CCR_REF_CH2    MDR_TIM_EVENT_CCR_REF_CH2_Msk
#define TIM_FL_CCR_REF_CH3    MDR_TIM_EVENT_CCR_REF_CH3_Msk
#define TIM_FL_CCR_REF_CH4    MDR_TIM_EVENT_CCR_REF_CH4_Msk
#define TIM_FL_CCR1_CAP_CH1   MDR_TIM_EVENT_CCR1_CAP_CH1_Msk
#define TIM_FL_CCR1_CAP_CH2   MDR_TIM_EVENT_CCR1_CAP_CH2_Msk
#define TIM_FL_CCR1_CAP_CH3   MDR_TIM_EVENT_CCR1_CAP_CH3_Msk
#define TIM_FL_CCR1_CAP_CH4   MDR_TIM_EVENT_CCR1_CAP_CH4_Msk

__STATIC_INLINE void MDR_Timer_ClearEvent(MDR_TIMER_Type *TIMERx, uint32_t eventFlags) {TIMERx->STATUS &= ~eventFlags;}


//=====================   Common Defines for Other counting modes ==================

typedef enum {
  TIM_CountUp,
  TIM_CountDown,
  TIM_CountUpDown 
} MDR_TIM_CountDir;

#ifdef MDR_TIMER_Can_UpDown_ByExtEvents
  #define MDR_TIM_Event_CountDir    MDR_TIM_CountDir
#else
  #define MDR_TIM_Event_CountDir    MDR_TIM_Dir
#endif

typedef enum {
  TIM_FrontRise = 0,
  TIM_FrontFall = 1
} MDR_Timer_Front;

typedef struct {  
  uint32_t SelectIRQ;              //  Active IRQ
  
  uint32_t priorityIRQ;            //  NVIC IRQ Activation
  bool     activateNVIC_IRQ;
} MDR_Timer_CfgIRQ;

typedef struct {
  MDR_BRG_DIV_128   clockBRG;             //  BRG for TimClock  
  uint_tim          period;               //  Timer Period - ARR
  uint_tim          startValue;           //  Start Value - CNT
  MDR_OnOff         periodUpdateImmediately;
} MDR_Timer_CfgPeriod;


//=====================   Internal Counting, CNT_CLOCK = TIM_CLOCK/(PSG + 1), TIM_CLOCK = CLK / BRG ==================
typedef struct {  
  MDR_Timer_CfgPeriod cfgPeriod;
  MDR_Timer_CfgIRQ    cfgIRQ; 
  MDR_TIM_CountDir    countDir;      //  Count Direction
} MDR_Timer_CfgCount;

typedef struct {  
  MDR_Timer_CfgPeriod cfgPeriod;
  MDR_Timer_CfgIRQ    cfgIRQ; 
  
  uint16_t            timClockPSG;   //  CNT_Clock = TIM_CLK/(PSG + 1)    
  MDR_TIM_CountDir    countDir;      //  Count Direction
} MDR_Timer_CfgCountClock;


void MDR_Timer_InitCountTimClock(const MDR_TIMER_TypeEx *TIMERex, const MDR_Timer_CfgCountClock *cfgCntClock);


//=====================   Count External Events CNT==ARR in Timers, TIM_CLOCK = CLK / BRG ==================
typedef struct {  
  MDR_Timer_CfgPeriod     cfgPeriod;
  MDR_Timer_CfgIRQ        cfgIRQ; 
  
  MDR_Timer_EventTimer    countEvent;     //  Event to Count  
  MDR_TIM_Event_CountDir  countDir;      //  Count Direction
} MDR_Timer_CfgCountTimerEvent;


void MDR_Timer_InitCountTimerEvent(const MDR_TIMER_TypeEx *TIMERex, const MDR_Timer_CfgCountTimerEvent *cfgCntTimEvent);


//=====================   Count External ETR Signal Events, TIM_CLOCK = CLK / BRG ==================
typedef struct {  
  MDR_Timer_CfgPeriod     cfgPeriod;
  MDR_Timer_CfgIRQ        cfgIRQ;
   
  MDR_Timer_Front         selFrontETR;    //  Event to Count
  
  MDR_TIM_Event_CountDir  countDir;       //  Count Direction  
  MDR_TIM_FDTS            clockDTS;       //  Sampling Frequency
} MDR_Timer_CfgCountETR;

//  �������� ������ ETR ������������ � 0 (reg BRKETR_CNTRL)
//  ��� ��������� � ������ �������� ������������ MDR_Timer_InitBRKETR()
void MDR_Timer_InitCountETR(const MDR_TIMER_TypeEx *TIMERex, const MDR_Timer_CfgCountETR *cfgCntETR);


//=====================   Count Channel Capture Events, TIM_CLOCK = CLK / BRG ==================

typedef enum {
  TIM_Event_CH1 = 0,
  TIM_Event_CH2 = 1,
  TIM_Event_CH3 = 2,
  TIM_Event_CH4 = 3,
} MDR_Timer_EventCH;

typedef struct {  
  MDR_Timer_CfgPeriod     cfgPeriod;
  MDR_Timer_CfgIRQ        cfgIRQ;
   
  MDR_Timer_EventCH       selEventCH;     //  Select channel to capture event from
  
  MDR_TIM_Event_CountDir  countDir;       //  Count Direction  
  MDR_TIM_FDTS            clockDTS;       //  Sampling Frequency
  //MDR_TimerCH_CfgCAP      cfgCapCH;
} MDR_Timer_CfgCountCH;

//  ��������� ����������� ��������� ������ ������� � ������� ������� �� ���������� �������!
void MDR_Timer_CountChannelEvent(const MDR_TIMER_TypeEx *TIMERex, const MDR_Timer_CfgCountCH *cfgCntCH);


//=========================================================================================================
//======================================    ETR_BRK Inputs    =============================================
//=========================================================================================================

typedef union {
    uint32_t                  Mask;
    MDR_TIM_BRKETR_CNTRL_Bits Bits;
} MDR_Timer_cfgBRKETR;

__STATIC_INLINE void MDR_Timer_InitBRKETR(const MDR_TIMER_TypeEx *TIMERex, const MDR_Timer_cfgBRKETR cfgBRKETR) {TIMERex->TIMERx->BRKETR_CNTRL = cfgBRKETR.Mask;}


//=========================================================================================================
//===================================    Timer channel Pins   =============================================
//=========================================================================================================

//  ������� ������������� ������� GPIO � ������� ������� �������� �������  (ETR, BRK) � ������� ��� ������� ������� � ��� (CH, nCH)
typedef struct {
  const MDR_GPIO_Port *portGPIO;
  uint32_t       pinIndex;
  MDR_PIN_FUNC   pinFunc;
} MDR_Timer_CfgPinGPIO;

void MDR_TimerCH_InitPinGPIO(const MDR_Timer_CfgPinGPIO *pinCfg, MDR_PIN_PWR pinsPower);
void MDR_TimerCH_DeInitPinGPIO(const MDR_Timer_CfgPinGPIO *pinCfg);


//=========================================================================================================
//======================================      Channel PWM    ==============================================
//=========================================================================================================
//  ��� ���������� ���������� PWM (�� ��� ��� �������� MDR_TimerCh_CfgPin) ������������� ��� ������ ������ CH � nCH.
//  ��� ������ ���������� ��������� ������ �� ���� � ������� ������ �������, �� ������� ���������� ������� ������.
//  � �������, ���� ����� nCH �� �����, �� �� ���� ��� ����������� � ������� GPIO TimerX_nCHx.


//  -------- ���������� ���������� ��������� ��������� -----------
//  ������� ������� ��������� ��� ���� ������� �������
#define MDR_TimerPulse_InitPeriod(timex, brg, psc, period)  MDR_Timer_InitPeriod(timex, brg, psc, period, false)
//  ������� ������������ ���������� � ���������, ������������ �������� ������ �� ������ CH (0 �� nCH)
//  � ��������� period ��������� �������� period �� MDR_TimerPulse_InitPeriod!
void MDR_TimerPulse_InitPulse  (MDR_TIMER_CH_Type *TIMER_CH, uint_tim period, uint8_t widthPerc);
void MDR_TimerPulse_ChangeWidth(MDR_TIMER_CH_Type *TIMER_CH, uint_tim period, uint8_t widthPerc);

                                              
// ----- ������ ������������� ������ PWM �� ����� �����������  -----
                                              
// ������ MDR_Timer_InitPeriod, �� � ������������ ���� ������ ����������� �����
// ����� ���� ������� ��� ��������� ������� ������������ ������� Ref
void MDR_Timer_InitPeriodDir(const MDR_TIMER_TypeEx *TIMERex, MDR_BRG_DIV_128 clockBRG, uint16_t timClockPSG, uint_tim period, bool enaIRQ, MDR_TIM_CountDir dir);
                                              
typedef struct {
  MDR_TIM_DTG_Presc   Presc;           // DTG Prescaller
  MDR_TIM_DTG_CLK     ClockSel;        // DTG Clock Select
  uint8_t             Mul;             // Delay = Mul*(Presc + 1)
} MDR_TimerCh_CfgDTG;

typedef struct {
  MDR_TIM_SelOutEn    ModeIO;
  MDR_TIM_SelOut      OutSignal;
  MDR_OnOff           Invert;
} MDR_TimerCh_CfgPin;

typedef struct {
  //  Pin options, if NULL sets by default to output PWM
  MDR_TimerCh_CfgPin   *cfgPin_CH;
  MDR_TimerCh_CfgPin   *cfgPin_nCH;
  
  // DTG, if not NULL PWM outputs DTG signal, otherwise Ref signal!
  MDR_TimerCh_CfgDTG   *cfgDTG;  
  //  BRK and ETR
  MDR_OnOff             ClearRef_ByBRK;
  MDR_OnOff             ClearRef_ByETR;
} MDR_TimerCh_CfgOptionsPWM;

typedef struct { 
  bool                  use_nCH;              //  Inverted CH signal 
  MDR_OnOff             CCRs_UpdImmediately;
  
  //  Options
  MDR_TimerCh_CfgOptionsPWM *cfgOptions;    
} MDR_TimerCh_CfgPWM;


typedef enum {                        
  MDR_TIM_PWM1_Ref0           = 0,     //  Ref=0
  MDR_TIM_PWM1_Ref1_eqCCRx    = 1,     //  if (CNT == [CCR, CCR1]) Ref=1, else Ref=1
  MDR_TIM_PWM1_Ref0_eqCCRx    = 2,     //  if (CNT == [CCR, CCR1]) Ref=0, else Ref=0
  MDR_TIM_PWM1_RefTgl_eqCCRx  = 3,     //  if (CNT == [CCR, CCR1]) Ref=!Ref
  MDR_TIM_PWM1_Ref_0          = 4,     //  Ref=0
  MDR_TIM_PWM1_Ref1           = 5,     //  Ref=1
                                       //  for CHxCONTROL2.CCR1_EN = false
  MDR_TIM_PWM1_RefNDir_inCCRs = 6,     //  if (CNT in [CCR..CCR1]) Ref=!Dir, else Ref=Dir
  MDR_TIM_PWM1_RefDIR_inCCRs  = 7,     //  if (CNT in [CCR..CCR1]) Ref=Dir, else Ref=!Dir
} MDR_TIM_PWM1_Ref;


// *cfgPWM can be NULL - Default PWM by Ref signal.
void MDR_TimerCh_InitPWM (MDR_TIMER_CH_Type *TIMER_CH, const MDR_TimerCh_CfgPWM *cfgPWM, MDR_TIM_PWM_Ref  modeRef, uint_tim CCR);
void MDR_TimerCh_InitPWM1(MDR_TIMER_CH_Type *TIMER_CH, const MDR_TimerCh_CfgPWM *cfgPWM, MDR_TIM_PWM1_Ref modeRef, uint_tim CCR, uint_tim CCR1);

void MDR_TimerCh_DeInit(MDR_TIMER_CH_Type *TIMER_CH);

void MDR_TimerCh_SetCCR(MDR_TIMER_CH_Type *TIMER_CH, uint_tim value);
void MDR_TimerCh_SetCCR1(MDR_TIMER_CH_Type *TIMER_CH, uint_tim value);

//  ������������� ������ ����� �������� �������� - ������������ ��������������, �� ���������� ����������� ����� ���� ����������.
void MDR_TimerCh_InitByCfgRegs(MDR_TIMER_CH_Type *TIMER_CH, MDR_TIMER_CH_CfgRegs *cfgRegs);


//=========================================================================================================
//=================================      Channel Capture (CAP)    =========================================
//=========================================================================================================

typedef struct {
  MDR_TIM_FLTR            Filter;   
  MDR_OnOff               Invert;         //  ETR Invert
  MDR_PSC_DIV_8           EventPSC;       //  div1, div2, div4, div8
  MDR_TIM_CapEvent        CapEvent;       //  Channel Capture Event
} MDR_TimerCH_CfgCAP;


#endif //_MDR_TIMER_H
