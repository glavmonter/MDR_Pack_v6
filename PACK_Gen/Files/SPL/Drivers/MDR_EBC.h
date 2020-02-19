#ifndef MDR_EBC_H
#define MDR_EBC_H

#include <MDR_Types.h>
#include <MDR_Config.h>

//  Initialiation
__STATIC_INLINE void MDR_EBC_Enable (void) { MDR_EBC->KEY = MDR_KEY_UNLOCK; }
__STATIC_INLINE void MDR_EBC_Disable(void) { MDR_EBC->KEY = 0; }

                void MDR_EBC_EnableRegion(MDR_EBC_RGN_Type *EBC_RGN, const MDR_EBC_RGN_CfgRegs *cfg);
__STATIC_INLINE void MDR_EBC_DisableRegion(MDR_EBC_RGN_Type *EBC_RGN) { EBC_RGN->CNTR &= MDR_EBC_CNTR_EN_Msk; }


//  ECC Control - Status
__STATIC_INLINE  void MDR_EBC_CS_SetFlags(MDR_EBC_RGN_Type *EBC_RGN, uint32_t selFlags) { EBC_RGN->CS |= selFlags; }
__STATIC_INLINE  void MDR_EBC_CS_ClrFlags(MDR_EBC_RGN_Type *EBC_RGN, uint32_t selFlags) { EBC_RGN->CS &= selFlags; }

__STATIC_INLINE  void MDR_EBC_SECC_ClearEvent  (MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_SECC_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_ClearEvent  (MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_DECC_Msk); }
__STATIC_INLINE  void MDR_EBC_SECC_ClearCounter(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_CLR_SECC_CNT_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_ClearCounter(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_CLR_DECC_CNT_Msk); }

__STATIC_INLINE  void MDR_EBC_SECC_LogEna(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_FIX_SECC_Msk); }
__STATIC_INLINE  void MDR_EBC_SECC_LogDis(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_ClrFlags(EBC_RGN, MDR_EBC_ECC_CS_FIX_SECC_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_LogEna(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_FIX_DECC_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_LogDis(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_ClrFlags(EBC_RGN, MDR_EBC_ECC_CS_FIX_DECC_Msk); }


__STATIC_INLINE  void MDR_EBC_SECC_IrqEna(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_SECC_IE_Msk); }
__STATIC_INLINE  void MDR_EBC_SECC_IrqDis(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_ClrFlags(EBC_RGN, MDR_EBC_ECC_CS_SECC_IE_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_IrqEna(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_SetFlags(EBC_RGN, MDR_EBC_ECC_CS_DECC_IE_Msk); }
__STATIC_INLINE  void MDR_EBC_DECC_IrqDis(MDR_EBC_RGN_Type *EBC_RGN) { MDR_EBC_CS_ClrFlags(EBC_RGN, MDR_EBC_ECC_CS_DECC_IE_Msk); }


#endif // MDR_CAN_H
