#pragma once

#include "XbusPacket.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {

typedef uint8_t guid_t[12]; //global unique node id

typedef uint16_t crc_t;

//---------------------------
// ident
//---------------------------
namespace ident {

typedef char name_t[16];     //device name string
typedef char version_t[16];  //fw version string
typedef char hardware_t[16]; //device hardware string

typedef struct
{
    name_t name;
    version_t version;
    hardware_t hardware;

    uint32_t hash; // dict hash

    union {
        struct
        {
            uint32_t dict : 1;   //set if dict available
            uint32_t reconf : 1; //set if conf was reset
            uint32_t loader : 1; //set if loader is running
            uint32_t busy : 1;   //set to postpone gcs requests
        } bits;
        uint32_t raw;
    } flags;

    static inline uint16_t psize()
    {
        return sizeof(name_t) + sizeof(version_t)
               + sizeof(hardware_t) + sizeof(flags.raw);
    }
    inline void read(XbusStreamReader *s)
    {
        s->read(name, sizeof(name));
        s->read(version, sizeof(version));
        s->read(hardware, sizeof(hardware));
        *s >> hash;
        *s >> flags.raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        s->write(name, sizeof(name));
        s->write(version, sizeof(version));
        s->write(hardware, sizeof(hardware));
        *s << hash;
        *s << flags.raw;
    }
} ident_s;

}; // namespace ident

//---------------------------
// msg
//---------------------------
namespace msg {

typedef enum {
    info,
    warn,
    err,
} type_e;

typedef uint8_t type_t;

}; // namespace msg

//---------------------------
// file
//---------------------------
namespace file {

typedef enum {
    ls,    // re: <names>
    info,  // q: <name> re: <name><fd><size>
    ropen, // q: <name> re: <name><fd><size>
    wopen, // q: <name> re: <name><fd><size>
    close, // q: <fd> re: <fd>
    read,  // q: <fd> re: <fd><offset><data>
    write, // q: <fd><offset><data> re: <fd>
    abort, // re: <name>
} op_e;

typedef uint8_t op_t;
typedef uint8_t fd_t;
typedef uint32_t offset_t;
typedef uint32_t size_t;

}; // namespace file

//---------------------------
// dict
//---------------------------
namespace dict {

typedef uint16_t fid_t;

typedef enum {
    group = 0,
    command,
    option = 4,
    byte,
    word,
    uid,
    real,
    string,
} type_e;

// field descriptor packed into dict array
typedef struct
{
    type_e type : 4;
    uint8_t array : 4; // >0 if array
    uint8_t group;     // group idx
    //strings 0-terminated appended
} field_s;

typedef struct
{
    uint32_t hash; // dict hash
    uint16_t size; // number of fields
} hdr_s;

// name, descr, units
static constexpr const size_t field_strings_count = 3;

}; // namespace dict

} // namespace node
} // namespace xbus
