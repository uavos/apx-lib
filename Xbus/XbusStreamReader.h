#pragma once
#include <inttypes.h>
#include <sys/types.h>

#include <array>
#include <cstring>

#include "endian.h"

class XbusStreamReader
{
public:
    explicit XbusStreamReader(const void *p, uint16_t size = 0)
        : msg(static_cast<const uint8_t *>(p))
        , pos(0)
        , len(size)

    {}

    inline void reset(uint16_t new_pos = 0) { pos = new_pos; }
    inline uint16_t position() const { return pos; }
    inline uint16_t tail() const { return len > pos ? len - pos : 0; }
    inline const uint8_t *data() const { return &(msg[len > pos ? pos : (len - 1)]); }

    template<typename _T>
    void read(_T &data);

    template<class _T, size_t _Size>
    void read(std::array<_T, _Size> &data);

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

    template<class _T, size_t _Size>
    inline void operator>>(std::array<_T, _Size> &data)
    {
        read(data);
    }

private:
    const uint8_t *msg;
    uint16_t pos;
    uint16_t len;

    template<typename _T, typename _Tout>
    inline void get_data(_T &buf, _Tout &data);
};

// implementation

template<typename _T, typename _Tout>
void XbusStreamReader::get_data(_T &buf, _Tout &data)
{
    memcpy(&buf, &msg[pos], sizeof(_T));

    // message is trimmed - bzero tail
    if (len > 0 && pos + sizeof(_T) > len) {
        union {
            _T d;
            uint8_t u8[sizeof(_T)];
        } bz;

        size_t toclean = (pos + sizeof(_T)) - len;
        size_t start_pos = sizeof(_T) - toclean;

        bz.d = buf;
        memset(&bz.u8[start_pos], 0, toclean);
        buf = bz.d;
    }

    switch (sizeof(_T)) {
    case 2:
        buf = le16toh(buf);
        break;

    case 4:
        buf = le32toh(buf);
        break;

    case 8:
        buf = le64toh(buf);
        break;

    default:
        assert(false);
    }

    if (std::is_floating_point<_Tout>::value) {
        data = *static_cast<_Tout *>(static_cast<void *>(&buf));
    } else {
        data = buf;
    }
}

template<typename _T>
void XbusStreamReader::read(_T &data)
{
    // message is trimmed - fill with zeroes
    if (len > 0 && pos >= len) {
        data = 0;
        pos += sizeof(_T);
        return;
    }

    switch (sizeof(_T)) {
    case 1:
        data = msg[pos];
        break;

    case 2:
        uint16_t data_le16;
        get_data(data_le16, data);
        break;

    case 4:
        uint32_t data_le32;
        get_data(data_le32, data);
        break;

    case 8:
        uint64_t data_le64;
        get_data(data_le64, data);
        break;

    default:
        assert(false);
    }
    pos += sizeof(_T);
}

template<class _T, size_t _Size>
void XbusStreamReader::read(std::array<_T, _Size> &data)
{
    for (auto &v : data) {
        *this >> v;
    }
}
