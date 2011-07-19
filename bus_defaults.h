#ifndef _BUS_DEFAULTS_H_
#define _BUS_DEFAULTS_H_
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#define BUS_MAX_PACKET          1024
//=============================================================================
typedef struct{
  uint8_t       src;    //source node <dadr> or service <dest>|0x80
  uint8_t       adr;    //destination <var_idx> or service <cmd>
  uint8_t       data[BUS_MAX_PACKET-2];   //payload
}__attribute__((packed)) _bus_packet;
//=============================================================================
//default bus commands (service packets)
enum{
  apc_ACK=0,
  apc_ID,       //return _node_id
  apc_Info,     //return _node_info
  apc_SetAdr,   //set new address[dadr] _node_sn used to filter

  //node configuration
  apc_ReadConfig,
  apc_WriteConfig,
  apc_ResetConfig,

  //user commands
  apc_user=100,
  apc_loader=0xFF
};
//=============================================================================
// node information, dynamically assigned values
typedef uint8_t _node_sn[12]; //chip serial number
typedef uint8_t _node_name[16]; //device name string
typedef uint8_t _node_version[8]; //fw version string
//-------------------------------------------------
// firmware information, saved in FLASH section [CONFIG]
typedef struct {
  _node_name    name;
  _node_version version;
}__attribute__((packed)) _fw_info;
//-------------------------------------------------
typedef struct{
  _node_sn      sn;             //MCU SN (hardware reg)
  uint32_t      mcu_id;         //MCU DEV_ID code (hardware reg)
  uint8_t       dadr;           //device address (opt byte)
  _fw_info      fw;
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
  ldc_erase,    // force erase all user_app flash memory
} _ldr_cmd;
//loader data
typedef struct{
  uint32_t      start_address;
  uint16_t      size;        // flash size in kB
  uint8_t       xor_crc;     // all file data XORed
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

