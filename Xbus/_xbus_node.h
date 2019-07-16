#pragma once
#include "xbus.h"

#ifdef __cplusplus
namespace xbus {
#endif


typedef uint8_t node_guid_t [12];       //chip serial number
typedef uint8_t node_name_t [16];       //device name string
typedef uint8_t node_hardware_t [16];   //device hardware string
typedef uint8_t node_version_t [16];    //fw version string
typedef uint8_t node_cmd_t;             //service command

typedef struct {
  node_name_t     name;
  node_version_t  version;
  node_version_t  hardware;
  struct{
    uint32_t    conf_reset:     1;      //set when conf was reset
    uint32_t    loader_support: 1;      //set if loader available
    uint32_t    in_loader:      1;      //set in loader
    uint32_t    addressing:     1;      //set while CAN addressing
    uint32_t    reboot:         1;      //set when reboot requested
    uint32_t    busy:           1;      //set when flash write sequence
  }flags;
}__attribute__((packed)) node_info_t;

typedef struct {
  uint16_t      VBAT;           // supply voltage [mV]
  uint16_t      IBAT;           // supply current [mA]
}__attribute__((packed)) node_power_t;

typedef struct{
  node_power_t  power;
  uint8_t       err_cnt;        // errors counter
  uint8_t       can_rxc;        // CAN received packets counter
  uint8_t       can_adr;        // CAN address
  uint8_t       can_err;        // CAN errors counter
  uint8_t       load;           // MCU load
  uint8_t       dump[16];       // error dump or information dump
}__attribute__((packed)) node_status_t;

// system service commands (service packets)
typedef enum{
  apc_ack=0,    //acknowledge - sent back in response to commands
  //------------------
  //system commands
  apc_search,   //return just sn (broadcast)

  apc_info=8,   //return _node_info
  apc_nstat,    //return _node_name + _node_status
  apc_reboot,   //reset/reboot node
  apc_mute,     //stop sending data (sensors)
  apc_reconf,   //reset all conf to defaults
  apc_power,    //return _node_power
  apc_mem,      //print node memory usage
  apc_shock,    //shock test

  //------------------
  //conf
  apc_conf_inf=32,      //return _conf_inf structure
  apc_conf_cmds,        //return node commands descrptor
  apc_conf_dsc,         //return <num> parameter descriptor
  apc_conf_read,        //return parameter <num>
  apc_conf_write,       //save parameter <num>,<data>

  //------------------
  //script storage
  apc_script_file,     //get or set file info [<_flash_file>], re: [<_flash_file>]
  apc_script_read,     //read data <_flash_data_hdr>, re: <_flash_data>
  apc_script_write,    //write data <_flash_data>, re: <_flash_data_hdr>

  apc_msg=64,   //text message
  //------------------
  //user commands
  apc_user=128,
  apc_loader=0xFF
} node_comands_t;

typedef enum {
  ldc_init=0,   // start loader, re: <_flash_file> =flash size
  ldc_file,     // set file info <_flash_file>, re: <_flash_file>
  ldc_write     // write data <_flash_data>, re: <_flash_data_hdr>
} node_comands_loader_t;


typedef struct {
  hdr_t         hdr;
  node_guid_t   guid;
  node_cmd_t    cmd;
  //payload data follows
} __attribute__((packed)) hdr_node_t;


#ifdef __cplusplus
} //namespace
#endif
