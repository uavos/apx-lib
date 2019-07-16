#pragma once
#include "XbusNode.h"

class XbusNodeIdent: protected XbusNode
{
public:
    XbusNodeIdent(uint8_t *packet_ptr);

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
    } flags_bits_t;

    typedef union {
        flags_bits_t    bits;
        uint32_t        raw;
    } flags_t;

    virtual bool isValid(const uint16_t packet_size) const override;

    struct Ident {
        name_t      name;
        version_t   version;
        hardware_t  hardware;
        flags_t     flags;
    };

    void read(Ident &d);
    void write(const Ident &d);
};
