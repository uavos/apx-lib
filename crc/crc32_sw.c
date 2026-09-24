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
#include "crc32_sw.h"

#include <string.h>

uint32_t stm_crc_slow(uint32_t crc, uint32_t data)
{
    int i;

    crc = crc ^ data;

    for (i = 0; i < 32; i++)
        if (crc & 0x80000000)
            crc = (crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
        else
            crc = (crc << 1);

    return (crc);
}

uint32_t stm_crc_fast(uint32_t crc, uint32_t data)
{
    static const uint32_t crcTable[16] = {// Nibble lookup table for 0x04C11DB7 polynomial
                                          0x00000000,
                                          0x04C11DB7,
                                          0x09823B6E,
                                          0x0D4326D9,
                                          0x130476DC,
                                          0x17C56B6B,
                                          0x1A864DB2,
                                          0x1E475005,
                                          0x2608EDB8,
                                          0x22C9F00F,
                                          0x2F8AD6D6,
                                          0x2B4BCB61,
                                          0x350C9B64,
                                          0x31CD86D3,
                                          0x3C8EA00A,
                                          0x384FBDBD};

    crc = crc ^ data; // Apply all 32-bits

    // Process 32-bits, 4 at a time, or 8 rounds
    crc = (crc << 4) ^ crcTable[crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
    crc = (crc << 4) ^ crcTable[crc >> 28]; // 0x04C11DB7 Polynomial used in STM32
    crc = (crc << 4) ^ crcTable[crc >> 28];
    crc = (crc << 4) ^ crcTable[crc >> 28];
    crc = (crc << 4) ^ crcTable[crc >> 28];
    crc = (crc << 4) ^ crcTable[crc >> 28];
    crc = (crc << 4) ^ crcTable[crc >> 28];
    crc = (crc << 4) ^ crcTable[crc >> 28];

    return (crc);
}
static uint32_t stm_crc(uint32_t crc, uint32_t data)
{
    return stm_crc_fast(crc, data);
}

uint32_t CRC32_SW(const void *data, size_t sz, uint32_t crc)
{
    // data is usually not word aligned (packet buffers, strings): read the words with memcpy
    const uint8_t *p = (const uint8_t *) data;
    uint32_t w;
    for (; sz >= sizeof(uint32_t); sz -= sizeof(uint32_t), p += sizeof(uint32_t)) {
        memcpy(&w, p, sizeof(w));
        crc = stm_crc(w, crc);
    }
    if (!sz)
        return crc;
    w = 0;
    memcpy(&w, p, sz);
    return stm_crc(w, crc);
}
