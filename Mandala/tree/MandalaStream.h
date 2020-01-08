#pragma once

#include <cmath>

#include "MandalaMetaBase.h"

namespace mandala {

class stream
{
public:
    //pack
    template<mandala::sfmt_id_t _sfmt, typename _DataType>
    static size_t pack(void *buf, const _DataType &value)
    {
        if (std::is_floating_point<_DataType>::value) {
            if (_sfmt == sfmt_f4) {
                return pack_raw_int(buf, float_to_f32(value));
            } else if (_sfmt == sfmt_f2) {
                return pack_raw_int(buf, float_to_f16(value));
            } else if (_sfmt == sfmt_f1) {
                return pack_raw_int(buf, float_to_u8(value));
            } else if (_sfmt == sfmt_f1div10) {
                return pack_raw_int(buf, float_to_u8(value / 10.0f));
            } else if (_sfmt == sfmt_f1mul10) {
                return pack_raw_int(buf, float_to_u8(value * 10.0f));
            } else if (_sfmt == sfmt_f1mul100) {
                return pack_raw_int(buf, float_to_u8(value * 100.0f));
            } else if (_sfmt == sfmt_f1s) {
                return pack_raw_int(buf, float_to_s8(value));
            } else if (_sfmt == sfmt_f1smul100) {
                return pack_raw_int(buf, float_to_s8(value * 100.0f));
            }
        } else if (_sfmt == sfmt_u4) {
            static_assert(sizeof(_DataType) == 4, "sfmt");
            uint32_t v = value;
            return pack_raw_int(buf, v);
        } else if (_sfmt == sfmt_u2) {
            static_assert(sizeof(_DataType) >= 2, "sfmt");
            uint16_t v = value > 0xFFFF ? 0xFFFF : value;
            return pack_raw_int(buf, v);
        } else if (_sfmt == sfmt_u1) {
            static_assert(sizeof(_DataType) >= 1, "sfmt");
            uint8_t v = value > 0xFF ? 0xFF : value;
            return pack_raw_int(buf, v);
        }
        return 0;
    }

    //unpack
    template<mandala::sfmt_id_t _sfmt, typename _DataType>
    static size_t unpack(const void *buf, _DataType &value)
    {
        if (std::is_floating_point<_DataType>::value) {
            if (_sfmt == sfmt_f4) {
                uint32_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = float_from_f32(v);
                return 4;
            } else if (_sfmt == sfmt_f2) {
                uint16_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = float_from_f16(v);
                return 2;
            } else if (_sfmt == sfmt_f1) {
                uint8_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = v;
                return 1;
            } else if (_sfmt == sfmt_f1div10) {
                uint8_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = v * 10.0f;
                return 1;
            } else if (_sfmt == sfmt_f1mul10) {
                uint8_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = v / 10.0f;
                return 1;
            } else if (_sfmt == sfmt_f1mul100) {
                uint8_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = v / 100.0f;
                return 1;
            } else if (_sfmt == sfmt_f1s) {
                int8_t v;
                if (!unpack_raw_int(buf, v))
                    return 0;
                value = v;
                return 1;
            }
        } else if (_sfmt == sfmt_u4) {
            static_assert(sizeof(_DataType) == 4, "sfmt");
            uint32_t v;
            if (!unpack_raw_int(buf, v))
                return 0;
            value = static_cast<_DataType>(v);
            return 4;
        } else if (_sfmt == sfmt_u2) {
            static_assert(sizeof(_DataType) >= 2, "sfmt");
            uint16_t v;
            if (!unpack_raw_int(buf, v))
                return 0;
            value = static_cast<_DataType>(v);
            return 2;
        } else if (_sfmt == sfmt_u1) {
            static_assert(sizeof(_DataType) >= 2, "sfmt");
            uint8_t v;
            if (!unpack_raw_int(buf, v))
                return 0;
            value = static_cast<_DataType>(v);
            return 1;
        }
        return 0;
    }

private:
    template<typename _Tin>
    static size_t pack_raw_int(void *buf, const _Tin &value)
    {
        uint8_t *dest = static_cast<uint8_t *>(buf);
        if (sizeof(_Tin) == 4) {
            uint32_t v = static_cast<uint32_t>(value);
            *dest++ = v & 0xFF;
            *dest++ = (v >> 8) & 0xFF;
            *dest++ = (v >> 16) & 0xFF;
            *dest = (v >> 24) & 0xFF;
        } else if (sizeof(_Tin) == 2) {
            uint16_t v = static_cast<uint16_t>(value);
            *dest++ = v & 0xFF;
            *dest = v >> 8;
        } else if (sizeof(_Tin) == 1) {
            uint8_t v = static_cast<uint8_t>(value);
            *dest = v;
        } else
            return 0;
        return sizeof(_Tin);
    }

    template<typename _Tout>
    static size_t unpack_raw_int(const void *buf, _Tout &value)
    {
        const uint8_t *src = static_cast<const uint8_t *>(buf);
        if (sizeof(_Tout) == 4) {
            uint32_t v;
            v = *src++;
            v |= static_cast<uint32_t>(*src++) << 8;
            v |= static_cast<uint32_t>(*src++) << 16;
            v |= static_cast<uint32_t>(*src) << 24;
            value = static_cast<_Tout>(v);
        } else if (sizeof(_Tout) == 2) {
            uint16_t v;
            v = *src++;
            v |= static_cast<uint32_t>(*src) << 8;
            value = static_cast<_Tout>(v);
        } else if (sizeof(_Tout) == 1) {
            uint8_t v;
            v = *src;
            value = static_cast<_Tout>(v);
        } else
            return 0;
        return sizeof(_Tout);
    }

    static uint16_t float_to_f16(const float &v)
    {
        //IEEE 754r
        uint32_t x = float_to_f32(v);
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

    static float float_from_f16(const uint16_t &v)
    {
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
                } while ((hm & 0x0400u) == 0);                              // Shift until leading bit overflows into exponent bit
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
        return float_from_f32(xp);
    }
    static float float_from_f32(const uint32_t &f)
    {
        float v;
        /*const uint8_t *src = reinterpret_cast<const uint8_t *>(&f);
        uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;*/
        v = *static_cast<const float *>(static_cast<const void *>(&f));
        if (isnan(v))
            return 0;
        return v;
    }
    static uint32_t float_to_f32(const float &f)
    {
        if (isnan(f))
            return 0;
        uint32_t v;
        /*const uint8_t *src = reinterpret_cast<const uint8_t *>(&f);
        uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;*/
        v = *static_cast<const uint32_t *>(static_cast<const void *>(&f));
        return v;
    }
    static uint8_t float_to_u8(const float &f)
    {
        if (isnan(f))
            return 0;
        int32_t v32 = static_cast<int32_t>(f);
        uint8_t v = v32 > 255 ? 255 : v32 < 0 ? 0 : static_cast<uint8_t>(f);
        return v;
    }
    static int8_t float_to_s8(const float &f)
    {
        if (isnan(f))
            return 0;
        int32_t v32 = static_cast<int32_t>(f);
        int8_t v = v32 > 127 ? 127 : v32 < -128 ? -128 : static_cast<int8_t>(f);
        return v;
    }
};

}; // namespace mandala
