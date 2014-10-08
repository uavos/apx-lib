#ifndef _NODE_H_
#define _NODE_H_
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#define BUS_MAX_PACKET          1024    //2k MAX
//=============================================================================
// node information, dynamically assigned values
typedef uint8_t _node_sn[12];           //chip serial number
typedef uint8_t _node_name[16];         //device name string
typedef uint8_t _node_hardware[16];     //device hardware string
typedef uint8_t _node_version[16];      //fw version string
//-------------------------------------------------
// hard coded firmware information
typedef struct {
  _node_name    name;
  _node_version version;
  _node_version hardware;
  uint32_t      size;   //program memory size
}__attribute__((packed)) _node_info;
// node power, returned by request
typedef struct {
  uint16_t    VBAT;           // supply voltage [mV]
  uint16_t    IBAT;           // supply current [mA]
}__attribute__((packed)) _node_power;
// node status, returned by request
typedef struct{
  _node_power power;
  uint8_t     err_cnt;        // errors counter
  uint8_t     can_rxc;        // CAN received packets counter
  uint8_t     can_adr;        // CAN address
  uint8_t     can_err;        // CAN errors counter
  uint8_t     dump[16];       // error dump or information dump
}_node_status;
// local node description struct (hardware.h)
typedef struct{
  _node_sn      sn;     //serial number
  _node_info    info;   //firmware/hardware info
  _node_status  status; //dynamic status
}__attribute__((packed)) _node;
//=============================================================================
// bus packet structure:
// service packets filtered by _node_sn, or 0x00000 for broadcast
typedef struct{
  uint8_t       id;   //<var_idx>
  union {
    uint8_t     data[BUS_MAX_PACKET-1];   //payload
    struct{
      _node_sn  sn;     //filter for service
      uint8_t   cmd;    //service command
      uint8_t   data[BUS_MAX_PACKET-1-sizeof(_node_sn)-1];   //service data
    }srv;
    struct{
      uint8_t   tag;    //user tag
      uint8_t   packet[BUS_MAX_PACKET-1-1];   //tagged packet
    }tagged;
  };
}__attribute__((packed)) _bus_packet;
#define bus_packet_size_hdr             (1)
#define bus_packet_size_hdr_srv         (bus_packet_size_hdr+sizeof(_node_sn)+1)
#define bus_packet_size_hdr_tagged      (bus_packet_size_hdr+1)
//=============================================================================
// system service commands (service packets)
enum{
  apc_ack=0,    //acknowledge - sent back in response to commands
  //------------------
  //system commands
  apc_info,     //return _node_info
  apc_nstat,    //return _node_name + _node_status
  apc_reboot,   //reset/reboot node
  apc_mute,     //stop sending data (sensors)
  apc_reconf,   //reset all conf to defaults
  apc_power,    //return _node_power

  //------------------
  //conf
  apc_conf_inf=32,      //return _conf_inf structure
  apc_conf_cmds,        //return node commands descrptor
  apc_conf_dsc,         //return <num> parameter descriptor
  apc_conf_read,        //return parameter <num>
  apc_conf_write,       //save parameter <num>,<data>

  //------------------
  //user commands
  apc_user=100,
  apc_loader=0xFF
};
//=============================================================================
// Loader packet data (packet.srv.cmd=apc_loader):
// <ldc_COMMAND>,<data...>
//------------------------------------------------------------------------------
typedef enum {
  ldc_init=0,   // start loader, re: <_ldc_file> =flash size
  ldc_file,     // set file info <_ldc_file>, re: <_ldc_file>
  ldc_write    // write file data <_ldc_write>, re: <_ldc_write::hdr>
} _ldr_cmd;
//loader data
typedef struct{
  uint32_t      start_address;
  uint32_t      size;        // flash size in kB
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
typedef uint8_t   _ft_option;
typedef uint16_t  _ft_varmsk;
typedef uint32_t  _ft_uint;
typedef float     _ft_float;
typedef uint8_t   _ft_byte;
typedef uint8_t   _ft_string[16];
typedef uint8_t   _ft_lstr[64];
typedef uint8_t   _ft_protocol[128];
typedef struct{
  _ft_float x;
  _ft_float y;
  _ft_float z;
}__attribute__((packed)) _vec;
typedef _vec      _ft_vec;
typedef struct{
  _ft_varmsk varmsk;  //var idx with mask
  _ft_float  mult;    //multiplier (x0.1) -127[-12.7]..+127[+12.7]
  int8_t  diff;       //differential multiplier (x0.01+1) for pos/neg var value
  uint8_t speed;      //speed of change (x0.1) 0..25.5
  uint8_t pwm_ch;     //pwm channel number 0...x
}__attribute__((packed)) _ctr;
typedef _ctr      _ft_ctr;
typedef struct {
  int8_t zero;          //pwm zero shift -127[-1]..+127[+1]
  int8_t max;           //pwm maximum -127[-1]..+127[+1]
  int8_t min;           //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _pwm;
typedef _pwm      _ft_pwm;
typedef struct {
  _ft_varmsk varmsk;  //var idx with mask
  _ft_byte   opt;     //option: b7=1 if INVERTED, b[0:6]=protocol
  _ft_float  mult;    //multiplier
  _ft_float  bias;    //bias
}__attribute__((packed)) _gpio;
typedef _gpio     _ft_gpio;
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
}__attribute__((packed)) _regPID;
typedef _regPID  _ft_regPID;
typedef struct {
  _ft_float  Kp;
  _ft_byte   Lp;
  _ft_float  Ki;
  _ft_byte   Li;
  _ft_byte   Lo;
}__attribute__((packed)) _regPI;
typedef _regPI  _ft_regPI;
typedef struct {
  _ft_float  Kp;
  _ft_byte   Lo;
}__attribute__((packed)) _regP;
typedef _regP  _ft_regP;
typedef struct {
  _ft_float  Kpp;
  _ft_byte   Lpp;
  _ft_float  Kp;
  _ft_byte   Lp;
  _ft_float  Ki;
  _ft_byte   Li;
  _ft_byte   Lo;
}__attribute__((packed)) _regPPI;
typedef _regPPI  _ft_regPPI;
//-----------------------------------------------------------------------------
//serial protocol state machine
enum{
  sp_off=0,
  //operation
  sp_init,      //<fields>
  sp_write,     //<period ms[U16]>,<fields>
  sp_read,      //<fields>
  sp_wupd,      //<var_idx>,<fields> [write on var update]
  //fields
  sp_skip=0x10, //ignore byte
  sp_byte,      //<byte>
  sp_float,     //<varmsk>
  sp_value,     //<varmsk>,<type>,<bias>,<mult>
  sp_servo,     //<pwm idx>,<type>,<bias>,<mult>
  sp_CRC16_HL,  //<poly>,<initial>,<start pos>,<end pos>
  sp_CRC16_LH,  //<poly>,<initial>,<start pos>,<end pos>
  sp_CRC8XOR,   //<initial>,<start pos>,<end pos>
  sp_CRC8SUM,   //<initial>,<start pos>,<end pos>
  //value types
  sp_U32=0x40,
  sp_S32,
  sp_U16,
  sp_S16,
  sp_U8,
  sp_S8,
  sp_F32,
  sp_string,

  sp_Cnt
};
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
  ft_protocol,
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
#endif

