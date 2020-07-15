#ifndef MDR_1923KX029_MODE0_CTRL_H
#define MDR_1923KX029_MODE0_CTRL_H

#include <MDR_Config.h>
#include <MDR_1923KX028.h>
#include <MDR_1923KX028_AXI_defs.h>
#include <MDR_1923KX028_Config.h>
#include <MDR_1923KX028_GlobConst.h>

void MDR_KX028_M0_SetupBaseAddrRegisters(void);
void MDR_KX028_M0_WaitPC_DriverReady(void);

#endif  //MDR_1923KX029_MODE0_CTRL_H
