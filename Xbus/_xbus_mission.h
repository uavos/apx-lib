#pragma once
#include "xbus.h"

#ifdef __cplusplus
namespace xbus {
namespace mission {
#endif

//---- Mission (typedefs only) ----
typedef enum{ //4bits
  mi_stop=0,
  mi_wp,
  mi_rw,
  mi_tw,
  mi_pi,
  mi_action,
  mi_restricted,
  mi_emergency
} mission_item_t;

typedef struct{
  uint8_t type        :4;     //wp,rw,scr, ..
  uint8_t option      :4;     //left,right,line,hdg, ..
}__attribute__((packed)) mission_hdr_t;

typedef struct{
  mission_hdr_t hdr;
  float           lat;
  float           lon;
  int16_t         alt;
}__attribute__((packed)) mission_wp_t;

typedef enum {
  mo_hdg,
  mo_line,
} mission_wp_option_t;

typedef struct{
  mission_hdr_t hdr;
  float           lat;
  float           lon;
  int16_t         hmsl;
  int16_t         dN;
  int16_t         dE;
  uint16_t        approach;
}__attribute__((packed)) mission_rw_t;

typedef enum {
  mo_left,
  mo_right,
} mission_rw_option_t;

typedef struct{
  mission_hdr_t hdr;
  float           lat;
  float           lon;
}__attribute__((packed)) mission_tw_t;

typedef struct{
  mission_hdr_t hdr;
  float           lat;
  float           lon;
  int16_t         hmsl;
  int16_t         turnR;
  uint8_t         loops;
  uint16_t        timeS;
}__attribute__((packed)) mission_pi_t;

typedef enum {
  mo_speed,
  mo_poi,
  mo_scr,
  mo_loiter,
  mo_shot,
} mission_action_option_t;

typedef struct{
  mission_hdr_t hdr;
  union{
    uint8_t         speed;      //0=cruise
    uint8_t         poi;        //linked POI [1...n]
    char            scr[16];    //public func @name
    struct{
      int16_t       turnR;
      uint8_t       loops;      //loops to loiter
      uint16_t      timeS;      //time to loiter
    }__attribute__((packed)) loiter;
    struct{
      int16_t       dist      :12;      //distance for series
      int16_t       opt       :4;       //0=single,1=start,2=stop
    }__attribute__((packed)) shot;
  }__attribute__((packed));


#ifdef __cplusplus
  int size() const
  {
    int sz=sizeof(mission_hdr_t);
    switch(hdr.option){
      case mo_speed: sz+=sizeof(speed);break;
      case mo_poi:   sz+=sizeof(poi);break;
      case mo_scr:   sz+=sizeof(scr);break;
      case mo_loiter:sz+=sizeof(loiter);break;
      case mo_shot:  sz+=sizeof(shot);break;
    }
    return sz;
  }
#endif
}__attribute__((packed)) mission_action_t;

typedef struct{
  float   lat;
  float   lon;
}__attribute__((packed)) mission_area_point_t;

typedef struct{
  mission_hdr_t hdr;
  uint8_t   pointsCnt;

#ifdef __cplusplus
  int size() const
  {
    return sizeof(hdr)+sizeof(pointsCnt)+sizeof(mission_area_point_t)*pointsCnt;
  }
#endif
}__attribute__((packed)) mission_area_t;





#ifdef __cplusplus
} //namespace
} //namespace
#endif
