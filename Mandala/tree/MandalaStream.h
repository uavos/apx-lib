#pragma once

#include "MandalaValue.h"

#include <cmath>

namespace mandala {

class Stream
{
public:
    //pack
    static size_t pack_float_s1X(void *buf, const mandala::float_t &v);
    static size_t pack_float_u1X(void *buf, const mandala::float_t &v);

    static size_t pack_float_s1(void *buf, const void *value_ptr);
    static size_t pack_float_s01(void *buf, const void *value_ptr);
    static size_t pack_float_s001(void *buf, const void *value_ptr);
    static size_t pack_float_s10(void *buf, const void *value_ptr);

    static size_t pack_float_u1(void *buf, const void *value_ptr);
    static size_t pack_float_u01(void *buf, const void *value_ptr);
    static size_t pack_float_u001(void *buf, const void *value_ptr);
    static size_t pack_float_u10(void *buf, const void *value_ptr);
    static size_t pack_float_u100(void *buf, const void *value_ptr);

    static size_t pack_float_f2(void *buf, const void *value_ptr);
    static size_t pack_float_f4(void *buf, const void *value_ptr);

    static size_t pack_enum(void *buf, const void *value_ptr);
    static size_t pack_byte_u1(void *buf, const void *value_ptr);
    static size_t pack_uint_u4(void *buf, const void *value_ptr);
    static size_t pack_uint_u2(void *buf, const void *value_ptr);

    //unpack
    static size_t unpack_float_s1(const void *buf, void *value_ptr);
    static size_t unpack_float_s01(const void *buf, void *value_ptr);
    static size_t unpack_float_s001(const void *buf, void *value_ptr);
    static size_t unpack_float_s10(const void *buf, void *value_ptr);

    static size_t unpack_float_u1(const void *buf, void *value_ptr);
    static size_t unpack_float_u01(const void *buf, void *value_ptr);
    static size_t unpack_float_u001(const void *buf, void *value_ptr);
    static size_t unpack_float_u10(const void *buf, void *value_ptr);
    static size_t unpack_float_u100(const void *buf, void *value_ptr);

    static size_t unpack_float_f2(const void *buf, void *value_ptr);
    static size_t unpack_float_f4(const void *buf, void *value_ptr);

    static size_t unpack_enum(const void *buf, void *value_ptr);
    static size_t unpack_byte_u1(const void *buf, void *value_ptr);
    static size_t unpack_uint_u4(const void *buf, void *value_ptr);
    static size_t unpack_uint_u2(const void *buf, void *value_ptr);
};

template<typename _T, size_t _Size>
class stream
{
public:
    static size_t pack(void *buf, const void *value_ptr)
    {
        if (std::is_floating_point<_T>::value) {
        }

        uint32_t v = *static_cast<const _T *>(value_ptr);
        uint8_t *dest = static_cast<uint8_t *>(buf);
        if (_Size == 4) {
            *dest++ = v & 0xFF;
            *dest++ = (v >> 8) & 0xFF;
            *dest++ = (v >> 16) & 0xFF;
            *dest = (v >> 24) & 0xFF;
        } else if (_Size == 2) {
            if (v > 0xFFFF)
                v = 0xFFFF;
            *dest++ = v & 0xFF;
            *dest = (v >> 8) & 0xFF;
        } else if (_Size == 1) {
            if (v > 0xFF)
                v = 0xFF;
            *dest = v & 0xFF;
        } else
            return 0;
        return _Size;
    }

    static size_t unpack(const void *buf, void *value_ptr)
    {
        uint32_t v;
        const uint8_t *src = static_cast<const uint8_t *>(buf);
        uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
        if (_Size == 4) {
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest = *src;
        } else if (_Size == 2) {
            *dest++ = *src++;
            *dest = *src;
        } else if (_Size == 1) {
            *dest = *src;
        } else
            return 0;
        *static_cast<_T *>(value_ptr) = v;
        return _Size;
    }

    static uint16_t float_to_f16(const float &v)
    {
        //IEEE 754r
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&v);
        uint32_t x = static_cast<uint32_t>(ptr[0]) | static_cast<uint32_t>(ptr[1]) << 8 | static_cast<uint32_t>(ptr[2]) << 16 | static_cast<uint32_t>(ptr[3]) << 24;
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
        return float_copy(&xp);
    }
    static float float_copy(const void *buf)
    {
        float v;
        const uint8_t *src = reinterpret_cast<const uint8_t *>(&buf);
        uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;

        if (isnan(v))
            return 0;
        return v;
    }
};

}; // namespace mandala
