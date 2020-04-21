#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

static constexpr const size_t enc_slots_size{128};

#pragma pack(1)
typedef struct
{
    mandala::type_id_e type : 4; // raw value format
    uint8_t _rsv : 3;            // reserved
    bool upd : 1;                // raw value updated
    mandala::raw_t value;        // raw value
    mandala::raw_t buf;          // packed value
} enc_data_s;

typedef struct
{
    field_s fields[enc_slots_size];
    uint8_t crc_xor;
    enc_data_s data[enc_slots_size];
} enc_slot_s;
static_assert(sizeof(enc_slot_s) == 12 * enc_slots_size + 1, "size error");
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
    xbus::telemetry::enc_slot_s _slots;

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
