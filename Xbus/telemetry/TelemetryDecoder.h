#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

static constexpr const size_t dec_slots_size{128};

#pragma pack(1)
typedef struct
{
    field_s fields[dec_slots_size];
    uint8_t crc_xor;
    struct
    {
        bool upd : 1;         // value updated
        mandala::raw_t value; // value
    } data[dec_slots_size];
} dec_slot_s;
static_assert(sizeof(dec_slot_s) == 8 * dec_slots_size + 1, "size error");
#pragma pack()

} // namespace telemetry
} // namespace xbus

class TelemetryDecoder
{
public:
    explicit TelemetryDecoder();

    bool decode(const xbus::pid_s &pid, XbusStreamReader &stream);

    uint8_t rate_hz();

protected:
    xbus::telemetry::dec_slot_s _slots;
    uint16_t _slots_cnt{0};

    uint8_t _seq{0};
    uint8_t _rate{0};

    xbus::telemetry::hash_s _hash;
    uint8_t _hash_valid{0};
    inline bool _is_hash_valid() { return _hash_valid == 4; }

    // fmt feed decoder
    void _set_feed_fmt(uint8_t v);
    uint16_t _fmt_pos{0};
    uint8_t _cobs_code{0};
    uint8_t _cobs_copy{0};

    bool decode_values(XbusStreamReader &stream);
};
