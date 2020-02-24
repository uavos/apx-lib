#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {

typedef uint16_t pid_t;
typedef uint8_t pid8_t;

constexpr const size_t size_packet_max = 512;

} // namespace xbus
