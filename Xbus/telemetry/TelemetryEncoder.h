#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

static constexpr const size_t enc_slots_size{128};

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
    field_s fields[enc_slots_size];
    uint8_t crc_cobs;
    enc_flags_s flags[enc_slots_size];
    mandala::raw_t value[enc_slots_size];  // raw value
    mandala::raw_t packed[enc_slots_size]; // packed value
} enc_slots_s;
//static_assert(sizeof(enc_slots_s) == 12 * enc_slots_size + 1, "size error");
#pragma pack()

} // namespace telemetry
} // namespace xbus

class TelemetryEncoder
{
public:
    explicit TelemetryEncoder();

    void add(const xbus::telemetry::field_s &field);

    bool encode(XbusStreamWriter &stream, uint8_t seq, xbus::telemetry::rate_e rate);

    bool update(const xbus::pid_s &pid, const mandala::spec_s &spec, XbusStreamReader &stream);

private:
    xbus::telemetry::enc_slots_s _slots;
    uint16_t _slots_cnt{0};

    xbus::telemetry::hash_s _hash;

    void _set_data(size_t n, const mandala::spec_s &spec, XbusStreamReader &stream);
    void _update_feeds();

    // fmt feed COBS encoder
    uint8_t _get_feed_fmt();
    uint16_t _fmt_pos;
    uint16_t _fmt_size;

    uint8_t _cobs_code;

    void encode_values(XbusStreamWriter &stream, uint8_t seq);
};
