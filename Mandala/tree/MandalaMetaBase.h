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
    sfmt_f4,
    sfmt_f2,
    sfmt_u1,
    sfmt_u2,
    sfmt_u4,
    sfmt_u10,
    sfmt_u01,
    sfmt_u001,
    sfmt_s1,
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

template<typename T>
struct tree_base_t
{
    static constexpr mandala::uid_t uid() { return T::uid; }
};

template<typename T>
struct tree_0_t : public tree_base_t<T>
{
};

template<typename T>
struct tree_1_t : public tree_base_t<T>
{
};

template<typename T>
struct tree_2_t : public tree_base_t<T>
{
};

template<typename T>
struct tree_3_t : public tree_base_t<T>
{
};

template<typename T>
struct tree_4_t : public tree_base_t<T>
{
};

template<typename T>
struct tree_5_t : public tree_base_t<T>
{
};

template<typename DataType, typename T>
class tree_value_t : public tree_base_t<T>
{
public:
    operator DataType() const { return m_value; }

    const DataType &get() const { return m_value; }

    bool set(const DataType &v)
    {
        if (m_value == v)
            return false;
        m_value = v;
        return true;
    }

    /*explicit field_t() {}
    field_t(const field_t &) = delete;
    field_t &operator=(const field_t &) = delete;
    field_t(field_t &&) = delete;
    field_t &operator=(field_t &&) = delete;*/

private:
    DataType m_value{};
};

}; // namespace mandala
