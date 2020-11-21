#pragma once

#include <stdint.h>
#include <visibility.h>

__BEGIN_DECLS

uint16_t CRC_16_IBM(const void *data, uint32_t size, uint16_t crc);
uint8_t CRC_8XOR(const uint8_t *buf, uint32_t size, uint8_t crc);
uint8_t CRC_8_8C(const uint8_t *buf, uint32_t size, uint8_t crc);
uint8_t CRC_8(uint8_t poly, const uint8_t *buf, uint32_t size, uint8_t crc);
uint32_t CRC_SUM(const uint8_t *buf, uint32_t size, uint32_t crc);
uint16_t CRC_16_CCITT(const uint8_t *buf, uint32_t size, uint16_t crc);
uint16_t CRC_16(uint16_t poly, const uint8_t *buf, uint32_t size, uint16_t crc);
uint16_t CRC_16_CCITT_QT(const uint8_t *buf, uint32_t size, uint16_t crc);

uint32_t CRC_32(const void *data, uint32_t size, uint32_t crc, const uint32_t poly);

//used by nodes conf
uint32_t CRC_32_NODES(const void *data, uint32_t size, uint32_t crc);

__END_DECLS
