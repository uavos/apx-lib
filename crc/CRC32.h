/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>
#include <sys/types.h>
#include <visibility.h>

namespace apx {

__EXPORT uint32_t crc32(const void *data, size_t sz);

}
