#pragma once
#include "xbus_node.h"

#ifdef __cplusplus
namespace xbus {
#endif

typedef struct{
  uint32_t magic;
  uint32_t uptime;
  uint32_t oncnt;
  uint32_t evtcnt[3];
  uint32_t rec_size; //blocks
  uint8_t  padding[32-20-4];
  uint32_t crc;
}__attribute__((packed)) node_bb_hdr_t;

typedef enum {
  bbr_hdr,
  bbr_data
} node_bb_req_t;


#ifdef __cplusplus
} //namespace
#endif
