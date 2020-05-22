/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CRC32.h"

#include "crc32_sw.h"

CRC32::CRC32()
{
    reset();
}

CRC32::CRC32(const void *data, size_t sz)
{
    reset();
    add(data, sz);
}

void CRC32::reset()
{
    _value = 0xFFFFFFFF;
}

uint32_t CRC32::result() const
{
    return _value;
}

void CRC32::add(const void *data, size_t sz)
{
    _value = CRC32_SW(data, sz, _value);
}
