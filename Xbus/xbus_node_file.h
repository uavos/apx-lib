#pragma once
#include "xbus_node.h"

#ifdef __cplusplus
namespace xbus {
#endif

typedef uint8_t node_file_buf_t [256];

typedef struct{
  uint32_t      start_address;
  uint32_t      size;        // available size in bytes
  uint8_t       xor_crc;     // all file data XORed
}__attribute__((packed)) node_file_t;

typedef struct{
  uint32_t      start_address;
  uint16_t      data_size;
}__attribute__((packed)) node_file_data_hdr_t;

typedef struct{
  node_file_data_hdr_t  data_hdr;
  node_file_buf_t       data;
}__attribute__((packed)) node_file_data_t;



#ifdef __cplusplus
} //namespace
#endif
