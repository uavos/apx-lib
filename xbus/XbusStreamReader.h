/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <cinttypes>
#include <cstring>
#include <sys/types.h>
#include <type_traits>

#include "XbusStream.h"
#include "xbus_endian.h"

class XbusStreamReader : public XbusStream
{
public:
    explicit XbusStreamReader(const void *p, size_t size)
        : XbusStream(size)
        , _buf(static_cast<const uint8_t *>(p))

    {}

    void init(const void *p, size_t size)
    {
        _buf = static_cast<const uint8_t *>(p);
        resize(size);
    }
    void init(const void *p)
    {
        _buf = static_cast<const uint8_t *>(p);
    }
    void resize(size_t size, size_t offset = 0)
    {
        _size = size;
        reset(offset);
    }

    inline void discard() { _pos = _size; }
    inline const uint8_t *buffer() const { return _buf; }
    inline const uint8_t *ptr() const { return &(_buf[_pos]); }
    inline const void *ptr_void() const { return &(_buf[_pos]); }

    inline void skip(size_t size)
    {
        reset(_pos + size);
    }
    inline void trim(size_t pos, size_t size)
    {
        init(_buf + pos, size);
    }
    inline void trim(size_t pos)
    {
        trim(pos, _size - pos);
    }
    inline void trim()
    {
        trim(_pos);
    }

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
        if (available() < 1) {
            return nullptr;
        }
        const char *s = reinterpret_cast<const char *>(ptr());
        size_t len = strnlen(s, max_size);
        if (len > max_size)
            return nullptr;
        //check ASCII
        const uint8_t *c = reinterpret_cast<const uint8_t *>(s);
        if (len == max_size && max_size == available()) {
            // string is NOT null-terminated
            // not the best way of doing the fix though
            size_t p = _pos + len;
            if (p <= _size)
                const_cast<uint8_t *>(_buf)[p] = 0;
            else
                return nullptr;
        }
        while (*c) {
            const uint8_t &v = *c++;
            if (v == '\n')
                continue;
            if (v == '\r')
                continue;
            if (v == '\t')
                continue;
            if (v < 32 || v >= 127)
                return nullptr;
        }
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
        data = static_cast<_Tout>(src);
    }
}

template<typename _T>
void XbusStreamReader::read(_T &data)
{
    if ((_pos + sizeof(_T)) > _size) {
        _pos = _size;
        data = {};
        return;
    }

    switch (sizeof(_T)) {
    case 1:
        data = static_cast<_T>(_buf[_pos]);
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
        // support struct with fixed size
        // i.e. stream >> struct{uint32_t a; uint16_t b;};
        memcpy(&data, &_buf[_pos], sizeof(_T));
        break;
    }
    _pos += sizeof(_T);
}
