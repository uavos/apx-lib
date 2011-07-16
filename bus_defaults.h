#ifndef _BUS_DEFAULTS_H_
#define _BUS_DEFAULTS_H_
#include <inttypes.h>
//=============================================================================
#define BUS_MAX_PACKET          1024
//=============================================================================
//default bus commands (service packets)
enum{
  apc_ACK=0,
  apc_ID,       //return _node_id
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
typedef uint8_t _node_sn[12]; //chip serial number
typedef struct{
  _node_sn      sn;
  uint32_t      mcu_id;         //MCU DEV_ID code
  uint8_t       dadr;           //device address (dynamically assigned)
  uint8_t       name[16];       //device name string
}__attribute__((packed)) _node_id; //filled by hwInit;
//-------------------------------------------------
typedef struct {
  _node_id  id;
  struct{             //general ports availability (dynamically set by drivers)
    uint8_t ctr;      //configurable controls
    uint8_t pwm;      //pwm servo outputs
    uint8_t out;      //logical outputs
    uint8_t inp;      //logical inputs
    uint8_t adc;      //analog inputs
  }ports;
  uint8_t vars[32];   //bitmask of known variables
}__attribute__((packed)) _node_info;
//=============================================================================
//=============================================================================
// Loader packet: [ALL PACKETS ARE BROADCAST SRV - src=0x80]
// hdr: src=0x80 (srv broadcast), cmd=apc_loader
// <0x80>,<apc_loader=0xFF>,<ldc_XXX>,<_node_sn>,<data...>
// to enter loader:
// <0x80>,<apc_loader=0xFF>,<ldc_init>,<_node_sn>
// loader filter packets by _node_sn
// loader must reply to standard command 'apc_ID'
//------------------------------------------------------------------------------
typedef enum {
  ldc_init=0,   // start loader, re: <_ldc_file> =flash size
  ldc_file,     // set file info <_ldc_file>, re: <_ldc_file>
  ldc_write,    // write file data <_ldc_write>, re: <_ldc_write::hdr>
} _ldr_cmd;
//loader data
typedef struct{
  uint32_t      start_address;
  uint16_t      size;        // flash size in kB
}__attribute__((packed)) _ldc_file;
typedef struct{
  struct{
    uint32_t    start_address;
    uint16_t    data_size;
  }__attribute__((packed)) hdr;
  uint8_t       data[256];
}__attribute__((packed)) _ldc_write;
//=============================================================================
#endif

