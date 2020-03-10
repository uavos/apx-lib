#pragma once

#include <cstring>
#include <type_traits>

#include "XbusStream.h"
#include "endian.h"

class XbusStreamReader : public XbusStream
{
public:
    explicit XbusStreamReader(const void *p, size_t size)
        : XbusStream(size)
        , _buf(static_cast<const uint8_t *>(p))

    {}

    inline void discard() { _pos = _size; }
    inline const uint8_t *buffer() const { return _buf; }
    inline const uint8_t *ptr() const { return &(_buf[_pos]); }

    template<typename _T>
    void read(_T &data);

    size_t read(void *dest, size_t sz)
    {
        if (!sz)
            return 0;
        size_t t = available();
        if (sz > t)
            sz = t;
        memcpy(dest, &_buf[_pos], sz);
        _pos += sz;
        return sz;
    }

    const char *read_string(size_t max_size)
    {
        if (available() <= 1)
            return nullptr;
        const char *s = reinterpret_cast<const char *>(ptr());
        size_t len = strnlen(s, max_size);
        if (len > max_size)
            return nullptr;
        reset(pos() + len + 1);
        return s;
    }

    template<typename _T, typename _Tout>
    inline _Tout read()
    {
        _T v;
        read(v);
        return static_cast<_Tout>(v);
    }
    template<typename _Tout>
    inline _Tout read()
    {
        _Tout v;
        read(v);
        return v;
    }

    template<typename _T>
    inline void operator>>(_T &data)
    {
        read(data);
    }

private:
    const uint8_t *_buf;

    template<typename _T, typename _Tout>
    inline void _get_data(_T &buf, _Tout &data);
};

// implementation

template<typename _T, typename _Tout>
void XbusStreamReader::_get_data(_T &src, _Tout &data)
{
    memcpy(&src, &_buf[_pos], sizeof(_T));

    switch (sizeof(_T)) {
    case 2:
        src = le16toh(src);
        break;

    case 4:
        src = le32toh(src);
        break;

    case 8:
        src = le64toh(src);
        break;

    default:
        return;
    }

    if (std::is_floating_point<_Tout>::value) {
        data = *static_cast<_Tout *>(static_cast<void *>(&src));
    } else {
        data = src;
    }
}

template<typename _T>
void XbusStreamReader::read(_T &data)
{
    if ((_pos + sizeof(_T)) > _size) {
        _pos = _size;
        data = 0;
        return;
    }

    switch (sizeof(_T)) {
    case 1:
        data = _buf[_pos];
        break;

    case 2:
        uint16_t data_le16;
        _get_data(data_le16, data);
        break;

    case 4:
        uint32_t data_le32;
        _get_data(data_le32, data);
        break;

    case 8:
        uint64_t data_le64;
        _get_data(data_le64, data);
        break;

    default:
        return;
    }
    _pos += sizeof(_T);
}
