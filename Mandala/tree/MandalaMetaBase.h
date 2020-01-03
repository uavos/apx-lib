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

class meta_base_t
{
};

class meta_0_t : public meta_base_t
{
};

class meta_1_t : public meta_base_t
{
};

class meta_2_t : public meta_base_t
{
};

class meta_3_t : public meta_base_t
{
};

class meta_4_t : public meta_base_t
{
};

class meta_5_t : public meta_base_t
{
};

template<typename T>
class meta_field_t : public meta_base_t
{
};

template<typename M>
class Text
{
public:
    constexpr const char *name() { return M::name; }
    constexpr const char *title() { return M::title; }
    constexpr const char *descr() { return M::descr; }
};

}; // namespace mandala
