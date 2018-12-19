/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 *  http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MISSION_H
#define MISSION_H
//=============================================================================
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
class Mission
{
public:
  Mission();

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
  } _item_type;

  typedef struct{
    uint8_t type        :4;     //wp,rw,scr, ..
    uint8_t option      :4;     //left,right,line,hdg, ..
  }__attribute__((packed)) _item_hdr;

  typedef struct{
    _item_hdr hdr;
    float           lat;
    float           lon;
    int16_t         alt;
  }__attribute__((packed)) _item_wp;

  typedef enum {mo_hdg,mo_line} _item_wp_option;
  typedef struct{
    _item_hdr hdr;
    float           lat;
    float           lon;
    int16_t         hmsl;
    int16_t         dN;
    int16_t         dE;
    uint16_t        approach;
  }__attribute__((packed)) _item_rw;

  typedef enum {mo_left,mo_right} _item_rw_option;
  typedef struct{
    _item_hdr hdr;
    float           lat;
    float           lon;
  }__attribute__((packed)) _item_tw;

  typedef struct{
    _item_hdr hdr;
    float           lat;
    float           lon;
    int16_t         hmsl;
    int16_t         turnR;
    uint8_t         loops;
    uint16_t        timeS;
  }__attribute__((packed)) _item_pi;

  typedef struct{
    _item_hdr hdr;
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
  }__attribute__((packed)) _item_action;
  typedef enum {mo_speed,mo_poi,mo_scr,mo_loiter,mo_shot} _item_action_option;

  typedef struct{
    _item_hdr hdr;
    uint8_t   pointsCnt;
  }__attribute__((packed)) _item_area;
  typedef struct{
    float   lat;
    float   lon;
  }__attribute__((packed)) _item_area_point;

  void clear(void);
  bool update(uint8_t *buf,uint cnt);
  int next(int pos, _item_type type);
  int next(int pos, _item_type type, const uint8_t *data_ptr, const uint data_size);
  _item_wp *wp(int idx);
  _item_pi *pi(int idx);
  _item_rw *rw(int idx);
  _item_tw *tw(int idx);

  _item_action *action(int idx); //current wp appended action

  _item_area *restricted(int idx);
  _item_area_point *area_point(_item_area *area,int idx);

  struct{
    _item_wp *wp;
    _item_pi *pi;
    //loiter
    int turnR;
    uint loops;
    uint timeS;
  }current;

  void reset_current(void);

  static inline int action_size(uint8_t option)
  {
    int sz=sizeof(_item_hdr);
    switch(option){
      case mo_speed: sz+=sizeof(_item_action::speed);break;
      case mo_poi:   sz+=sizeof(_item_action::poi);break;
      case mo_scr:   sz+=sizeof(_item_action::scr);break;
      case mo_loiter:sz+=sizeof(_item_action::loiter);break;
      case mo_shot:  sz+=sizeof(_item_action::shot);break;
    }
    return sz;
  }

  static inline int area_size(uint8_t pointsCnt)
  {
    return sizeof(_item_area)+sizeof(_item_area_point)*pointsCnt;
  }

  uint8_t data[4096];
  uint size;
};
//=============================================================================
#endif
