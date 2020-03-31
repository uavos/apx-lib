#pragma once

#include <common/visibility.h>
#include <sys/types.h>

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {

typedef uint16_t pid_raw_t;

constexpr const size_t size_packet_max = 512;

enum pri_e : uint8_t {
    pri_primary = 0,
    pri_secondary,
    pri_failsafe,

    pri_gcs = 5,      // command from gcs
    pri_response = 6, // response not request
    pri_request = 7,  // request not response
};

// Packet identifier [16 bits]
#pragma pack(1)
union pid_s {
    pid_raw_t _raw;

    struct
    {
        uint16_t uid : 11; // dictionary

        uint8_t pri : 3; // [0,7] sub index, 1=request not response
        uint8_t seq : 2; // sequence counter
    };

    explicit pid_s()
        : _raw(0)
    {}

    explicit pid_s(uint16_t _uid, uint8_t _pri, uint8_t _seq)
        : uid(_uid)
        , pri(_pri)
        , seq(_seq)
    {}

    static constexpr inline uint16_t psize()
    {
        return sizeof(pid_raw_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> _raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << _raw;
    }

    // Disallow copy construction and move assignment
    pid_s(const pid_s &pid) { _raw = pid._raw; }
    pid_s &operator=(const pid_s &pid)
    {
        _raw = pid._raw;
        return *this;
    }

    pid_s(pid_s &&) = delete;
    pid_s &operator=(pid_s &&) = delete;
};
static_assert(sizeof(pid_s) == 2, "pid_s size error");
#pragma pack()

} // namespace xbus
