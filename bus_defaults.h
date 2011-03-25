#ifndef _BUS_DEFAULTS_H_
#define _BUS_DEFAULTS_H_
//=============================================================================
#define BUS_MAX_PACKET          1024
//=============================================================================
//default bus commands
enum{
  apc_ACK,
  apc_Info,     //return _node_info
  apc_SetAdr,   //set new address[dadr] _node_serial+<dadr>

  //node configuration
  apc_ReadConfig,
  apc_WriteConfig,
  apc_ResetConfig,

  //user commands
  apc_user,
  apc_loader=0xFF
};
//=============================================================================
// node configuration. saved in EEPROM
typedef struct{
  unsigned char dtype;          //device type (stored in flash @end of page 0)
  unsigned char FUID[3];        //device serial number (set during loader prg)
  unsigned char dadr;           //device address (dynamically assigned)
}__attribute__((packed)) _node_id;
//-------------------------------------------------
typedef struct {
  _node_id      id;
  struct{             //general ports availability (dynamically set by drivers)
    unsigned char ctr;      //configurable controls
    unsigned char pwm;      //pwm servo outputs
    unsigned char out;      //logical outputs
    unsigned char inp;      //logical inputs
    unsigned char adc;      //analog inputs
  }ports;
}__attribute__((packed)) _node_info;
//=============================================================================
//=============================================================================
#endif

