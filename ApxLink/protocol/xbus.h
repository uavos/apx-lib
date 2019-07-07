#pragma once

#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#define BUS_MAX_PACKET          1024    //2k MAX

typedef uint8_t _node_sn[12];           //chip serial number

// bus packet structure:
// service packets filtered by _node_sn, or 0x00000 for broadcast
#define bus_packet_size_hdr             (1)
#define bus_packet_size_hdr_srv         (bus_packet_size_hdr+sizeof(_node_sn)+1)
#define bus_packet_size_hdr_tagged      (bus_packet_size_hdr+3)
#define bus_packet_size_hdr_uav         (bus_packet_size_hdr+2+1)
typedef struct{
  uint8_t       id;   //<var_idx>
  union {
    uint8_t     data[BUS_MAX_PACKET-bus_packet_size_hdr];   //payload
    struct{
      _node_sn  sn;     //filter for service
      uint8_t   cmd;    //service command
      uint8_t   data[BUS_MAX_PACKET-bus_packet_size_hdr_srv];   //service data
    }srv;
    struct{
      uint8_t   nodeID;    //address
      uint8_t   prio;      //priority
      uint8_t   id;     //<var_idx>
      uint8_t   data[BUS_MAX_PACKET-bus_packet_size_hdr_tagged];
    }prio;
    struct{
      uint16_t  squawk;
      uint8_t   id;     //<var_idx>
      uint8_t   data[BUS_MAX_PACKET-bus_packet_size_hdr_tagged];
    }uav;
  };
}__attribute__((packed)) _bus_packet;
//=============================================================================
// system service commands (service packets)
enum{
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
};
//=============================================================================
// Loader packet data (packet.srv.cmd=apc_loader):
// <ldc_COMMAND>,<data...>
//------------------------------------------------------------------------------
typedef enum {
  ldc_init=0,   // start loader, re: <_flash_file> =flash size
  ldc_file,     // set file info <_flash_file>, re: <_flash_file>
  ldc_write     // write data <_flash_data>, re: <_flash_data_hdr>
} _ldr_cmd;
//loader data
typedef struct{
  uint32_t      start_address;
  uint32_t      size;        // available size in bytes
  uint8_t       xor_crc;     // all file data XORed
}__attribute__((packed)) _flash_file;
typedef struct{
  uint32_t    start_address;
  uint16_t    data_size;
}__attribute__((packed)) _flash_data_hdr;
typedef struct{
  _flash_data_hdr hdr;
  uint8_t       data[256];
}__attribute__((packed)) _flash_data;
//=============================================================================
// NODE CONF typedefs
//=============================================================================
typedef uint8_t   _ft_option;
typedef uint16_t  _ft_varmsk;
typedef uint32_t  _ft_uint;
typedef float     _ft_float;
typedef uint8_t   _ft_byte;
typedef uint8_t   _ft_string[16];
typedef uint8_t   _ft_lstr[64];
typedef uint8_t   _ft_raw[128];
typedef struct{
  union{
    struct{
    _ft_float x;
    _ft_float y;
    _ft_float z;
    }__attribute__((packed));
    _ft_float v[3];
  }__attribute__((packed));
}__attribute__((packed)) _ft_vec;
typedef struct{
  uint32_t size;
  uint32_t code_size;
  uint32_t crc;
  char name[32];
}__attribute__((packed)) _ft_script;
typedef struct{
  _ft_varmsk varmsk;  //var idx with mask
  _ft_float  mult;    //multiplier (x0.1) -127[-12.7]..+127[+12.7]
  int8_t  diff;       //differential multiplier (x0.01+1) for pos/neg var value
  uint8_t speed;      //speed of change (x0.1) 0..25.5
  uint8_t pwm_ch;     //pwm channel number 0...x
}__attribute__((packed)) _ft_ctr;
typedef struct {
  int8_t zero;          //pwm zero shift -127[-1]..+127[+1]
  int8_t max;           //pwm maximum -127[-1]..+127[+1]
  int8_t min;           //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _ft_pwm;
typedef struct {
  _ft_varmsk varmsk;  //var idx with mask
  _ft_byte   opt;     //option: b7=1 if INVERTED, b[0:6]=protocol
  _ft_float  mult;    //multiplier
  _ft_float  bias;    //bias
}__attribute__((packed)) _ft_gpio;
//-----------------------------------------------------------------------------
//APCFG support
typedef struct {
  _ft_float  Kp;
  _ft_byte   Lp;
  _ft_float  Kd;
  _ft_byte   Ld;
  _ft_float  Ki;
  _ft_byte   Li;
  _ft_byte   Lo;
}__attribute__((packed)) _ft_regPID;
typedef struct {
  _ft_float  Kp;
  _ft_byte   Lp;
  _ft_float  Ki;
  _ft_byte   Li;
  _ft_byte   Lo;
}__attribute__((packed)) _ft_regPI;
typedef struct {
  _ft_float  Kp;
  _ft_byte   Lo;
}__attribute__((packed)) _ft_regP;
typedef struct {
  _ft_float  Kpp;
  _ft_byte   Lpp;
  _ft_float  Kp;
  _ft_byte   Lp;
  _ft_float  Ki;
  _ft_byte   Li;
  _ft_byte   Lo;
}__attribute__((packed)) _ft_regPPI;
//=============================================================================
typedef enum{
  ft_option=0,
  ft_varmsk,
  ft_uint,
  ft_float,
  ft_byte,
  ft_string,
  ft_lstr,
  ft_vec,
  ft_ctr,
  ft_pwm,
  ft_gpio,
  ft_regPID,
  ft_regPI,
  ft_regP,
  ft_regPPI,
  ft_raw,
  ft_script,
  //---------
  ft_cnt
}_node_ft;
//-----------------------------------------------------------------------------
typedef struct{
  uint8_t       cnt;    //number of parameters
  uint16_t      size;   //total size of 'conf' structure [bytes]
  uint32_t      mn;     //magic number
}__attribute__((packed)) _conf_inf;
//=============================================================================
//=============================================================================
//blackbox reader
typedef struct{
  uint32_t magic;
  uint32_t uptime;
  uint32_t oncnt;
  uint32_t evtcnt[3];
  uint32_t rec_size; //blocks
  uint8_t  padding[32-20-4];
  uint32_t crc;
}__attribute__((packed)) _bb_hdr;

typedef enum {bbr_hdr,bbr_data} _bb_req_type;
//=============================================================================
