#pragma once

#include <cstring>
#include <type_traits>

#include "XbusStream.h"
#include "endian.h"

class XbusStreamWriter : public XbusStream
{
public:
    explicit XbusStreamWriter(void *p, size_t size)
        : XbusStream(size)
        , _buf(static_cast<uint8_t *>(p))
    {}

    inline uint8_t *buffer() const { return _buf; }
    inline uint8_t *ptr() const { return &(_buf[_pos]); }

    template<typename _T, typename _Tin>
    size_t write(const _Tin data);

    size_t write(const void *src, size_t sz)
    {
        size_t t = available();
        if (sz > t)
            sz = t;
        memcpy(&_buf[_pos], src, sz);
        _pos += sz;
        return sz;
    }

    template<typename _T>
    inline void operator<<(const _T data)
    {
        write<_T>(data);
    }

private:
    uint8_t *_buf;

    template<typename _T, typename _Tin>
    inline void _set_data(_T &buf, _Tin data);
};

// implementation
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

template<typename _T, typename _Tin>
void XbusStreamWriter::_set_data(_T &src, _Tin data)
{
    if (std::is_floating_point<_Tin>::value) {
        src = *static_cast<const _T *>(static_cast<const void *>(&data));
    } else {
        src = data;
    }

    // htoleXX functions may be empty macros,
    // switch will be optimized-out
    switch (sizeof(_T)) {
    case 2:
        src = htole16(src);
        break;

    case 4:
        src = htole32(src);
        break;

    case 8:
        src = htole64(src);
        break;

    default:
        return;
    }

    memcpy(&_buf[_pos], &src, sizeof(src));
}
#pragma GCC diagnostic pop

template<typename _T, typename _Tin>
size_t XbusStreamWriter::write(const _Tin data)
{
    static_assert(!(std::is_pointer<_Tin>::value || std::is_pointer<_T>::value), "can't stream pointer");

    if ((_pos + sizeof(_T)) > _size) {
        return 0;
    }

    switch (sizeof(_T)) {
    case 1:
        _buf[_pos] = static_cast<_T>(data);
        break;

    case 2:
        uint16_t data_le16;
        _set_data(data_le16, static_cast<_T>(data));
        break;

    case 4:
        uint32_t data_le32;
        _set_data(data_le32, static_cast<_T>(data));
        break;

    case 8:
        uint64_t data_le64;
        _set_data(data_le64, static_cast<_T>(data));
        break;

    default:
        return 0;
    }
    _pos += sizeof(_T);
    return sizeof(_T);
}
