#include <MDR_1923KX028_M2_TablesCtrl.h>
#include <MDR_1923KX028_ErrLog.h>
#include <MDR_1923KX028_GlobConst.h>

static void MDR_KX028_MAC_DeleteTableItem(MDR_KX028_MAC_TableItem_t *tableItem, uint32_t waitCyclesMax)
{
  uint8_t mac[6];
  uint16_t vlanid;
  
  mac[0] = ( tableItem->regMAC1 >> 24 ) & 0xFF;
  mac[1] = ( tableItem->regMAC1 >> 16 ) & 0xFF;
  mac[2] = ( tableItem->regMAC1 >> 8 )  & 0xFF;
  mac[3] =   tableItem->regMAC1 & 0xFF;
  mac[4] = ( tableItem->regMAC2 >> 8 )  & 0xFF;
  mac[5] =   tableItem->regMAC2 & 0xFF;
  vlanid = ( tableItem->regMAC2 >> 16 ) & 0xFFFF;

  MDR_KX028_CRITSECT_ENTER;
  MDR_KX028_MAC_TableDel(mac, vlanid, waitCyclesMax);
  MDR_KX028_CRITSECT_EXIT;
}

static inline void MDR_KX028_MAC_ReadTableItem( MDR_KX028_MAC_TableItem_t *tableItem, uint16_t entry, uint32_t waitCyclesMax)
{
  MDR_KX028_CRITSECT_ENTER;
  MDR_KX028_MAC_TableRead(tableItem, entry, waitCyclesMax);
  MDR_KX028_CRITSECT_EXIT;
}

static inline void MDR_KX028_MAC_WriteTableItem( MDR_KX028_MAC_TableItem_t *tableItem, uint16_t entry, uint32_t waitCyclesMax)
{
  MDR_KX028_CRITSECT_ENTER;
  MDR_KX028_MAC_TableWrite(tableItem, entry, waitCyclesMax);
  MDR_KX028_CRITSECT_EXIT;
}


void MDR_KX028_M2_ProcessTableItemAgeing(uint16_t hashAddr, uint32_t waitCyclesMax)
{
  MDR_KX028_MAC_TableItem_t tableItem;
  
  // Set Fresh if Item IsActive and not Static
  // Delete Fresh Items
  while (hashAddr)
  {
    MDR_KX028_MAC_ReadTableItem(&tableItem, hashAddr, waitCyclesMax);
    if (KX028_MAC_ITEM_IS_ACTIVE(tableItem))
    { 
      if (KX028_MAC_ITEM_IS_FRESH(tableItem))
        MDR_KX028_MAC_DeleteTableItem(&tableItem, waitCyclesMax);
      else if (!KX028_MAC_ITEM_IS_STATIC(tableItem))
      {
        KX028_MAC_ITEM_SET_FRESH(tableItem);
        MDR_KX028_MAC_WriteTableItem(&tableItem, hashAddr, waitCyclesMax);
      }
      
      if (KX028_MAC_ITEM_IS_HAS_COLLISION(tableItem))
        hashAddr = KX028_MAC_ITEM_COLLIS_PTR(tableItem);
      else
        hashAddr = 0;
    }
    else
      hashAddr = 0;
  }  
  
  
//  MDR_KX028_MAC_ReadTableItem(&tableItem, hashAddr, waitCyclesMax);
//  if (KX028_MAC_ITEM_IS_ACTIVE(tableItem))
//  {
//    if (KX028_MAC_ITEM_IS_HAS_COLLISION(tableItem))
//      coll_ptr = KX028_MAC_ITEM_COLLIS_PTR(tableItem);

//    if (KX028_MAC_ITEM_IS_FRESH(tableItem))
//      MDR_KX028_MAC_DeleteTableItem(&tableItem, waitCyclesMax);
//    else if (!KX028_MAC_ITEM_IS_STATIC(tableItem))
//    {
//      KX028_MAC_ITEM_SET_FRESH(tableItem);
//      MDR_KX028_MAC_WriteTableItem(&tableItem, hashAddr, waitCyclesMax);
//    }
//  }

//  // Process collision space entries
//  while( coll_ptr )
//  {
//    MDR_KX028_MAC_ReadTableItem(&tableItem, coll_ptr, waitCyclesMax);
//    if (KX028_MAC_ITEM_IS_ACTIVE(tableItem))
//    { 
//      if (KX028_MAC_ITEM_IS_FRESH(tableItem))
//        MDR_KX028_MAC_DeleteTableItem(&tableItem, waitCyclesMax);
//      else if (!KX028_MAC_ITEM_IS_STATIC(tableItem))
//      {
//        KX028_MAC_ITEM_SET_FRESH(tableItem);
//        MDR_KX028_MAC_WriteTableItem(&tableItem, coll_ptr, waitCyclesMax);
//      }
//      
//      if (KX028_MAC_ITEM_IS_HAS_COLLISION(tableItem))
//        coll_ptr = KX028_MAC_ITEM_COLLIS_PTR(tableItem);
//      else
//        coll_ptr = 0;
//    }
//    else
//      coll_ptr = 0;
//  }
}


//void MDR_KX028_M2_ProcessTablesAgeing(uint32_t framesToProcessMax, uint32_t waitCyclesMax)
uint16_t MDR_KX028_M2_ProcessTablesAgeing(uint16_t framesToProcessMax, uint16_t fromHashAddr, uint32_t waitCyclesMax)
{
  uint16_t i;  
  for (i = 0; i <  framesToProcessMax; i++)
  {
    MDR_KX028_M2_ProcessTableItemAgeing(fromHashAddr, waitCyclesMax);
    fromHashAddr += 1;
    if (fromHashAddr >= MDR_KX028_MAC_TABLE_LEN)
      fromHashAddr = 0;
  }
  
  return fromHashAddr; 
}


//========================   Frame Learning ==========================
#define _ETH_TYPE_VLAN  0x8100
#define IS_FRAME_VLAN_TAGGED(vlanTag) (((vlanTag >> 16) & 0xFFFF ) == _ETH_TYPE_VLAN)
#define FRAME_GET_VLAN_ID(vlanTag)      (vlanTag) & 0x0FFF
 

void MDR_KX028_LearnFrame( MDR_KX028_FrameInfo *frmInfo, bool static_entry, uint32_t waitCyclesMax, uint16_t enabledPortList)
{  
  uint32_t vlan_id, fwd_action;
  MDR_KX028_MAC_Entry_t  entryMAC = {.value = 0};
  MDR_KX028_VLAN_Entry_t entryVLAN;
  
  //  Check PortInd
  uint16_t portInd = frmInfo->ctrl_b.portNum;   
  if (portInd >= AXI_CLASS_PORT_COUNT) {
    MDR_KX028_Log(MDR_KX028_Log_Learn_WrongStructInd, portInd);
    return;
  }
  //  Check Port Used (Port used in board)
  uint32_t portList = (1 << portInd) & enabledPortList;
  if (portList == 0)
  {
    MDR_KX028_Log(MDR_KX028_Log_Learn_UnusedPort, portInd);
    return;
  }
  
  vlan_id = 0;
  if (IS_FRAME_VLAN_TAGGED(frmInfo->vlanTag)) 
    vlan_id = FRAME_GET_VLAN_ID(frmInfo->vlanTag);
  else 
  { 
    //get default vlan_id from Struct1    
    vlan_id = MDR_KX028_ReadAXI(AXI_CLASS_HW1_BASE_ADDR + KX028_PortOffsStruct1[portInd]);    
    vlan_id = _FLD2VAL(AXI_CLASS_STRUC1_PORT_FALLBACK_BDID, vlan_id);
  }  
  //printf( "vlan id %d\n", vlan_id );

//  uint8_t *pRxMac = (uint8_t *)(&frmInfo->destMAC_srcMAC) + 2;  
//  uint8_t src_mac[6];  
//////src_mac = ( uint8_t* )( ( ( uint32_t )ptr ) + 22 );  
//  src_mac[0] = ( ( frmInfo->destMAC_srcMAC >> 8 ) & 0xFF );
//  src_mac[1] = (   frmInfo->destMAC_srcMAC & 0xFF );
//  src_mac[2] = ( ( frmInfo->srcMAC >> 24 ) & 0xFF );
//  src_mac[3] = ( ( frmInfo->srcMAC >> 16 ) & 0xFF );
//  src_mac[4] = ( ( frmInfo->srcMAC >> 8 ) & 0xFF );
//  src_mac[5] = (   frmInfo->srcMAC & 0xFF );
  
  uint32_t macReg1 = (frmInfo->destMAC_srcMAC << 16) | (frmInfo->srcMAC >> 16);
  MDR_KX028_KeyEntryMAC_t keyEntryMAC;
  MDR_KX028_MAC_FillKeyEntryR(macReg1, frmInfo->srcMAC, vlan_id, entryMAC, &keyEntryMAC);
  //MDR_KX028_MAC_FillKeyEntry(src_mac, vlan_id, entryMAC, &keyEntryMAC);
  
  //printf( "src mac %02X:%02X:%02X:%02X:%02X:%02X\n", src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5] );
  //entryMAC = MDR_KX028_MAC_TableSearch(src_mac, vlan_id, waitCyclesMax);
  entryMAC = MDR_KX028_MAC_TableSearchByKey(&keyEntryMAC.key, waitCyclesMax);
  if( entryMAC.value == -1 )
  { // MAC Search fail
    //printf( "Search in mac fail\n" );    
    entryVLAN = MDR_KX028_VLAN_TableSearch(vlan_id, waitCyclesMax);
    if( entryVLAN.value == -1 ){
      //printf( "Search in vlan fail\n" );
      fwd_action = MDR_KX028_ReadAXI( AXI_CLASS_HW1_BASE_ADDR + AXI_CLASS_GLOBAL_CFG1 );
      fwd_action = ( fwd_action >> AXI_CLASS_CFG1_FRW_ACK_Pos ) & AXI_CLASS_CFG1_FRW_ACK_Msk;
    }else{
      fwd_action = (entryVLAN.value & MDR_KX028_VLAN_ENTRY_UCAST_HIT_ACT_MASK) >> MDR_KX028_VLAN_ENTRY_UCAST_HIT_ACT_POS;
    }

    keyEntryMAC.entry.value = MDR_KX028_MAC_ENTRY_FILL_DEF(portList, fwd_action, static_entry);    
    MDR_KX028_MAC_TableAddByKey(&keyEntryMAC, waitCyclesMax);
  }
  else
  { // MAC hash entry found
    //printf( "Search in mac success\n" );
    switch (frmInfo->ctrl_b.puntReason) {
      case KX028_FRAME_PUNT_SA_IS_ACTIVE: {
        //printf( "Punt is ACTIVE\n" );
        if (entryMAC.value & MDR_KX028_MAC_ENTRY_FRESH_Msk)
          entryMAC.value = entryMAC.value & ~MDR_KX028_MAC_ENTRY_FRESH_Msk;
        else
          return;
        //  Unreachable
        break;        
      }
      case KX028_FRAME_PUNT_SA_RELEARN: {
        //printf( "Punt is RELEARN\n" );
        if ((entryMAC.value & portList) == 0) 
        {
          entryMAC.value &= ~MDR_KX028_MAC_ENTRY_FWD_PORT_LIST_Msk;
          entryMAC.value |= portList;
        }
        else
          return;
        break;
      }       
      case KX028_FRAME_PUNT_SA_MISS:
        //printf( "Punt is MISS\n" );
        return;       
      default:
        return;
    }
    keyEntryMAC.entry.value = entryMAC.value;    
    MDR_KX028_MAC_TableUpdateByKey(&keyEntryMAC, waitCyclesMax);
  }
}

void MDR_KX028_FreeFrameBMU( uint32_t start_packet_pointer, int32_t pck_size)
{
  uint32_t tmp, next_buffer, offset, addrBMU;
  
  next_buffer = start_packet_pointer;
  offset = CFG_HGPI_LMEM_BUF_HDR_CHAIN_SIZE * 2;
  
  while(1)
  { 
    addrBMU = next_buffer & AXI_BLOCKS_BASE_ADDR_MSK;
    if (addrBMU == (CFG_BMU1_SEL_LMEM_ADDR | CBUS_BASE_ADDR))
      MDR_KX028_WriteAXI(AXI_BMU1_BASE_ADDR + AXI_BMU_FREE_CTRL, next_buffer);
    else if (addrBMU == (CFG_BMU1_SEL_LMEM_ADDR | CBUS_BASE_ADDR))
      MDR_KX028_WriteAXI(AXI_BMU2_BASE_ADDR + AXI_BMU_FREE_CTRL, next_buffer);
    else {
      //printf("\n\r\tall buffers for this packet was free\n\r");
      break;
    }
        
    pck_size -= (128 - offset);
    offset = CFG_HGPI_LMEM_BUF_HDR_CHAIN_SIZE;
    if( pck_size <= 0) {
      //printf("\n\r\tlen equal or below 0\n\r");
      break;
    }
    
    next_buffer = MDR_KX028_ReadAXI(tmp);
    //printf("\n\r\tbuffer for free: 0x%X, next buffer: 0x%X", tmp, next_buffer );
  } ;
}

static void MDR_KX028_ProcessFrame(uint32_t frameAddr, uint32_t waitCyclesMax, uint16_t enabledPortList)
{   
    MDR_KX028_FrameInfo frmInfo;
      
    MDR_KX028_CRITSECT_ENTER;// taskENTER_CRITICAL();

    frameAddr &= KX028_FRAME_PTR_ADDR_MSK;
    frmInfo.size = MDR_KX028_ReadAXI(frameAddr + KX028_FRAME_SIZE_ADDR_OFFS) & KX028_FRAME_SIZE_MSK;
    frmInfo.ctrl = MDR_KX028_ReadAXI(frameAddr + KX028_FRAME_PR_ADDR_OFFS);
        
    if( frmInfo.ctrl_b.ctrl & KX028_FRAME_CTRL_FP_RX_TS_REPORT_EN)
    {    
      //timestamp report      
    }
    else
    {      
      //packet      
      if( !( frmInfo.ctrl_b.ctrl & KX028_FRAME_CTRL_FP_PUNT_VALID ) )
      { 
        //packet doesn't punt with reasone code (normal packet)
      }
      else
      {    
        frmInfo.destMAC         = MDR_KX028_ReadAXI( frameAddr + CFG_LMEM_NEW_PACKET_OFFS);       //DST MAC
        frmInfo.destMAC_srcMAC  = MDR_KX028_ReadAXI( frameAddr + CFG_LMEM_NEW_PACKET_OFFS + 4);   //DST MAC, SRC MAC
        frmInfo.srcMAC          = MDR_KX028_ReadAXI( frameAddr + CFG_LMEM_NEW_PACKET_OFFS + 8);   //SRC MAC
        frmInfo.vlanTag         = MDR_KX028_ReadAXI( frameAddr + CFG_LMEM_NEW_PACKET_OFFS + 12);  //VLAN TAG
        
        //printf( "Pck ptr: 0x%X port num %d punt res %d ctrl 0x%X size %d\n", pck_pointer, packet_buffer[1], packet_buffer[3], packet_buffer[2], packet_buffer[0] );
        //printf( "Pck: %08X %08X %08X %08X\n", packet_buffer[4], packet_buffer[5], packet_buffer[6], packet_buffer[7] );
        
        switch( frmInfo.ctrl_b.puntReason ){          
          case KX028_FRAME_PUNT_SA_MISS:
          case KX028_FRAME_PUNT_SA_RELEARN:
          case KX028_FRAME_PUNT_SA_IS_ACTIVE:
            MDR_KX028_LearnFrame( &frmInfo, false, waitCyclesMax, enabledPortList);
            break;          
          default:
          break;
        }        
      }
    }
    
    // Free BMU buffers
    MDR_KX028_FreeFrameBMU(frameAddr, frmInfo.size);
    // Free polling register
    MDR_KX028_WriteAXI(CFG_NEW_PACKET_IN_LMEM_ADDR, 0 );
    // Reset HGPI
    MDR_KX028_WriteAXI(AXI_HGPI_BASE_ADDR + AXI_GPI_CTRL, AXI_GPI_CTRL_EN_Msk | AXI_GPI_CTRL_RESET_Msk );
    
    MDR_KX028_CRITSECT_EXIT;//    taskEXIT_CRITICAL();
}


//  Return processed frames count
uint32_t MDR_KX028_M2_ProcessTablesLearning(uint32_t framesToProcessMax, uint32_t waitCyclesMax, uint16_t enabledPortList)
{
  uint32_t i = 0;
  uint32_t pck_pointer = 0;
  
  for (i = 0; i < framesToProcessMax; i++)
  {
    pck_pointer = MDR_KX028_ReadAXI(CFG_NEW_PACKET_IN_LMEM_ADDR);   
    if (pck_pointer)
      MDR_KX028_ProcessFrame(pck_pointer, waitCyclesMax, enabledPortList);
    else
      break;  
  }
  
  return i;
}

