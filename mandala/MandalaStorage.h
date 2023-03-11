#pragma once

#include <cinttypes>
#include <sys/types.h>

#include "MandalaMetaBase.h"

namespace mandala {
namespace storage {

// file header
struct fhdr_s
{
    // 32 bytes of file header format UID
    struct magic_s
    {
        char magic[16];   // i.e. APXTLM11
        uint16_t version; // version number (11)

        uint8_t _pad[14]; // set to zero when not used
    } magic;
    static_assert(sizeof(magic_s) == 32, "size error");

    // current version implementation
    uint64_t ts_file; // file timestamp [ms since epoch] 1970-01-01T00:00:00.000, Coordinated Universal Time

    uint32_t data_offset; // offset of data binary stream in bytes
    uint32_t aux_offset;  // offset of tail auxilary data in bytes

    char tags[256 - 32 - 16]; // tags list
};
static_assert(sizeof(fhdr_s) == 256, "size error");

// record header [32 bits]
/*struct rhdr_s
{
    uint32_t ts_data; // record timestamp [ms since start]
};

// data payload specifier [16 bits]
struct dspec_s
{
    enum bcnt_e {
        b1,
        b2,
        b4,
        b8,
    };

    uint8_t fmt : 1; // dspec format

    bcnt_e bcnt : 2; // number of bytes for payload
    bool sign : 1;   // value is signed
    bool real : 1;   // value is floating point type

    uid_t uid : 11; // mandala UID
};*/

enum dspec_e { // 4 bits
    ext,
    null,

    // unsigned raw
    u8,
    u16,
    u24,
    u32,
    u64,

    // floating point
    f16,
    f32,
    f64,

    // auxilary data
    aux,

    // angle
    a16,
    a32,

    //
    _rsv13,
    _rsv14,
    _rsv15,
};

struct dspec_s
{
    uid_t uid : 11; // mandala UID
    bool _rsv11 : 1;
    dspec_e dspec : 4;
};
static_assert(sizeof(dspec_s) == 2, "size error");

} // namespace storage
} // namespace mandala
