#pragma once

#include <Mandala/MandalaMetaTree.h>

namespace mandala {

template<class M>
class Value : public M
{
public:
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
