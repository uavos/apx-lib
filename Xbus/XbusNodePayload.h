#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

class XbusNodePayload
{
public:

    // Node identification information
    typedef std::array<char,16> name_t;         //device name string
    typedef std::array<char,16> version_t;      //fw version string
    typedef std::array<char,16> hardware_t;     //device hardware string

    typedef struct {
        uint32_t    conf_reset     :1;      //set when conf was reset
        uint32_t    loader_support :1;      //set if loader available
        uint32_t    in_loader      :1;      //set in loader
        uint32_t    addressing     :1;      //set while CAN addressing
        uint32_t    reboot         :1;      //set when reboot requested
        uint32_t    busy           :1;      //set when flash write sequence

        uint32_t    padding :26;
    } flags_bits_t;

    typedef union {
        flags_bits_t    bits;
        uint32_t        raw;
    } flags_t;

    struct Ident {
        name_t      name;
        version_t   version;
        hardware_t  hardware;
        flags_t     flags;

        static inline uint16_t psize()
        {
            return std::tuple_size<name_t>()
                    + std::tuple_size<version_t>()
                    + std::tuple_size<hardware_t>()
                    + sizeof(flags_t);
        }
        void read(const uint8_t *p)
        {
            XbusStreamReader s(p);
            s >> name;
            s >> version;
            s >> hardware;
            s >> flags.raw;
        }
        void write(uint8_t *p) const
        {
            XbusStreamWriter s(p);
            s << name;
            s << version;
            s << hardware;
            s << flags.raw;
        }
    };



    // Node status
    typedef std::array<uint8_t,16> dump_t; //error dump or information dump

    struct Status {
        name_t      name;
        float       vbat;           // supply voltage [V]
        float       ibat;           // supply current [A]
        uint8_t     err_cnt;        // errors counter
        uint8_t     can_rxc;        // CAN received packets counter
        uint8_t     can_adr;        // CAN address
        uint8_t     can_err;        // CAN errors counter
        uint8_t     load;           // MCU load [%]
        dump_t      dump;           // error dump or information dump

        static inline uint16_t psize()
        {
            return std::tuple_size<name_t>()
                    + sizeof(uint16_t)*2
                    + sizeof(uint8_t)*5
                    + std::tuple_size<dump_t>();
        }
        void read(const uint8_t *p)
        {
            XbusStreamReader s(p);
            vbat=s.read<uint16_t,float>()/1000.0f;
            ibat=s.read<uint16_t,float>()/1000.0f;
            s >> err_cnt;
            s >> can_rxc;
            s >> can_adr;
            s >> can_err;
            s >> load;
            s >> dump;
        }
        void write(uint8_t *p) const
        {
            XbusStreamWriter s(p);
            s.write<uint16_t>(vbat*1000.0f);
            s.write<uint16_t>(ibat*1000.0f);
            s << err_cnt;
            s << can_rxc;
            s << can_adr;
            s << can_err;
            s << load;
            s << dump;
        }
    };


    // Node config identification
    struct IdentConf {
        uint8_t       pcnt;   //number of parameters
        uint16_t      csize;  //total size of 'conf' structure [bytes]
        uint32_t      mn;     //magic number

        static inline uint16_t psize()
        {
            return sizeof(uint8_t)
                    + sizeof(uint16_t)
                    + sizeof(uint32_t);
        }
        void read(const uint8_t *p)
        {
            XbusStreamReader s(p);
            s >> pcnt;
            s >> csize;
            s >> mn;
        }
        void write(uint8_t *p) const
        {
            XbusStreamWriter s(p);
            s << pcnt;
            s << csize;
            s << mn;
        }
    };


};

