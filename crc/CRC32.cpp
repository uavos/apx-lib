/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CRC32.h"

#include "crc32_sw.h"

namespace apx {

uint32_t crc32(const void *data, size_t sz, uint32_t crc)
{
    return CRC32_SW(data, sz, crc);
}

} // namespace apx
