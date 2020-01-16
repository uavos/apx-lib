#pragma once

#include <cinttypes>
#include <sys/types.h>

namespace mandala {

typedef uint16_t uid_t;

typedef float float_t;
typedef uint32_t dword_t;
typedef uint16_t word_t;
typedef uint8_t byte_t;
typedef uint8_t enum_t;

enum type_id_t {
    type_float,
    type_dword,
    type_word,
    type_byte,
    type_enum,
};

enum sfmt_id_t {
    sfmt_void,

    sfmt_bit,

    sfmt_u4, //(uint32)uint
    sfmt_u2, //(uint16)uint
    sfmt_u1, //(uint8)uint

    sfmt_f4,      //float32
    sfmt_f2,      //float16
    sfmt_f1,      //(uint8)float
    sfmt_f1_10,   //(uint8)float/10
    sfmt_f1_01,   //(uint8)float*10
    sfmt_f1_001,  //(uint8)float*100
    sfmt_f1_s,    //(int8)float
    sfmt_f1_s10,  //(int8)float/10
    sfmt_f1_s01,  //(int8)float*10
    sfmt_f1_s001, //(int8)float*100

};

struct meta_t
{
    const char *name;
    const char *title;
    const char *descr;
    const char *units;
    const char *path;

    const uid_t uid : sizeof(uid_t) * 8;
    const uint8_t level : 3;
    const bool group : 1;
    const type_id_t type_id : 4;
};

struct stream_item_t
{
    const uid_t uid : sizeof(uid_t) * 8;
    const sfmt_id_t sfmt : 4;
};

struct stream_id_t
{
    const char *name;
    const char *title;
    const stream_item_t *content;
};

}; // namespace mandala
