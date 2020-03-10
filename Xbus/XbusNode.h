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

typedef struct
{
    uint32_t format; // protocol format
    uint32_t hash;   // structure hash
    union {
        struct
        {
            uint32_t files : 4;  //number of files
            uint32_t busy : 1;   //set to postpone gcs requests
            uint32_t reconf : 1; //set if conf was reset
        } bits;
        uint32_t raw;
    } flags;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) * 3;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> format;
        *s >> hash;
        *s >> flags.raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << format;
        *s << hash;
        *s << flags.raw;
    }
} ident_s;

static constexpr const size_t strings_count = 3;

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
    re,    // re: <op><payload>
    info,  // q: <name> re: <name><size>
    ropen, // q: <name> re: <name><size>
    wopen, // q: <name> re: <name><size>
    close, // q: <name> re: <name><size>
    read,  // q: <name><offset> re: <name><offset><data>
    write, // q: <name><offset><data> re: <name>
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
static constexpr const size_t strings_count = 3;

}; // namespace dict

} // namespace node
} // namespace xbus
