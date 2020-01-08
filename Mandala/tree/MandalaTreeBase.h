#pragma once

#include "MandalaMetaBase.h"
#include "MandalaStream.h"

namespace mandala {

template<typename _Tree>
struct tree_base_t
{
    static constexpr mandala::uid_t uid() { return _Tree::uid; }
};

template<typename _Tree>
struct tree_0_t : public tree_base_t<_Tree>
{
};

template<typename _Tree>
struct tree_1_t : public tree_base_t<_Tree>
{
};

template<typename _Tree>
struct tree_2_t : public tree_base_t<_Tree>
{
};

template<typename _DataType, typename _Tree>
class tree_value_t : public tree_base_t<_Tree>, private stream
{
public:
    constexpr operator _DataType() const { return m_value; }

    constexpr const _DataType &get() const { return m_value; }

    bool set(const _DataType &v)
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

    constexpr inline size_t pack(void *buf) const
    {
        return stream::pack<_Tree::meta.sfmt>(buf, m_value);
    }
    constexpr inline size_t unpack(const void *buf)
    {
        return stream::unpack<_Tree::meta.sfmt>(buf, m_value);
    }

    constexpr inline size_t copy_to(void *buf) const
    {
        if (std::is_floating_point<_DataType>::value) {
            return stream::pack<sfmt_f4>(buf, m_value);
        } else {
            return pack_raw_int(buf, m_value);
        }
    }
    constexpr inline size_t copy_from(const void *buf)
    {
        if (std::is_floating_point<_DataType>::value) {
            return stream::unpack<sfmt_f4>(buf, m_value);
        } else {
            return unpack_raw_int(buf, m_value);
        }
    }

protected:
    _DataType m_value{};
};

}; // namespace mandala
