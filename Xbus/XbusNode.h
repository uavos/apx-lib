#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {

typedef std::array<uint8_t, 12> guid_t; //global unique node id
typedef uint8_t cmd_t;                  //service command

// system service commands (service packets)
typedef enum {
    apc_ack = 0, //acknowledge - sent back in response to commands
    //------------------
    //system commands
    apc_search, //return just sn (broadcast)

    apc_info = 8, //return _node_info
    apc_nstat,    //return _node_name + _node_status
    apc_reboot,   //reset/reboot node
    apc_mute,     //stop sending data (sensors)
    apc_reconf,   //reset all conf to defaults
    apc_power,    //return _node_power
    apc_mem,      //print node memory usage
    apc_shock,    //shock test

    //------------------
    //conf
    apc_conf_inf = 32, //return _conf_inf structure
    apc_conf_cmds,     //return node commands descrptor
    apc_conf_dsc,      //return <num> parameter descriptor
    apc_conf_read,     //return parameter <num>
    apc_conf_write,    //save parameter <num>,<data>

    //------------------
    //script storage
    apc_script_file,  //get or set file info [<_flash_file>], re: [<_flash_file>]
    apc_script_read,  //read data <_flash_data_hdr>, re: <_flash_data>
    apc_script_write, //write data <_flash_data>, re: <_flash_data_hdr>

    apc_msg = 64, //text message
    //------------------
    //user commands
    apc_user = 128,
    apc_loader = 0xFF
} comands_t;

typedef enum {
    ldc_init = 0, // start loader, re: <_flash_file> =flash size
    ldc_file,     // set file info <_flash_file>, re: <_flash_file>
    ldc_write     // write data <_flash_data>, re: <_flash_data_hdr>
} comands_loader_t;

// Node identification information
typedef std::array<char, 16> name_t;     //device name string
typedef std::array<char, 16> version_t;  //fw version string
typedef std::array<char, 16> hardware_t; //device hardware string

typedef struct
{
    uint32_t conf_reset : 1;     //set when conf was reset
    uint32_t loader_support : 1; //set if loader available
    uint32_t in_loader : 1;      //set in loader
    uint32_t addressing : 1;     //set while CAN addressing
    uint32_t reboot : 1;         //set when reboot requested
    uint32_t busy : 1;           //set when flash write sequence

    uint32_t padding : 26;
} flags_bits_t;

typedef union {
    flags_bits_t bits;
    uint32_t raw;
} flags_t;

struct Ident
{
    name_t name;
    version_t version;
    hardware_t hardware;
    flags_t flags;

    static inline uint16_t psize()
    {
        return std::tuple_size<name_t>() + std::tuple_size<version_t>()
               + std::tuple_size<hardware_t>() + sizeof(flags_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> name;
        *s >> version;
        *s >> hardware;
        *s >> flags.raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << name;
        *s << version;
        *s << hardware;
        *s << flags.raw;
    }
};

// Node status
typedef std::array<uint8_t, 16> dump_t; //error dump or information dump

struct Status
{
    name_t name;
    float vbat;      // supply voltage [V]
    float ibat;      // supply current [A]
    uint8_t err_cnt; // errors counter
    uint8_t can_rxc; // CAN received packets counter
    uint8_t can_adr; // CAN address
    uint8_t can_err; // CAN errors counter
    uint8_t load;    // MCU load [0..255]
    dump_t dump;     // error dump or information dump

    static inline uint16_t psize()
    {
        return std::tuple_size<name_t>() + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 5
               + std::tuple_size<dump_t>();
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> name;
        vbat = s->read<uint16_t, float>() / 1000.0f;
        ibat = s->read<uint16_t, float>() / 1000.0f;
        *s >> err_cnt;
        *s >> can_rxc;
        *s >> can_adr;
        *s >> can_err;
        *s >> load;
        *s >> dump;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << name;
        s->write<uint16_t>(vbat * 1000.0f);
        s->write<uint16_t>(ibat * 1000.0f);
        *s << err_cnt;
        *s << can_rxc;
        *s << can_adr;
        *s << can_err;
        *s << load;
        *s << dump;
    }
};

// Node config identification
struct IdentConf
{
    uint8_t pcnt;   //number of parameters
    uint16_t csize; //total size of 'conf' structure [bytes]
    uint32_t mn;    //magic number

    static inline uint16_t psize() { return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t); }
    inline void read(XbusStreamReader *s)
    {
        *s >> pcnt;
        *s >> csize;
        *s >> mn;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << pcnt;
        *s << csize;
        *s << mn;
    }
};

} // namespace node
} // namespace xbus
