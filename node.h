#ifndef _NODE_H_
#define _NODE_H_
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
  apc_ack=0,    //acknowledge - sent back in response to commands
  //------------------
  //system commands
  apc_id,       //return _node_id
  apc_adr,      //set new address[dadr] _node_sn used to filter
  apc_debug,    //stdout message
  apc_reboot,   //reset/reboot node
  apc_stop,     //stop sending data (sensors)

  //------------------
  //conf
  apc_fields,           //return conf descriptor
  apc_commands,         //return node commands descrptor
  apc_rconf,            //return _node_conf structure
  apc_wconf,            //save _node_conf
  apc_econf,            //reset conf to defaults

  //------------------
  //standard commands
  apc_data,     //send/receive some data [portNo]

  //------------------
  //deprecated node configuration
  apc_info,
  apc_readConfig,
  apc_writeConfig,
  apc_resetConfig,

  //------------------
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
// NODE CONF typedefs
//=============================================================================
typedef struct{
    uint8_t var_idx;    //i.e. ctr_ailerons or any other
    uint8_t bitmask;    //mask if var bitfield or vect idx or array idx
    int8_t  mult;       //multiplier (x0.1) -127[-12.7]..+127[+12.7]
    int8_t  diff;       //differential multiplier (x0.01+1) for pos/neg var value
    uint8_t weight;     //weight to set port value 0..255
    uint8_t speed;      //speed of change (x0.1) 0..25.5
    uint8_t port;       //output port number 0...x
    uint8_t type;       //type of port
}__attribute__((packed)) _ctr;
typedef struct {
  int8_t zero;         //pwm zero shift -127[-1]..+127[+1]
  int8_t max;          //pwm maximum -127[-1]..+127[+1]
  int8_t min;          //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _pwm;
//-----------------------------------------------------------------------------
typedef struct {
  int8_t  protocol;     //protocol
  int16_t speed;        //baud rate
}__attribute__((packed)) _serial;
//-----------------------------------------------------------------------------
typedef float     _ft_float;
typedef uint32_t  _ft_uint;
typedef uint8_t   _ft_bool;
typedef uint8_t   _ft_option;
typedef _ctr      _ft_ctr;
typedef _pwm      _ft_pwm;
typedef _serial   _ft_serial;
//-----------------------------------------------------------------------------
typedef enum{
  ft_uint=0,
  ft_bool,
  ft_option,
  ft_float,
  ft_ctr,
  ft_pwm,
  ft_serial,
  //---------
  ft_cnt
}_node_ft;
//=============================================================================
#endif

