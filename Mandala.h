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
//#include <stdint.h>
#include <math.h>
//-----------------------------------------------------------------------------
#include "MandalaCore.h"
#define printf(...) fprintf(stdout, __VA_ARGS__ )
//=============================================================================
// flight plan types
typedef enum { wtHdg=0,wtLine,  wtCnt } _wpt_type;
#define wt_str_def "Hdg","Line"
typedef struct {
  Vector        LLA;  //lat,lon,agl
  _wpt_type     type;
  uint8_t       cmd[9]; //TODO: implement wpt commands
  uint          cmdSize;
}_waypoint;
//----------------------
typedef enum { rwtApproach=0,rwtParachute,  rwtCnt } _rw_type;
#define rwt_str_def "Approach","Parachute"
typedef struct {
  Vector        LLA;  //start pos [lat lon agl]
  Vector        dNED;
  _rw_type      type;
  //calculated
  double        hdg;
}_runway;
//=============================================================================
class Mandala : public MandalaCore
{
public:
  const char *    var_name[maxVars];  //text name
  const char *    var_descr[maxVars]; //text description
  double          var_round[maxVars]; //round value for CFG vars
  uint            var_size[maxVars];  //size of whole packed var
  //double          var_mult[maxVars];  //multiplier to pack and fit to bytes
  //uint            var_bytes[maxVars]; //size of packed member
  //uint8_t*        var_sig[maxVars];
  void *          var_ptr[maxVars];
  _var_type       var_type[maxVars];
  uint            var_array[maxVars];
  double          var_span[maxVars];

  uint            var_bits[maxVars];    // number of bitfield bits
  const char      *var_bits_name[maxVars][32];  // bit descriptions
  const char      *var_bits_descr[maxVars][32];  // bit descriptions


  //---- CFG Vars ----
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)  var_typedef_cfg_##aname aname;
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
#include "MandalaVars.h"


  //---- Waypoints ----
  _waypoint waypoints[100];
  _runway   runways[10];
  const char *wt_str[wtCnt];  //wt_type string descr
  const char *rwt_str[rwtCnt];  //wt_type string descr

  //---- Internal use -----
  // telemetry framework
  uint8_t dl_snapshot[2048];    // all archived variables (first 128) snapshot
  bool    dl_reset;             // set true to send everything next time
  uint8_t dl_reset_mask[128/8]; // bitmask 1=var send anyway, auto clear after sent
  uint8_t dl_var[32];           // one var max size (tmp buf)
  uint    dl_frcnt;             // downlink frame cnt for eeror check (inc by archiveTelemety)
  uint    dl_errcnt;            // errors counter (by extractTelemetry)
  uint    dl_timestamp;         // time[ms]
  uint    dl_dt;                // dt[ms] by extractTelemetry
  uint    dl_size;              // last telemetry size statistics
  //---- derivatives calc by calcDGPS ----
  bool    derivatives_init;
  Vector  last_vNED,last_aXYZ;
  double  last_course;
  double  gps_lat_s,gps_lon_s; //change detect

  //---- strings ----
  // var names and descriptions string declarations: name_XXXX, descr_XXX
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define SIGDEF(aname,adescr,...)                              VARDEF( ,aname, , , )
#define VARDEF(atype,aname,aspan,abytes,adescr)               const char *name_##aname,*descr_##aname;
#include "MandalaVars.h"
  const char      *mode_names[fmCnt];
  const char      *mode_descr[fmCnt];

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

  //uint size(void);          // size (bytes) of all archived mandala vars
  uint sig_size(_var_signature signature);

  //-----------------------------------------------------------------------------
  // additional methods (debug, math, NAV helper functions)
  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void dump(const Vector &v,const char *str="");
  void dump(const uint var_idx);
  void print_report(FILE *stream);

  // flags
  uint status_set(uint mask,bool value=true);
  uint status_clear(uint mask);
  bool status_get(uint mask);

  // math operations
  double boundAngle(double v,double span=180.0);
  Vector boundAngle(const Vector &v,double span=180.0);
  uint snap(uint v, uint snapv=10);
  void filterValue(double v,double *vLast,double S,double L);
  double hyst(double err,double hyst);
  double limit(const double v,const double vL=1.0);
  double limit(const double v,const double vMin,const double vMax);
  double ned2hdg(const Vector &ned,bool back=false); //return heading to NED frm (0,0,0)
  double ned2dist(const Vector &ned); //return distance to to NED frm (0,0,0)
  const Vector lla2ned(const Vector &lla);  // return NED from Lat Lon AGL

  void calcDGPS(const double dt=(1.0/(double)GPS_FREQ)); //calculate GPS derivatives
  void calc(void); // calculate vars dependent on current and desired UAV position

  const Vector llh2ned(const Vector llh);
  const Vector llh2ned(const Vector llh,const Vector home_llh);
  const Vector rotate(const Vector &v_in,const double theta);
  const Vector rotate(const Vector &v_in,const Vector &theta);
  const Vector LLH_dist(const Vector &llh1,const Vector &llh2,const double lat,const double lon);
  const Vector ECEF_dist(const Vector &ecef1,const Vector &ecef2,const double lat,const double lon);
  const Vector ECEF2Tangent(const Vector &ECEF,const double latitude,const double longitude);
  const Vector Tangent2ECEF(const Vector &Local,const double latitude,const double longitude);
  const Vector ECEF2llh(const Vector &ECEF);
  const Vector llh2ECEF(const Vector &llh);
  double sqr(double x);
private:
  // some special protocols
  uint archive_flightplan(uint8_t *buf,uint bufSize);  //pack wypoints to buf, return size
  uint extract_flightplan(uint8_t *buf,uint cnt);//read packed waypoints from buf
  uint archive_downstream(uint8_t *buf,uint maxSize);    //pack telemetry DownlinkStream (128 vars)
  uint extract_downstream(uint8_t *buf,uint cnt);  //read telemetry DownlinkStream

};
//=============================================================================
#endif // MANDALA_H
