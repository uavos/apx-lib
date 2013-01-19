#ifndef _NODE_H_
#define _NODE_H_
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#define BUS_MAX_PACKET          2048    //2k MAX
//=============================================================================
// node information, dynamically assigned values
typedef uint8_t _node_sn[12]; //chip serial number
typedef uint8_t _node_name[16]; //device name string
typedef uint8_t _node_version[16]; //fw version string
//-------------------------------------------------
// firmware information, saved in FLASH section [CONFIG]
typedef struct {
  _node_name    name;
  _node_version version;
  uint8_t       crc;
  uint32_t      size;
}__attribute__((packed)) _fw_info;
// node information data to identify the node
typedef struct{
  _node_sn      sn;     //serial number
  _fw_info      fw;     //firmware info
}__attribute__((packed)) _node_info; //filled by hwInit, saved in RAM
//=============================================================================
// bus packet structure:
// <tag> is an optional byte (stored in conf)
// <tag> can be any value 7bits (<0x7F)
// <tag>=0x7F is reserved for 'shiva'
// MSB in <tag> flags service packet
// all service packets start with _node_sn in <data>
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
// default bus commands (service packets)
// all service packets addressed by _node_sn
enum{
  apc_ack=0,    //acknowledge - sent back in response to commands
  //------------------
  //system commands
  apc_info,     //return _fw_info
  apc_debug,    //debug message
  apc_reboot,   //reset/reboot node
  apc_mute,     //stop sending data (sensors)

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
  //user commands
  apc_user=100,
  apc_loader=0xFF
};
//=============================================================================
// Loader packet data (packet.srv.cmd=apc_loader):
// <ldc_COMMAND>,<data...>
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
enum{pt_pwm,pt_out,pt_inp,pt_adc}; //type of port
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
  int8_t zero;          //pwm zero shift -127[-1]..+127[+1]
  int8_t max;           //pwm maximum -127[-1]..+127[+1]
  int8_t min;           //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _pwm;
typedef struct {
  uint8_t dummy;
}__attribute__((packed)) _out;
//-----------------------------------------------------------------------------
typedef struct {
  uint8_t  protocol;    //protocol
  uint32_t baudrate;    //baud rate for some protocols
}__attribute__((packed)) _serial;
//-----------------------------------------------------------------------------
typedef struct {
  uint8_t  type;        //input capture type
  uint8_t  var_idx;     //i.e. ctr_ailerons or any other
  uint8_t  bitmask;    //mask if var bitfield or vect idx or array idx
}__attribute__((packed)) _capture;
//-----------------------------------------------------------------------------
typedef uint8_t   _ft_option;
typedef uint16_t  _ft_uint;
typedef float     _ft_float;
typedef _ctr      _ft_ctr;
typedef _pwm      _ft_pwm;
typedef _out      _ft_out;
typedef _serial   _ft_serial;
typedef _capture  _ft_capture;
typedef uint8_t   _ft_byte;
typedef uint8_t   _ft_string[16];
//-----------------------------------------------------------------------------
typedef enum{
  ft_option=0,
  ft_uint,
  ft_float,
  ft_ctr,
  ft_pwm,
  ft_out,
  ft_serial,
  ft_capture,
  ft_byte,
  ft_string,
  //---------
  ft_cnt
}_node_ft;
//=============================================================================
#endif

