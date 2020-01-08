#pragma once

#include <cinttypes>
#include <sys/types.h>

namespace mandala {

typedef uint16_t uid_t;

typedef float float_t;
typedef uint32_t uint_t;
typedef uint8_t byte_t;
typedef uint8_t enum_t;

enum type_id_t {
    type_float,
    type_uint,
    type_byte,
    type_enum,
};

enum sfmt_id_t {
    sfmt_u1, //(uint8)uint
    sfmt_u2, //(uint16)uint
    sfmt_u4, //(uint32)uint

    sfmt_f4,        //float32
    sfmt_f2,        //float16
    sfmt_f1,        //(uint8)float
    sfmt_f1div10,   //(uint8)float/10
    sfmt_f1mul10,   //(uint8)float*10
    sfmt_f1mul100,  //(uint8)float*100
    sfmt_f1s,       //(int8)float
    sfmt_f1smul100, //(int8)float*100

};

struct meta_t
{
    const char *name;
    const char *title;
    const char *descr;
    const char *units;
    const char *alias;
    const char *path;

    const uid_t uid : sizeof(uid_t) * 8;
    const uint8_t level : 3;
    const bool group : 1;
    const type_id_t type_id : 4;
    const uint8_t prec : 4;
    const sfmt_id_t sfmt : 4;
};

}; // namespace mandala
