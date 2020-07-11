#pragma once

#include "TelemetryFormat.h"
#include <Mandala/MandalaMetaBase.h>
#include <Xbus/XbusStreamWriter.h>

#include <cmath>

namespace xbus {
namespace telemetry {

// type cast and limits

template<typename T>
inline const T &limit(const T &v, const T &vmin, const T &vmax)
{
    return v < vmin ? vmin : (v > vmax ? vmax : v);
}

template<typename T>
struct rtype
{};

template<typename Tin>
int8_t cast_value(rtype<int8_t>, const Tin &v)
{
    return (int8_t)(limit<int32_t>((int32_t) v, -127, 128));
}

template<typename Tin>
uint8_t cast_value(rtype<uint8_t>, const Tin &v)
{
    return (uint8_t)(limit<int32_t>((int32_t) v, 0, 255));
}

template<typename Tin>
uint16_t cast_value(rtype<uint16_t>, const Tin &v)
{
    return (uint16_t)(limit<int32_t>((int32_t) v, 0, 65535));
}

template<typename Tin>
uint32_t cast_value(rtype<uint32_t>, const Tin &v)
{
    return (uint32_t)(v < 0 ? 0 : v);
}

template<typename Tin>
float cast_value(rtype<float>, const Tin &v)
{
    float ret = v;
    return std::isnan(ret) ? 0 : ret;
}

template<typename Tin>
int32_t cast_value(rtype<int32_t>, const Tin &v)
{
    return (int32_t) v;
}

template<typename T, typename Tin>
T cast_value(const Tin &v)
{
    return cast_value(rtype<T>{}, v);
}

// raw value getter

template<typename T>
T raw_value(const void *src, mandala::type_id_e type)
{
    switch (type) {
    default:
        return *static_cast<const mandala::raw_t *>(src);
    case mandala::type_real:
        return cast_value<T>(*static_cast<const mandala::real_t *>(src));
    case mandala::type_dword:
        return cast_value<T>(*static_cast<const mandala::dword_t *>(src));
    case mandala::type_word:
        return cast_value<T>(*static_cast<const mandala::word_t *>(src));
    case mandala::type_byte:
        return cast_value<T>(*static_cast<const mandala::byte_t *>(src));
    case mandala::type_option:
        return cast_value<T>(*static_cast<const mandala::option_t *>(src));
    }
}

// packing helpers

template<typename T>
size_t pack_value(const T &v, void *dest)
{
    T &v0 = *static_cast<T *>(dest);
    if (v == v0)
        return 0;
    v0 = v;
    return sizeof(T);
}

static uint16_t float_to_f16(const float &v)
{
    //IEEE 754r
    if (v == 0.f)
        return 0;
    uint32_t x = std::isnan(v) ? 0 : *static_cast<const uint32_t *>(static_cast<const void *>(&v));
    uint32_t xs, xe, xm;
    uint16_t hs, he, hm;
    uint16_t hp; // = static_cast<uint16_t *>(buf);
    int16_t hes;
    if ((x & 0x7FFFFFFFu) == 0) { // Signed zero
        hp = x >> 16;             // Return the signed zero
    } else {                      // Not zero
        xs = x & 0x80000000u;     // Pick off sign bit
        xe = x & 0x7F800000u;     // Pick off exponent bits
        xm = x & 0x007FFFFFu;     // Pick off mantissa bits
        if (xe == 0) {            // Denormal will underflow, return a signed zero
            hp = xs >> 16;
        } else if (xe == 0x7F800000u) {    // Inf or NaN (all the exponent bits are set)
            if (xm == 0) {                 // If mantissa is zero ...
                hp = (xs >> 16) | 0x7C00u; // Signed Inf
            } else {
                hp = 0xFE00u; // NaN, only 1st mantissa bit set
            }
        } else {                           // Normalized number
            hs = (xs >> 16);               // Sign bit
            hes = ((xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
            if (hes >= 0x1F) {             // Overflow
                hp = (xs >> 16) | 0x7C00u; // Signed Inf
            } else if (hes <= 0) {         // Underflow
                if ((14 - hes) > 24) {     // Mantissa shifted all the way off & no rounding possibility
                    hm = 0u;               // Set mantissa to zero
                } else {
                    xm |= 0x00800000u;                            // Add the hidden leading bit
                    hm = static_cast<uint16_t>(xm >> (14 - hes)); // Mantissa
                    if ((xm >> (13 - hes)) & 0x00000001u)         // Check for rounding
                        hm += 1u;                                 // Round, might overflow into exp bit, but this is OK
                }
                hp = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            } else {
                he = static_cast<uint16_t>(hes << 10); // Exponent
                hm = static_cast<uint16_t>(xm >> 13);  // Mantissa
                if (xm & 0x00001000u)                  // Check for rounding
                    hp = (hs | he | hm) + 1u;          // Round, might overflow to inf, this is OK
                else
                    hp = (hs | he | hm); // No rounding
            }
        }
    }
    return hp;
}
static int16_t float_to_rad(const float &v)
{
    const float span = (float) M_PI;
    const float dspan = span * 2.f;
    const float a = v - std::floor(v / dspan + 0.5f) * dspan;
    return (int16_t)(a * (32768.f / (float) M_PI));
}
static int16_t float_to_rad2(const float &v)
{
    const float span = (float) M_PI / 2.f;
    const float dspan = span * 2.f;
    const float a = v - std::floor(v / dspan + 0.5f) * dspan;
    return (int16_t)(a * (32768.f / (float) M_PI));
}

// pack method

size_t pack_value(const void *src, void *dest, mandala::type_id_e type, fmt_e fmt)
{
    switch (fmt) {
    default:
        return 0;
    case fmt_real: {
        const mandala::real_t &v = raw_value<mandala::real_t>(src, type);
        return pack_value(v, dest);
    }
    case fmt_dword: {
        const mandala::dword_t &v = raw_value<mandala::dword_t>(src, type);
        return pack_value(v, dest);
    }
    case fmt_word: {
        const mandala::word_t &v = raw_value<mandala::word_t>(src, type);
        return pack_value(v, dest);
    }
    case fmt_opt:
    case fmt_byte: {
        const mandala::byte_t &v = raw_value<mandala::byte_t>(src, type);
        return pack_value(v, dest);
    }

    case fmt_f16: {
        const mandala::real_t &v = raw_value<mandala::real_t>(src, type);
        return pack_value(float_to_f16(v), dest);
    }

    case fmt_sbyte: {
        const int8_t &v = raw_value<int8_t>(src, type);
        return pack_value(v, dest);
    }
    case fmt_sbyte_10: {
        const int8_t &v = cast_value<int8_t>(raw_value<int32_t>(src, type) / 10);
        return pack_value(v, dest);
    }
    case fmt_sbyte_01: {
        const int8_t &v = cast_value<int8_t>(raw_value<float>(src, type) * 10.f);
        return pack_value(v, dest);
    }
    case fmt_sbyte_001: {
        const int8_t &v = cast_value<int8_t>(raw_value<float>(src, type) * 100.f);
        return pack_value(v, dest);
    }
    case fmt_byte_10: {
        const uint8_t &v = cast_value<uint8_t>(raw_value<int32_t>(src, type) / 10);
        return pack_value(v, dest);
    }
    case fmt_byte_01: {
        const uint8_t &v = cast_value<uint8_t>(raw_value<float>(src, type) * 10.f);
        return pack_value(v, dest);
    }
    case fmt_byte_001: {
        const uint8_t &v = cast_value<uint8_t>(raw_value<float>(src, type) * 100.f);
        return pack_value(v, dest);
    }

    case fmt_rad: {
        const mandala::real_t &v = raw_value<mandala::real_t>(src, type);
        return pack_value(float_to_rad(v), dest);
    }
    case fmt_rad2: {
        const mandala::real_t &v = raw_value<mandala::real_t>(src, type);
        return pack_value(float_to_rad2(v), dest);
    }

    case fmt_byte_u: {
        const uint8_t &v = cast_value<uint8_t>(raw_value<float>(src, type) * 255.f);
        return pack_value(v, dest);
    }
    case fmt_sbyte_u: {
        const int8_t &v = cast_value<int8_t>(raw_value<float>(src, type) * 127.f);
        return pack_value(v, dest);
    }
    }
}

} // namespace telemetry
} // namespace xbus
