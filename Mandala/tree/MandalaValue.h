#pragma once

#include "MandalaMetaBase.h"

namespace mandala {

template<typename T>
class field_t
{
public:
    operator T() const { return m_value; }

    const T &value() const { return m_value; }

    bool setValue(const T &v)
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
    T m_value{};
};

}; // namespace mandala
