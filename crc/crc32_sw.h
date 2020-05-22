#pragma once

#include <common/visibility.h>
#include <stdint.h>
#include <sys/types.h>

__BEGIN_DECLS

uint32_t CRC32_SW(const void *data, size_t sz, uint32_t crc);

__END_DECLS
