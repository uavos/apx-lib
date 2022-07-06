#pragma once

#include <cinttypes>

template<class T>
class InstanceT
{
public:
    explicit InstanceT()
    {
        _instance = static_cast<T *>(this);
    }

    template<typename T2 = T>
    static inline constexpr T2 *instance()
    {
        return static_cast<T2 *>(_instance);
    }

private:
    static T *_instance;
};

template<class T>
T *InstanceT<T>::_instance{};
