#ifndef _BUS_DEFAULTS_H_
#define _BUS_DEFAULTS_H_
//=============================================================================
#define BUS_MAX_PACKET          1024
//=============================================================================
//default bus commands
enum{
  apc_ACK=0,
  apc_Info,     //return _node_info
  apc_SetAdr,   //set new address[_node_id] dtype and FUID used to filter

  //node configuration
  apc_ReadConfig,
  apc_WriteConfig,
  apc_ResetConfig,

  //user commands
  apc_user=100,
  apc_loader=0xFF
};
//=============================================================================
// node configuration. saved in EEPROM
typedef struct{
  unsigned char dtype;          //device type (stored in flash @end of page 0)
  unsigned char FUID[3];        //device serial number (set during loader prg)
  unsigned char dadr;           //device address (dynamically assigned)
  char          name[16];       //device name string
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
  unsigned char vars[32];   //bitmask of known variables
}__attribute__((packed)) _node_info;
//=============================================================================
//=============================================================================
#endif

