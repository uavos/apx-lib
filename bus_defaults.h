#ifndef _CONFIG_DEFAULTS_H_
#define _CONFIG_DEFAULTS_H_
//=============================================================================
#define BUS_MAX_PACKET          1024
//=============================================================================
//default bus commands
enum{
  apc_ACK,
  apc_Info,     //return <dtype>+<FUID[3]>+<dadr>
  apc_SetAdr,   //set new address[dadr] <dtype>+<FUID[3]>+<dadr>

  //node configuration
  apc_ReadConfig,
  apc_WriteConfig,
  apc_ResetConfig,

  //user commands
  apc_user,
  apc_loader=0xFF
};
//=============================================================================
#endif

