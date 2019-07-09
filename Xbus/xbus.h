#pragma once
#include <inttypes.h>
#include <visibility.h>

#ifdef __cplusplus
namespace xbus {
#endif

// xbus general data packet structure

typedef uint8_t pid_t;

enum {
  size_xbus_packet      = 1024,
};

typedef struct {
  pid_t   pid;   //<var_idx>
  //payload data follows
} __attribute__((packed)) hdr_t;

#ifdef __cplusplus
} //namespace
#endif
