#pragma once

#include <inttypes.h>
#include <sys/types.h>

#ifdef __GNUG__
// UAV identification
namespace IDENT
{
#endif

typedef uint16_t      _squawk;
typedef char          _callsign[16];
typedef uint8_t       _uid[12];
typedef uint8_t       _vclass;
#define VEHICLE_CLASS_LIST UAV,GCU,UGV,USV,SAT,RELAY
typedef enum{VEHICLE_CLASS_LIST}_vehicle_class;
// UAV transponder data
typedef struct {
  _squawk   squawk;
  float     lat;
  float     lon;
  int16_t   alt;
  uint16_t  gSpeed;       // [m/s]*100
  int16_t   crs;          // [deg]*32768/180
  uint8_t   mode;         // flight mode
}__attribute__((packed)) _xpdr;
typedef struct {
  _squawk   squawk;       //dynamically assigned ID
  _callsign callsign;     //text name
  _uid      uid;          //unique number
  _vclass   vclass;       //UAV class
}__attribute__((packed)) _ident;

#ifdef __GNUG__
}
#endif
