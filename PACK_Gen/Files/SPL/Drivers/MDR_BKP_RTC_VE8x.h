#ifndef _MDR_BKP_RTC_VE8_H
#define _MDR_BKP_RTC_VE8_H

#include <MDR_Config.h>

typedef enum {
    MDR_BKP_REG0  = 0
  , MDR_BKP_REG1 
  , MDR_BKP_REG2 
  , MDR_BKP_REG3 
  , MDR_BKP_REG4 
  , MDR_BKP_REG5 
  , MDR_BKP_REG6 
  , MDR_BKP_REG7 
  , MDR_BKP_REG8 
  , MDR_BKP_REG9 
  , MDR_BKP_REG10
  , MDR_BKP_REG11
  , MDR_BKP_REG12
  , MDR_BKP_REG13
  , MDR_BKP_REG14
  , MDR_BKP_REG15
  , MDR_BKP_REG16
  , MDR_BKP_REG17
  , MDR_BKP_REG18
  , MDR_BKP_REG19
  , MDR_BKP_REG20
  , MDR_BKP_REG21
  , MDR_BKP_REG22
  , MDR_BKP_REG23
  , MDR_BKP_REG24
  , MDR_BKP_REG25
  , MDR_BKP_REG26
  , MDR_BKP_REG27
  , MDR_BKP_REG28
  , MDR_BKP_REG29
  , MDR_BKP_REG30
  , MDR_BKP_REG31
  , MDR_BKP_REG32
  , MDR_BKP_REG33
  , MDR_BKP_REG34
  , MDR_BKP_REG35
  , MDR_BKP_REG36
  , MDR_BKP_REG37
  , MDR_BKP_REG38
  , MDR_BKP_REG39
  , MDR_BKP_REG40
  , MDR_BKP_REG41
  , MDR_BKP_REG42
  , MDR_BKP_REG43
  , MDR_BKP_REG44
  , MDR_BKP_REG45
  , MDR_BKP_REG46
  , MDR_BKP_REG47
  , MDR_BKP_REG48
  , MDR_BKP_REG49
  , MDR_BKP_REG50
  , MDR_BKP_REG51
  , MDR_BKP_REG52
  , MDR_BKP_REG53
  , MDR_BKP_REG54
  , MDR_BKP_REG55
  , MDR_BKP_REG56
  , MDR_BKP_REG57
  , MDR_BKP_REG58
  , MDR_BKP_REG59
} MDR_BKP_REGs;

__STATIC_INLINE void MDR_BKP_ClockOn(void)
{
#ifdef MDR_CLK_EN__RST_BKP
  REG32(MDR_CLK_EN_ADDR_BKP) |= MDR_CLK_EN_BIT_BKP;
#endif  
}

__STATIC_INLINE void MDR_BKP_ClockOff(void)
{
#ifdef MDR_CLK_EN__RST_BKP
  REG32(MDR_CLK_EN_ADDR_BKP) &= ~MDR_CLK_EN_BIT_BKP;
#endif
}

__STATIC_INLINE void MDR_BKP_SetReg(MDR_BKP_REGs bkpReg, uint32_t value)
{
  MDR_BKP->REG_User[(uint32_t)bkpReg] = value;
}

__STATIC_INLINE uint32_t MDR_BKP_GetReg(MDR_BKP_REGs bkpReg)
{
  return MDR_BKP->REG_User[(uint32_t)bkpReg];
}

//  ---------   Power Control (LDO) -------------
__STATIC_INLINE void MDR_BKP_SetTrimLDO(MDR_BKP_LDO_Trim trimLDO)
{
  uint32_t regValue = MDR_BKP->TMR0.REG_61;
  regValue = (regValue & (~MDR_BKP_REG61_LDO_TRIM_ClearAll)) | MDR_BKP_REG61_LDO_TRIM_Mask((uint32_t) trimLDO);
  
  MDR_BKP->TMR0.REG_61 = regValue;
  MDR_BKP->TMR1.REG_61 = regValue;
  MDR_BKP->TMR2.REG_61 = regValue;
}

__STATIC_INLINE void MDR_BKP_SetTrimLDO_Min(void) { MDR_BKP_SetTrimLDO(MDR_BKP_LDO_Trim_Min); }
__STATIC_INLINE void MDR_BKP_SetTrimLDO_Max(void) { MDR_BKP_SetTrimLDO(MDR_BKP_LDO_Trim_Max); }
__STATIC_INLINE void MDR_BKP_SetTrimLDO_Def(void) { MDR_BKP_SetTrimLDO(MDR_BKP_LDO_Trim_Def); }


#endif //_MDR_BKP_RTC_VE8_H

