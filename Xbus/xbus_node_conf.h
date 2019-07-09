#pragma once
#include "xbus_node.h"

#ifdef __cplusplus
namespace xbus {
#endif


typedef uint8_t   ft_option_t;
typedef uint16_t  ft_varmsk_t;
typedef uint32_t  ft_uint_t;
typedef float     ft_float_t;
typedef uint8_t   ft_byte_t;
typedef uint8_t   ft_string_t [16];
typedef uint8_t   ft_lstr_t [64];
typedef uint8_t   ft_raw_t [128];

typedef uint8_t   node_fid_t;  //field id

typedef struct{
  union{
    struct{
    ft_float_t x;
    ft_float_t y;
    ft_float_t z;
    }__attribute__((packed));
    ft_float_t v[3];
  }__attribute__((packed));
}__attribute__((packed)) ft_vec_t;

typedef struct{
  uint32_t size;
  uint32_t code_size;
  uint32_t crc;
  char name[32];
}__attribute__((packed)) ft_script_t;

typedef struct {
  ft_float_t  Kp;
  ft_byte_t   Lp;
  ft_float_t  Kd;
  ft_byte_t   Ld;
  ft_float_t  Ki;
  ft_byte_t   Li;
  ft_byte_t   Lo;
}__attribute__((packed)) ft_regPID_t;
typedef struct {
  ft_float_t  Kp;
  ft_byte_t   Lp;
  ft_float_t  Ki;
  ft_byte_t   Li;
  ft_byte_t   Lo;
}__attribute__((packed)) ft_regPI_t;
typedef struct {
  ft_float_t  Kp;
  ft_byte_t   Lo;
}__attribute__((packed)) ft_regP_t;
typedef struct {
  ft_float_t  Kpp;
  ft_byte_t   Lpp;
  ft_float_t  Kp;
  ft_byte_t   Lp;
  ft_float_t  Ki;
  ft_byte_t   Li;
  ft_byte_t   Lo;
}__attribute__((packed)) ft_regPPI_t;

typedef enum{
  ft_option=0,
  ft_varmsk,
  ft_uint,
  ft_float,
  ft_byte,
  ft_string,
  ft_lstr,
  ft_vec,
  ft_ctr_deprecated,
  ft_pwm_deprecated,
  ft_gpio_deprecated,
  ft_regPID,
  ft_regPI,
  ft_regP,
  ft_regPPI,
  ft_raw,
  ft_script,
  //---------
  ft_cnt
} node_fieldtypes_t;

typedef struct{
  uint8_t       cnt;    //number of parameters
  uint16_t      size;   //total size of 'conf' structure [bytes]
  uint32_t      mn;     //magic number
}__attribute__((packed)) node_conf_info_t;



#ifdef __cplusplus
} //namespace
#endif
