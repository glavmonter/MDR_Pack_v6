#ifndef MDR_SFP_READ_INFO_H
#define MDR_SFP_READ_INFO_H

#include <MDR_Config.h>
#include <MDR_SoftI2C_ByTimer.h>
#include "MDR_SFP_Config.h"

//  Info Items Select
typedef struct {
  uint8_t addr;
  uint8_t len;
  char *  name;
} SFP_Info_Item;


extern const SFP_Info_Item MDR_SFP_InfoItems[CFG_SFP_ITEMS_CNT];

bool MDR_ReadSFP_Start(MDR_I2Cst_MasterObj *masterI2C, uint8_t *infoBuff);
bool MDR_ReadSFP_ProcessCompleted(void);


#endif  //MDR_SFP_READ_INFO_H
