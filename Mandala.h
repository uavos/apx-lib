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
#define AHRS_FREQ       100     // AHRS Update (main loop freq) [Hz]
#define GPS_FREQ        5       // GPS Update rate (for derivatives) [Hz]
#define CTR_FREQ        50      // ctr (fast servo) send rate [Hz]
#define UPD_FREQ        5       // 'update' vars send rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz] MAX 10Hz!
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
#define MAX_TELEMETRY   64      // max telemetry packet size [bytes]
//=============================================================================
// flight plan types
#define MAX_WPCNT       100
#define MAX_RWCNT       10
typedef enum { wtHdg=0,wtLine,  wtCnt } _wpt_type;
typedef enum { rwaLeft=0,rwaRight, rwaCnt } _rw_app;
#define wt_str_def "Hdg","Line"
#define rwa_str_def "Left","Right"
typedef struct {
  _var_vect     LLA;  //lat,lon,agl
  uint8_t       type;
  uint8_t       cmdSize;
  uint8_t       cmd[128]; //TODO: implement wpt commands
}_waypoint;
typedef struct {
  _var_vect     LLA;  //start pos [lat lon agl]
  _var_vect     dNED;
  uint8_t       appType;
  _var_float    distApp;
  _var_float    altApp;
  _var_float    distTA;
  _var_float    altTA;
  //calculated
  _var_float    hdg;
}_runway;
typedef struct {
  _waypoint waypoints[MAX_WPCNT];
  _runway   runways[MAX_RWCNT];
  struct{ //config override
    _var_float altitude;
    _var_float dHome;
    _var_float dHomeERS;
  }safety;
}_flightplan;
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

  const char *    var_name[256];  //text name
  const char *    var_descr[256]; //text description
  uint            var_size[256];  //size of whole packed var
  void *          var_ptr[256];
  uint            var_type[256];
  _var_float      var_span[256];

  uint            var_bits[256];            // number of bitfield bits
  uint8_t         var_bits_mask[256][8];    // bitmask of bitfield
  const char      *var_bits_name[256][8];   // bit names
  const char      *var_bits_descr[256][8];  // bit descriptions

  //---- flightplan ----
  _flightplan fp;
  const char *wt_str[wtCnt];    //wt_type string descr
  const char *rwa_str[rwaCnt];  //wt_type string descr

  //---- Internal use -----
  // telemetry framework
  uint8_t dl_id;                // received uav_id
  uint8_t dl_snapshot[2048];    // all archived variables snapshot
  bool    dl_reset;             // set true to send everything next time
  uint8_t dl_reset_mask[128/8]; // bitmask 1=var send anyway, auto clear after sent
  uint8_t dl_var[32];           // one var max size (tmp buf)
  //filled by extract_downstream
  uint          dl_frcnt;       // downlink frame cnt (inc by extract_downlink)
  uint          dl_errcnt;      // errors counter (by extract_downlink)
  uint          dl_timestamp;   // time[ms]
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
  uint extract(uint8_t *buf,uint cnt,uint var_idx);

  //-----------------------------------------------------------------------------
  //overload - check buf size
  uint archive(uint8_t *buf,uint size,uint var_idx);
  uint extract(uint8_t *buf,uint size); //overloaded - first byte=var_idx
  //-----------------------------------------------------------------------------

  uint sig_size(_var_signature signature);

  //-----------------------------------------------------------------------------
  // additional methods (debug, math, NAV helper functions)
  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void dump(const _var_vect &v,const char *str="");
  void dump(const uint var_idx);
  void print_report(FILE *stream);

  // math operations
  _var_float boundAngle(_var_float v,_var_float span=180.0);
  _var_vect boundAngle(const _var_vect &v,_var_float span=180.0);
  uint snap(uint v, uint snapv=10);
  _var_float hyst(_var_float err,_var_float hyst);
  _var_float limit(const _var_float v,const _var_float vL=1.0);
  _var_float limit(const _var_float v,const _var_float vMin,const _var_float vMax);
  _var_float ned2hdg(const _var_vect &ned,bool back=false); //return heading to NED frm (0,0,0)
  _var_float ned2dist(const _var_vect &ned); //return distance to to NED frm (0,0,0)
  const _var_vect lla2ned(const _var_vect &lla);  // return NED from Lat Lon AGL

  void calc(void); // calculate vars dependent on current and desired UAV position

  const _var_vect llh2ned(const _var_vect llh);
  const _var_vect llh2ned(const _var_vect llh,const _var_vect home_llh);
  const _var_vect rotate(const _var_vect &v_in,const _var_float theta);
  const _var_vect rotate(const _var_vect &v_in,const _var_vect &theta);
  const _var_vect LLH_dist(const _var_vect &llh1,const _var_vect &llh2,const _var_float lat,const _var_float lon);
  const _var_vect ECEF_dist(const _var_vect &ecef1,const _var_vect &ecef2,const _var_float lat,const _var_float lon);
  const _var_vect ECEF2Tangent(const _var_vect &ECEF,const _var_float latitude,const _var_float longitude);
  const _var_vect Tangent2ECEF(const _var_vect &Local,const _var_float latitude,const _var_float longitude);
  const _var_vect ECEF2llh(const _var_vect &ECEF);
  const _var_vect llh2ECEF(const _var_vect &llh);
  const _var_vect ned2llh(const _var_vect &ned);
  const _var_vect ned2llh(const _var_vect &ned,const _var_vect &home_llh);
  _var_float sqr(_var_float x);
private:
  // some special protocols
  uint archive_flightplan(uint8_t *buf,uint bufSize);  //pack wypoints to buf, return size
  uint extract_flightplan(uint8_t *buf,uint cnt);//read packed waypoints from buf
  uint archive_downstream(uint8_t *buf,uint maxSize);    //pack telemetry
  uint extract_downstream(uint8_t *buf,uint cnt);  //read telemetry
  uint extract_setb(uint8_t *buf,uint cnt);  //read
  uint extract_clrb(uint8_t *buf,uint cnt);  //read
};
//=============================================================================
#endif // MANDALA_H
