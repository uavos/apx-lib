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
// flight plan types
typedef enum { wtHdg=0,wtLine,  wtCnt } _wpt_type;
#define wt_str_def "Hdg","Line"
typedef struct {
  _var_vect     LLA;  //lat,lon,agl
  _wpt_type     type;
  uint          cmdSize;
  uint8_t       cmd[128]; //TODO: implement wpt commands
}_waypoint;
//----------------------
typedef enum { rwtApproach=0,rwtParachute,  rwtCnt } _rw_type;
typedef enum { rwaLeft=0,rwaRight, rwaCnt } _rw_app;
#define rwt_str_def "Approach","Parachute"
#define rwa_str_def "Left","Right"
typedef struct {
  _var_vect     LLA;  //start pos [lat lon agl]
  _var_vect     dNED;
  _rw_type      rwType;
  _rw_app       appType;
  _var_float    distApp;
  _var_float    altApp;
  _var_float    distTA;
  _var_float    altTA;
  //calculated
  _var_float    hdg;
}_runway;
#define MAX_WPCNT       100
#define MAX_RWCNT       10
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
//=============================================================================
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
#define CFGDEF(atype,aname,aspan,abytes,around,adescr) idx_cfg_##aname,
enum {
  #include "MandalaVarsAP.h"
  cfgCnt
};
#define REGDEF(aname,adescr) reg##aname,
enum {
  #include "MandalaVarsAP.h"
  regCnt
};
//=============================================================================
//-----------------------------------------------------------------------------
// config variable typedefs
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) typedef _var_##atype var_typedef_cfg_##aname [asize];
#define CFGDEF(atype,aname,aspan,abytes,around,adescr) typedef _var_##atype var_typedef_cfg_##aname;
#include "MandalaVarsAP.h"
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
  double          var_span[256];

  uint            var_bits[256];            // number of bitfield bits
  uint8_t         var_bits_mask[256][8];    // bitmask of bitfield
  const char      *var_bits_name[256][8];   // bit names
  const char      *var_bits_descr[256][8];  // bit descriptions


  //---- AP CFG Vars ----
  struct {
    const char *    var_name[cfgCnt];  //text name
    const char *    var_descr[cfgCnt]; //text description
    double          var_round[cfgCnt]; //round value for CFG vars
    uint            var_size[cfgCnt];  //size of whole packed var
    void *          var_ptr[cfgCnt];
    uint            var_type[cfgCnt];
    uint            var_array[cfgCnt];
    uint            var_bytes[cfgCnt];
    double          var_span[cfgCnt];
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)  var_typedef_cfg_##aname aname;
#include "MandalaVarsAP.h"
  }cfg;

  //---- flightplan ----
  _flightplan fp;
  const char *wt_str[wtCnt];    //wt_type string descr
  const char *rwt_str[rwtCnt];  //wt_type string descr
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
  //---- calc ----
  double  gps_lat_s,gps_lon_s; //change detect

  const char      *reg_names[regCnt];
  const char      *reg_descr[regCnt];

//=============================================================================
  Mandala();
  //-----------------------------------------------------------------------------
  // Core overload
  //uint archive(uint8_t *buf,uint var_idx);
  uint extract(uint8_t *buf,uint cnt,uint var_idx);
//public:
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
  double boundAngle(double v,double span=180.0);
  _var_vect boundAngle(const _var_vect &v,double span=180.0);
  uint snap(uint v, uint snapv=10);
  double hyst(double err,double hyst);
  double limit(const double v,const double vL=1.0);
  double limit(const double v,const double vMin,const double vMax);
  double ned2hdg(const _var_vect &ned,bool back=false); //return heading to NED frm (0,0,0)
  double ned2dist(const _var_vect &ned); //return distance to to NED frm (0,0,0)
  const _var_vect lla2ned(const _var_vect &lla);  // return NED from Lat Lon AGL

  void calc(void); // calculate vars dependent on current and desired UAV position

  const _var_vect llh2ned(const _var_vect llh);
  const _var_vect llh2ned(const _var_vect llh,const _var_vect home_llh);
  const _var_vect rotate(const _var_vect &v_in,const double theta);
  const _var_vect rotate(const _var_vect &v_in,const _var_vect &theta);
  const _var_vect LLH_dist(const _var_vect &llh1,const _var_vect &llh2,const double lat,const double lon);
  const _var_vect ECEF_dist(const _var_vect &ecef1,const _var_vect &ecef2,const double lat,const double lon);
  const _var_vect ECEF2Tangent(const _var_vect &ECEF,const double latitude,const double longitude);
  const _var_vect Tangent2ECEF(const _var_vect &Local,const double latitude,const double longitude);
  const _var_vect ECEF2llh(const _var_vect &ECEF);
  const _var_vect llh2ECEF(const _var_vect &llh);
  const _var_vect ned2llh(const _var_vect &ned);
  const _var_vect ned2llh(const _var_vect &ned,const _var_vect &home_llh);
  double sqr(double x);
  double inHgToAltitude(double inHg);
private:
  // some special protocols
  void fill_config_vdsc(uint8_t *buf,uint i);
  uint archive_config(uint8_t *buf,uint bufSize);  //pack config to buf, return size
  uint extract_config(uint8_t *buf,uint cnt);//read packed config from buf
  uint archive_flightplan(uint8_t *buf,uint bufSize);  //pack wypoints to buf, return size
  uint extract_flightplan(uint8_t *buf,uint cnt);//read packed waypoints from buf
  uint archive_downstream(uint8_t *buf,uint maxSize);    //pack telemetry
  uint extract_downstream(uint8_t *buf,uint cnt);  //read telemetry
  uint extract_setb(uint8_t *buf,uint cnt);  //read
  uint extract_clrb(uint8_t *buf,uint cnt);  //read
};
//=============================================================================
#endif // MANDALA_H
