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

#include "TelemetryFormat.h"
#include <MandalaMetaBase.h>

#include <math.h>

namespace xbus {
namespace telemetry {

using namespace mandala;

// raw value getter

template<typename Tsrc, typename Tdest = Tsrc>
size_t unpack_value(const void *src, void *dest, size_t size)
{
    if (size < sizeof(Tsrc))
        return 0;
    *static_cast<Tdest *>(dest) = *static_cast<const Tsrc *>(src);
    return sizeof(Tsrc);
}

template<typename T>
size_t unpack_value(const void *src, void *dest, mandala::type_id_e *type, size_t size, const mandala::real_t &scale)
{
    *type = mandala::type_real;
    if (!unpack_value<T, mandala::real_t>(src, dest, size))
        return 0;
    *static_cast<mandala::real_t *>(dest) = *static_cast<mandala::real_t *>(dest) * scale;
    return sizeof(T);
}

static float float_from_f16(const uint16_t v)
{
    if (v == 0)
        return 0.f;
    uint16_t hs, he, hm;
    uint32_t xp;
    uint32_t xs, xe, xm;
    int32_t xes;
    int e;
    if ((v & 0x7FFFu) == 0) {                // Signed zero
        xp = static_cast<uint32_t>(v) << 16; // Return the signed zero
    } else {                                 // Not zero
        hs = v & 0x8000u;                    // Pick off sign bit
        he = v & 0x7C00u;                    // Pick off exponent bits
        hm = v & 0x03FFu;                    // Pick off mantissa bits
        if (he == 0) {                       // Denormal will convert to normalized
            e = -1;                          // The following loop figures out how much extra to adjust the exponent
            do {
                e++;
                hm <<= 1;
            } while ((hm & 0x0400u) == 0); // Shift until leading bit overflows into exponent bit
            xs = static_cast<uint32_t>(hs) << 16;                       // Sign bit
            xes = (he >> 10) - 15 + 127 - e;                            // Exponent unbias the halfp, then bias the single
            xe = static_cast<uint32_t>(xes) << 23;                      // Exponent
            xm = (hm & 0x03FFu) << 13;                                  // Mantissa
            xp = (xs | xe | xm);                                        // Combine sign bit, exponent bits, and mantissa bits
        } else if (he == 0x7C00u) {                                     // Inf or NaN (all the exponent bits are set)
            if (hm == 0) {                                              // If mantissa is zero ...
                xp = (static_cast<uint32_t>(hs) << 16) | (0x7F800000u); // Signed Inf
            } else {
                xp = 0xFFC00000u; // NaN, only 1st mantissa bit set
            }
        } else {                                   // Normalized number
            xs = static_cast<uint32_t>(hs) << 16;  // Sign bit
            xes = (he >> 10) - 15 + 127;           // Exponent unbias the halfp, then bias the single
            xe = static_cast<uint32_t>(xes) << 23; // Exponent
            xm = static_cast<uint32_t>(hm) << 13;  // Mantissa
            xp = (xs | xe | xm);                   // Combine sign bit, exponent bits, and mantissa bits
        }
    }
    const float &f = *static_cast<const mandala::real_t *>(static_cast<const void *>(&xp));
    return isnan(f) ? 0.f : f;
}

template<typename T>
static float float_from_angle(const T v, float span)
{
    return v / ((float) std::numeric_limits<T>::max() / span);
}

template<typename T>
static float float_from_rad(const T v)
{
    return float_from_angle<T>(v, (float) M_PI);
}
template<typename T>
static float float_from_rad2(const T v)
{
    return float_from_angle<T>(v, (float) M_PI / 2.f);
}

// unpack method

static size_t unpack_value(const void *src, void *dest, mandala::type_id_e *type, fmt_e fmt, size_t size)
{
    switch (fmt) {
    case fmt_none:
    case fmt_bit:
        return 0;

    case fmt_f32:
        *type = mandala::type_real;
        return unpack_value<mandala::real_t>(src, dest, size);
    case fmt_u32:
    case fmt_a32:
        *type = mandala::type_dword;
        return unpack_value<mandala::dword_t>(src, dest, size);
    case fmt_u16:
        *type = mandala::type_word;
        return unpack_value<mandala::word_t>(src, dest, size);
    case fmt_u4:
    case fmt_u8:
        *type = mandala::type_byte;
        return unpack_value<mandala::byte_t>(src, dest, size);

    case fmt_f16:
        *type = mandala::type_real;
        if (!unpack_value<mandala::word_t>(src, dest, size))
            break;
        *static_cast<mandala::real_t *>(dest) = float_from_f16(*static_cast<uint16_t *>(dest));
        return sizeof(mandala::word_t);

    case fmt_s8:
        *type = mandala::type_real;
        return unpack_value<int8_t, mandala::real_t>(src, dest, size);
    case fmt_s8_10:
        *type = mandala::type_real;
        if (!unpack_value<int8_t, mandala::real_t>(src, dest, size))
            break;
        *static_cast<mandala::real_t *>(dest) = *static_cast<mandala::real_t *>(dest) * 10.f;
        return sizeof(int8_t);
    case fmt_s8_01:
        return unpack_value<int8_t>(src, dest, type, size, 1.f / 10.f);
    case fmt_s8_001:
        return unpack_value<int8_t>(src, dest, type, size, 1.f / 100.f);

    case fmt_u8_10:
        *type = mandala::type_dword;
        if (!unpack_value<uint8_t, mandala::dword_t>(src, dest, size))
            break;
        *static_cast<mandala::dword_t *>(dest) = *static_cast<mandala::dword_t *>(dest) * 10;
        return sizeof(uint8_t);

    case fmt_u8_01:
        return unpack_value<uint8_t>(src, dest, type, size, 1.f / 10.f);
    case fmt_u8_001:
        return unpack_value<uint8_t>(src, dest, type, size, 1.f / 100.f);

    case fmt_s16_rad:
        *type = mandala::type_real;
        if (!unpack_value<int16_t>(src, dest, size))
            break;
        *static_cast<mandala::real_t *>(dest) = float_from_rad(*static_cast<int16_t *>(dest));
        return sizeof(int16_t);
    case fmt_s16_rad2:
        *type = mandala::type_real;
        if (!unpack_value<int16_t>(src, dest, size))
            break;
        *static_cast<mandala::real_t *>(dest) = float_from_rad2(*static_cast<int16_t *>(dest));
        return sizeof(int16_t);
    case fmt_s8_rad:
        *type = mandala::type_real;
        if (!unpack_value<int8_t>(src, dest, size))
            break;
        *static_cast<mandala::real_t *>(dest) = float_from_rad(*static_cast<int8_t *>(dest));
        return sizeof(int8_t);

    case fmt_u8_u:
        return unpack_value<uint8_t>(src, dest, type, size, 1.f / 255.f);
    case fmt_s8_u:
        return unpack_value<int8_t>(src, dest, type, size, 1.f / 127.f);

    case fmt_u16_10:
        *type = mandala::type_dword;
        if (!unpack_value<uint16_t, mandala::dword_t>(src, dest, size))
            break;
        *static_cast<mandala::dword_t *>(dest) = *static_cast<mandala::dword_t *>(dest) * 10;
        return sizeof(uint16_t);
    }
    return 0;
}

} // namespace telemetry
} // namespace xbus
