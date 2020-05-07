#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

#pragma pack(1)
typedef struct
{
    mandala::type_id_e type : 4; // value format
    uint8_t _rsv : 3;            // reserved
    bool upd : 1;                // value updated
} dec_flags_s;
#pragma pack()

typedef struct
{
    field_s fields[slots_size];
    uint8_t crc_cobs_dummy;
    dec_flags_s flags[slots_size];
    mandala::raw_t value[slots_size]; // value
} dec_slots_s;
//static_assert(sizeof(dec_slot_s) == 8 * slots_size + 1, "size error");

} // namespace telemetry
} // namespace xbus

class TelemetryDecoder
{
public:
    explicit TelemetryDecoder();

    bool decode(const xbus::pid_s &pid, XbusStreamReader &stream);

    bool decode_format(uint8_t part, uint8_t parts, XbusStreamReader &stream);

    float rate();

    bool valid() { return _valid; }
    void reset(bool reset_hash = true);

    uint32_t timestamp_10ms() { return _ts; }

    xbus::telemetry::dec_slots_s &dec_slots() { return _slots; }
    uint16_t slots_cnt() { return _slots_cnt; }
    uint16_t fmt_cnt() { return _fmt_pos; }
    uint8_t seq() { return _seq; }

protected:
    xbus::telemetry::dec_slots_s _slots;
    uint16_t _slots_cnt{0};

    uint8_t _seq{0};
    uint32_t _ts{0};
    uint32_t _dts{0};

    bool _valid{false};

    xbus::telemetry::hash_s _hash;
    uint8_t _hash_valid{0};
    inline bool _is_hash_valid() { return _hash_valid == 4; }
    uint32_t get_hash(size_t sz);
    bool check_hash(size_t sz);

    // fmt feed decoder
    void _set_feed_fmt(uint8_t v);
    uint16_t _fmt_pos{0};
    uint8_t _cobs_code{0};
    uint8_t _cobs_copy{0};

    bool decode_values(XbusStreamReader &stream, uint8_t seq);
};
