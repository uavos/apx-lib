#pragma once
#include "xbus.h"

#ifdef __cplusplus
namespace xbus {
#endif


typedef uint16_t      squawk_t;
typedef char          callsign_t [16];
typedef uint8_t       uid_t [12];
typedef uint8_t       vclass_t;

#define VEHICLE_CLASS_LIST UAV,GCU,UGV,USV,SAT,RELAY
typedef enum {
  VEHICLE_CLASS_LIST
} vehicle_class_t;

// UAV transponder data
typedef struct {
  hdr_t       hdr;
  squawk_t    squawk;
  float       lat;
  float       lon;
  int16_t     alt;
  uint16_t    gSpeed;       // [m/s]*100
  int16_t     crs;          // [deg]*32768/180
  uint8_t     mode;         // flight mode
}__attribute__((packed)) packet_xpdr_t;

typedef struct {
  hdr_t       hdr;
  squawk_t    squawk;       // dynamically assigned ID
  callsign_t  callsign;     // text name
  uid_t       uid;          // unique number
  vclass_t    vclass;       // UAV class
}__attribute__((packed)) packet_ident_t;


typedef struct {
  hdr_t       hdr;
  squawk_t    squawk;
  //payload data follows
} __attribute__((packed)) hdr_vehicle_t;


#ifdef __cplusplus
} //namespace
#endif
