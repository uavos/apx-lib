#pragma once
#include <inttypes.h>
#include <sys/types.h>

#include <array>
#include <string>

#include "endian.h"

class XbusStreamWriter
{
public:
    explicit XbusStreamWriter(uint8_t *p)
        : msg(p)
        , pos(0)

    {}
    explicit XbusStreamWriter(uint8_t &p)
        : msg(&p)
        , pos(0)

    {}

    inline void reset(uint16_t new_pos = 0) { pos = new_pos; }
    inline uint16_t position() const { return pos; }
    inline uint8_t *buf() const { return msg; }
    inline uint8_t *data() const { return &(msg[pos]); }

    //template<typename _T>
    //void write(const _T data);

    template<typename _T, typename _Tin>
    void write(const _Tin data);

    template<class _T, size_t _Size>
    void write(const std::array<_T, _Size> &data);

    template<typename _T>
    inline void operator<<(const _T data)
    {
        write<_T>(data);
    }

    template<class _T, size_t _Size>
    inline void operator<<(const std::array<_T, _Size> &data)
    {
        write<_T>(data);
    }

private:
    uint8_t *msg;
    uint16_t pos;

    template<typename _T, typename _Tin>
    inline void set_data(_T &buf, _Tin data);
};

// implementation
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

template<typename _T, typename _Tin>
void XbusStreamWriter::set_data(_T &buf, _Tin data)
{
    if (std::is_floating_point<_Tin>::value) {
        buf = *static_cast<const _T *>(static_cast<const void *>(&data));
    } else {
        buf = data;
    }

    // htoleXX functions may be empty macros,
    // switch will be optimized-out
    switch (sizeof(_T)) {
    case 2:
        buf = htole16(buf);
        break;

    case 4:
        buf = htole32(buf);
        break;

    case 8:
        buf = htole64(buf);
        break;

    default:
        assert(false);
    }

    memcpy(&msg[pos], &buf, sizeof(buf));
}
#pragma GCC diagnostic pop

template<typename _T, typename _Tin>
void XbusStreamWriter::write(const _Tin data)
{
    switch (sizeof(_T)) {
    case 1:
        msg[pos] = static_cast<_T>(data);
        break;

    case 2:
        uint16_t data_le16;
        set_data(data_le16, static_cast<_T>(data));
        break;

    case 4:
        uint32_t data_le32;
        set_data(data_le32, static_cast<_T>(data));
        break;

    case 8:
        uint64_t data_le64;
        set_data(data_le64, static_cast<_T>(data));
        break;

    default:
        assert(false);
    }
    pos += sizeof(_T);
}

template<class _T, size_t _Size>
void XbusStreamWriter::write(const std::array<_T, _Size> &data)
{
    for (auto &v : data) {
        *this << v;
    }
}
