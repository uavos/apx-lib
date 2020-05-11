#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

typedef void (*enc_codec_f)();

#pragma pack(1)
typedef struct
{
    mandala::type_id_e type : 4; // raw value format
    uint8_t _rsv : 3;            // reserved
    bool upd : 1;                // raw value updated
} enc_flags_s;
#pragma pack()

typedef struct
{
    field_s fields[slots_size];
    uint8_t crc_cobs;
    enc_flags_s flags[slots_size];
    mandala::raw_t value[slots_size];  // raw value
    mandala::raw_t packed[slots_size]; // packed value
} enc_slots_s;
//static_assert(sizeof(enc_slots_s) == 12 * slots_size + 1, "size error");
#pragma pack()

} // namespace telemetry
} // namespace xbus

class TelemetryEncoder
{
public:
    explicit TelemetryEncoder();

    bool add(const xbus::telemetry::field_s &field);
    void clear();

    bool encode(XbusStreamWriter &stream, uint32_t seq, xbus::telemetry::dt_e dt);
    void encode_format(XbusStreamWriter &stream, uint8_t part);

    bool update(const xbus::pid_s &pid, const mandala::spec_s &spec, XbusStreamReader &stream);

    xbus::telemetry::enc_slots_s &enc_slots() { return _slots; }
    uint16_t slots_cnt() { return _slots_cnt; }

    void reset_feeds();

private:
    xbus::telemetry::enc_slots_s _slots;
    uint16_t _slots_cnt{0};
    uint16_t _slots_upd_cnt{0}; // re-scheduled slots top limit (before bitfields)

    void _insert(size_t index, const xbus::telemetry::field_s &field);

    xbus::telemetry::hash_s _hash;

    void _set_data(size_t n, const mandala::spec_s &spec, XbusStreamReader &stream);

    void _update_feeds();

    // fmt feed COBS encoder
    uint8_t _get_feed_fmt();
    uint16_t _fmt_pos;
    uint16_t _fmt_size;

    uint8_t _cobs_code;
    uint8_t _cobs_copy;

    void encode_values(XbusStreamWriter &stream, uint32_t seq);
};
