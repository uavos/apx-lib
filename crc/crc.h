#pragma once

#include <common/visibility.h>
#include <stdint.h>
#include <sys/types.h>

__BEGIN_DECLS

uint16_t CRC_16_APX(const void *data, size_t sz, uint16_t crc);
uint32_t CRC_32_APX(const void *data, size_t sz, uint32_t crc);

// deprecated
uint16_t CRC_16_IBM(const void *data, size_t sz, uint16_t crc);
uint8_t CRC_8XOR(const uint8_t *buf, size_t sz, uint8_t crc);
uint8_t CRC_8_8C(const uint8_t *buf, size_t sz, uint8_t crc);
uint8_t CRC_8(uint8_t poly, const uint8_t *buf, size_t sz, uint8_t crc);
uint32_t CRC_SUM(const uint8_t *buf, size_t sz, uint32_t crc);
uint16_t CRC_16_CCITT(const uint8_t *buf, size_t sz, uint16_t crc);
uint16_t CRC_16(uint16_t poly, const uint8_t *buf, size_t sz, uint16_t crc);
uint16_t CRC_16_CCITT_QT(const uint8_t *buf, size_t sz, uint16_t crc);

uint32_t CRC_32(const void *data, size_t sz, uint32_t crc, const uint32_t poly);

//used by nodes conf
uint32_t CRC_32_NODES(const void *data, size_t sz, uint32_t crc);

__END_DECLS
