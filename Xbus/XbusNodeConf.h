#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {
namespace conf {


    typedef uint8_t   fid_t;  //field id

    // Node configuration parameters
    typedef uint8_t                     ft_option_t;
    typedef uint16_t                    ft_varmsk_t;
    typedef uint32_t                    ft_uint_t;
    typedef float                       ft_float_t;
    typedef uint8_t                     ft_byte_t;
    typedef std::array<char,16>         ft_string_t;
    typedef std::array<char,64>         ft_lstr_t;
    typedef std::array<char,128>        ft_raw_t;
    typedef std::array<ft_float_t,3>    ft_vec_t;


    typedef struct{
        uint32_t                size;
        uint32_t                code_size;
        uint32_t                crc;
        std::array<char,32>     name;

        inline void read(XbusStreamReader *s)
        {
            *s >> size;
            *s >> code_size;
            *s >> crc;
            *s >> name;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << size;
            *s << code_size;
            *s << crc;
            *s << name;
        }
    } ft_script_t;

    typedef struct {
        ft_float_t  Kp;
        ft_byte_t   Lp;
        ft_float_t  Kd;
        ft_byte_t   Ld;
        ft_float_t  Ki;
        ft_byte_t   Li;
        ft_byte_t   Lo;
    } ft_regPID_t;
    typedef struct {
        ft_float_t  Kp;
        ft_byte_t   Lp;
        ft_float_t  Ki;
        ft_byte_t   Li;
        ft_byte_t   Lo;
    } ft_regPI_t;
    typedef struct {
        ft_float_t  Kp;
        ft_byte_t   Lo;
    } ft_regP_t;
    typedef struct {
        ft_float_t  Kpp;
        ft_byte_t   Lpp;
        ft_float_t  Kp;
        ft_byte_t   Lp;
        ft_float_t  Ki;
        ft_byte_t   Li;
        ft_byte_t   Lo;
    } ft_regPPI_t;

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
    } ftype_t;

} // namespace
} // namespace
} // namespace


