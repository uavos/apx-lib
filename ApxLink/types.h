#pragma once

#include <inttypes.h>
#include <sys/types.h>

// node information
typedef uint8_t _node_sn[12];           //serial number
typedef uint8_t _node_name[16];         //device name string
typedef uint8_t _node_hardware[16];     //device hardware string
typedef uint8_t _node_version[16];      //fw version string

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
#endif

