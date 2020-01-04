
#include "MandalaStream.h"

#include <cmath>

using namespace mandala;

size_t Stream::pack_float_s1X(void *buf, const mandala::float_t &v)
{
    int x = static_cast<int>(v);
    *static_cast<int8_t *>(buf) = (x > 127) ? 127 : ((x < -128) ? -128 : static_cast<int8_t>(v));
    return 1;
}
size_t Stream::pack_float_u1X(void *buf, const mandala::float_t &v)
{
    int x = static_cast<int>(v);
    *static_cast<uint8_t *>(buf) = (x > 255) ? 255 : ((x < 0) ? 0 : static_cast<uint8_t>(v));
    return 1;
}

size_t Stream::pack_float_s1(void *buf, const void *value_ptr)
{
    return pack_float_s1X(buf, *static_cast<const mandala::float_t *>(value_ptr));
}

size_t Stream::pack_float_s01(void *buf, const void *value_ptr)
{
    return pack_float_s1X(buf, *static_cast<const mandala::float_t *>(value_ptr) * 10.0f);
}

size_t Stream::pack_float_s001(void *buf, const void *value_ptr)
{
    return pack_float_s1X(buf, *static_cast<const mandala::float_t *>(value_ptr) * 100.0f);
}

size_t Stream::pack_float_s10(void *buf, const void *value_ptr)
{
    return pack_float_s1X(buf, *static_cast<const mandala::float_t *>(value_ptr) / 10.0f);
}

size_t Stream::pack_float_u1(void *buf, const void *value_ptr)
{
    return pack_float_u1X(buf, *static_cast<const mandala::float_t *>(value_ptr));
}

size_t Stream::pack_float_u01(void *buf, const void *value_ptr)
{
    return pack_float_u1X(buf, *static_cast<const mandala::float_t *>(value_ptr) * 10.0f);
}

size_t Stream::pack_float_u001(void *buf, const void *value_ptr)
{
    return pack_float_u1X(buf, *static_cast<const mandala::float_t *>(value_ptr) * 100.0f);
}

size_t Stream::pack_float_u10(void *buf, const void *value_ptr)
{
    return pack_float_u1X(buf, *static_cast<const mandala::float_t *>(value_ptr) / 10.0f);
}

size_t Stream::pack_float_u100(void *buf, const void *value_ptr)
{
    return pack_float_u1X(buf, *static_cast<const mandala::float_t *>(value_ptr) / 100.0f);
}

size_t Stream::pack_float_f2(void *buf, const void *value_ptr)
{ //IEEE 754r
    float f = *static_cast<const mandala::float_t *>(value_ptr);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&f);
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
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest++ = hp & 0xFF;
    *dest++ = (hp >> 8) & 0xFF;
    return 2;
}

size_t Stream::pack_float_f4(void *buf, const void *value_ptr)
{
    const float v = *static_cast<const mandala::float_t *>(value_ptr);
    const uint8_t *src = reinterpret_cast<const uint8_t *>(&v);
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
    return 4;
}

size_t Stream::pack_byte_u1(void *buf, const void *value_ptr)
{
    const uint8_t v = *static_cast<const mandala::byte_t *>(value_ptr);
    const uint8_t *src = static_cast<const uint8_t *>(&v);
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest = *src;
    return 1;
}

size_t Stream::pack_enum(void *buf, const void *value_ptr)
{
    const uint8_t v = *static_cast<const mandala::enum_t *>(value_ptr);
    const uint8_t *src = static_cast<const uint8_t *>(&v);
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest = *src;
    return 1;
}

size_t Stream::pack_uint_u4(void *buf, const void *value_ptr)
{
    const uint32_t v = *static_cast<const mandala::uint_t *>(value_ptr);
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest++ = v & 0xFF;
    *dest++ = (v >> 8) & 0xFF;
    *dest++ = (v >> 16) & 0xFF;
    *dest = (v >> 24) & 0xFF;
    return 4;
}

size_t Stream::pack_uint_u2(void *buf, const void *value_ptr)
{
    uint32_t v = *static_cast<const mandala::uint_t *>(value_ptr);
    if (v > 0xFFFF)
        v = 0xFFFF;
    uint8_t *dest = static_cast<uint8_t *>(buf);
    *dest++ = v & 0xFF;
    *dest++ = (v >> 8) & 0xFF;
    return 2;
}

//=============================================================================

size_t Stream::unpack_float_s1(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const int8_t *>(buf);
    return 1;
}

size_t Stream::unpack_float_s01(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const int8_t *>(buf) / 10.0f;
    return 1;
}

size_t Stream::unpack_float_s001(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const int8_t *>(buf) / 100.0f;
    return 1;
}

size_t Stream::unpack_float_s10(const void *buf, void *value_ptr)
{
    int16_t v = *static_cast<const int8_t *>(buf);
    *static_cast<mandala::float_t *>(value_ptr) = v * 10;
    return 1;
}

size_t Stream::unpack_float_u1(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const uint8_t *>(buf);
    return 1;
}

size_t Stream::unpack_float_u01(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const uint8_t *>(buf) / 10.0f;
    return 1;
}

size_t Stream::unpack_float_u001(const void *buf, void *value_ptr)
{
    *static_cast<mandala::float_t *>(value_ptr) = *static_cast<const uint8_t *>(buf) / 100.0f;
    return 1;
}

size_t Stream::unpack_float_u10(const void *buf, void *value_ptr)
{
    uint16_t v = *static_cast<const uint8_t *>(buf);
    *static_cast<mandala::float_t *>(value_ptr) = v * 10;
    return 1;
}

size_t Stream::unpack_float_u100(const void *buf, void *value_ptr)
{
    uint16_t v = *static_cast<const uint8_t *>(buf);
    *static_cast<mandala::float_t *>(value_ptr) = v * 100;
    return 1;
}

size_t Stream::unpack_float_f2(const void *buf, void *value_ptr)
{
    uint16_t h;
    const uint8_t *src = static_cast<const uint8_t *>(buf);
    h = *src++;
    h |= *src++ >> 8;
    uint16_t hs, he, hm;
    uint32_t xp; //(uint32_t*)value_ptr;
    uint32_t xs, xe, xm;
    int32_t xes;
    int e;
    if ((h & 0x7FFFu) == 0) {                // Signed zero
        xp = static_cast<uint32_t>(h) << 16; // Return the signed zero
    } else {                                 // Not zero
        hs = h & 0x8000u;                    // Pick off sign bit
        he = h & 0x7C00u;                    // Pick off exponent bits
        hm = h & 0x03FFu;                    // Pick off mantissa bits
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
    unpack_float_f4(&xp, value_ptr);
    return 2;
}

size_t Stream::unpack_float_f4(const void *buf, void *value_ptr)
{
    float f;
    const uint8_t *src = static_cast<const uint8_t *>(buf);
    uint8_t *dest = reinterpret_cast<uint8_t *>(&f);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;

    if (isnan(f))
        f = 0;
    *static_cast<mandala::float_t *>(value_ptr) = f;
    return 4;
}

size_t Stream::unpack_enum(const void *buf, void *value_ptr)
{
    *static_cast<mandala::enum_t *>(value_ptr) = *static_cast<const uint8_t *>(buf);
    return 1;
}

size_t Stream::unpack_byte_u1(const void *buf, void *value_ptr)
{
    *static_cast<mandala::byte_t *>(value_ptr) = *static_cast<const uint8_t *>(buf);
    return 1;
}

size_t Stream::unpack_uint_u4(const void *buf, void *value_ptr)
{
    uint32_t v;
    const uint8_t *src = static_cast<const uint8_t *>(buf);
    uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
    *static_cast<mandala::uint_t *>(value_ptr) = v;
    return 4;
}

size_t Stream::unpack_uint_u2(const void *buf, void *value_ptr)
{
    uint16_t v;
    const uint8_t *src = static_cast<const uint8_t *>(buf);
    uint8_t *dest = reinterpret_cast<uint8_t *>(&v);
    *dest++ = *src++;
    *dest = *src;
    *static_cast<mandala::uint_t *>(value_ptr) = v;
    return 2;
}
//=============================================================================
