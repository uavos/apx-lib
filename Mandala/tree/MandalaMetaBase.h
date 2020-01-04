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

template<typename T>
struct meta_base_t
{
    static constexpr const char *name() { return T::name; }
    static constexpr const char *title() { return T::title; }
    static constexpr const char *descr() { return T::descr; }
    static constexpr mandala::uid_t uid() { return T::uid; }
};

template<typename T>
struct meta_0_t : public meta_base_t<T>
{
};

template<typename T>
struct meta_1_t : public meta_base_t<T>
{
};

template<typename T>
struct meta_2_t : public meta_base_t<T>
{
};

template<typename T>
struct meta_3_t : public meta_base_t<T>
{
};

template<typename T>
struct meta_4_t : public meta_base_t<T>
{
};

template<typename T>
struct meta_5_t : public meta_base_t<T>
{
};

template<typename DataType, typename T>
class meta_field_t : public meta_base_t<T>
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

template<typename M>
struct Text
{
    static constexpr const char *name() { return M::name; }
    static constexpr const char *title() { return M::title; }
    static constexpr const char *descr() { return M::descr; }
};

template<class M>
class Value : public M
{
};

template<class M>
class StaticValue
{
public:
    operator M() const { return m_data; }
    M &data() const { return m_data; }

private:
    static M m_data;
};

template<class M>
M StaticValue<M>::m_data;

}; // namespace mandala
