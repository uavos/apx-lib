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
#ifndef MANDALA_H
#define MANDALA_H
//=============================================================================
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
//-----------------------------------------------------------------------------
#include "MandalaCore.h"
#define printf(...) fprintf(stdout, __VA_ARGS__ )
//=============================================================================
// AP constants
//=============================================================================
#ifndef AHRS_FREQ
#define AHRS_FREQ       200     // AHRS Update (main loop freq) [Hz]
#endif
#define GPS_FREQ        5       // GPS Update rate (for derivatives) [Hz]
#define CTR_FREQ        100     // ctr (fast servo) send rate [Hz]
#define UPD_FREQ        5       // 'update' vars send rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz] MAX 10Hz!
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
#define MAX_TELEMETRY   100     // max telemetry packet size [bytes]
//=============================================================================
// UAV identification
typedef struct {
  uint8_t       id;             //dynamically assigned ID
  uint8_t       sn[12];         //serial number
  uint8_t       name[16];       //text name
  uint8_t       nodes_cnt;      //number of nodes in the system
}_uav_id;
//=============================================================================
class Mandala : public MandalaCore
{
public:
  _uav_id         id;

  bool get_text_names(uint16_t varmsk,const char **name,const char **descr);
  const char *var_name(uint8_t var_idx);
  uint8_t var_index(const char *name);

  //---- Mission (typedefs only) ----
  typedef enum{
    mi_stop=0,mi_wp,mi_rw
  } __mission_item_type;
  typedef struct{
    uint8_t type        :4;     //wp,rw,scr, ..
    uint8_t option      :4;     //left,right,line,hdg, ..
  }__attribute__((packed)) _mission_item_hdr;
  typedef struct{
    _mission_item_hdr hdr;
    float           lat;
    float           lon;
    int16_t         alt;
  }__attribute__((packed)) _mission_item_wp;
  typedef enum {owp_hdg,owp_line} _mission_item_wp_option;
  typedef struct{
    _mission_item_hdr hdr;
    float           lat;
    float           lon;
    int16_t         hmsl;
    int16_t         dN;
    int16_t         dE;
    uint16_t        approach;
  }__attribute__((packed)) _mission_item_rw;
  typedef enum {orw_left,orw_right} _mission_item_rw_option;
  typedef struct{
    _mission_item_hdr hdr;
    uint8_t   size;
    uint8_t   data[];
  }__attribute__((packed)) _mission_item_scr;

  //---- Internal use -----
  bool blockDownstream;
  // telemetry framework
  uint8_t dl_id;                // received uav_id
  bool    dl_reset;             // set true to send everything next time
  uint8_t dl_snapshot[1024];    // all archived variables snapshot
  uint8_t dl_reset_mask[(idx_vars_top-idxPAD)/8+(((idx_vars_top-idxPAD)&3)?1:0)]; // bitmask 1=send var, auto clear after sent
  uint8_t dl_var[3*4];          // one var max size (tmp buf)
  //filled by extract_downstream
  uint          dl_frcnt;       // downlink frame cnt (inc by extract_downlink)
  uint          dl_errcnt;      // errors counter (by extract_downlink)
  uint          dl_time_s;      // last dl sent time[ms]
  uint16_t      dl_ts;          // previous timestamp (to count dl_dt)
  uint16_t      dl_Pdt;         // previous delta (to count errors)
  bool          dl_e;           // if delta was different (errors counter)
  uint          dl_size;        // last telemetry size statistics
  bool          dl_hd_save;     // to watch change in alt_bytecnt

//=============================================================================
  Mandala();
  void init(void);
  //-----------------------------------------------------------------------------
  // Core overload
  uint extract(const uint8_t *buf, uint cnt, uint var_idx);

  //-----------------------------------------------------------------------------
  //extended - check buf size
  uint archive(uint8_t *buf,uint size,uint var_idx);
  uint extract(const uint8_t *buf,uint size); //overloaded - first byte=var_idx
  //-----------------------------------------------------------------------------

  //-----------------------------------------------------------------------------
  // additional methods (debug, math, NAV helper functions)
  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void dump(const _var_vect &v,const char *str="");
  void dump(uint8_t var_idx);

  // math operations
  _var_float boundAngle(_var_float v,_var_float span=180.0) const;
  _var_float boundAngle360(_var_float v) const;
  _var_vect  boundAngle(const _var_vect &v,_var_float span=180.0) const;
  _var_float smoothAngle(_var_float v,_var_float v_prev,_var_float speed);
  void filter_a(const _var_float &v,_var_float *var_p,const _var_float &f);
  uint snap(uint v, uint snapv=10);
  _var_float hyst(_var_float err,_var_float hyst);
  _var_float limit(const _var_float v,const _var_float vL=1.0);
  _var_float limit(const _var_float v,const _var_float vMin,const _var_float vMax);

  void calc(void); // calculate vars dependent on current and desired UAV position

  const _var_point lla2ne(const _var_vect &lla) const;  // return NED from Lat Lon AGL
  const _var_point llh2ne(const _var_vect llh) const;
  const _var_point llh2ne(const _var_vect llh,const _var_vect home_llh) const;
  _var_float heading(const _var_point &ne,bool back=false) const; //return heading to NED frm (0,0,0)
  _var_float heading(const _var_float N,const _var_float E,bool back=false) const;
  _var_float distance(const _var_point &ne) const; //return distance to to NED frm (0,0,0)
  _var_float distance(const _var_float N,const _var_float E) const;
  const _var_point rotate(const _var_point &v_in,const _var_float psi) const;
  const _var_point rotate(const _var_float N,const _var_float E,const _var_float psi) const;
  const _var_vect rotate(const _var_vect &v_in,const _var_float psi) const;
  const _var_vect rotate(const _var_vect &v_in,const _var_vect &theta) const;
  const _var_point LLH_dist(const _var_vect &llh1,const _var_vect &llh2,const _var_float lat,const _var_float lon) const;
  const _var_point ECEF_dist(const _var_vect &ecef1,const _var_vect &ecef2,const _var_float lat,const _var_float lon) const;
  const _var_point ECEF2Tangent(const _var_vect &ECEF,const _var_float latitude,const _var_float longitude) const;
  const _var_vect Tangent2ECEF(const _var_point &ne,const _var_float latitude,const _var_float longitude) const;
  const _var_point ECEF2ll(const _var_vect &ECEF) const;
  const _var_vect llh2ECEF(const _var_vect &llh) const;
  const _var_point ne2ll(const _var_point &ne) const;
  const _var_point ne2ll(const _var_point &ne,const _var_vect &home_llh) const;
  _var_float sqr(const _var_float x) const;
  _var_float wind_triangle(_var_float crs) const; //return TAS to gSpeed multiplier for given course
  _var_float wind_circle(_var_float crs,_var_float span,_var_float r) const; //return air path length for ground circle flight
private:
  // some special protocols
  uint archive_downstream(uint8_t *buf,uint maxSize);
  uint extract_downstream(const uint8_t *buf,uint cnt);
};
//=============================================================================
#endif // MANDALA_H
