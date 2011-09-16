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
#include <inttypes.h>
#include <sys/types.h>
#include "MandalaTypes.h"
typedef Vect _var_vect;
typedef uint    _var_uint;
typedef const uint8_t*  _var_signature;
typedef double   _var_float;
//=============================================================================
#include "MandalaVars.h"
#include <string.h>
#include <stdio.h>
#define printf(...) fprintf(stdout, __VA_ARGS__ )
//=============================================================================
//modes enum
#define MODEDEF(aname,adescr) fm##aname,
enum {
#include "MandalaVars.h"
  fmCnt
};
//PID regs enum
#define REGDEF(aname,adescr) reg##aname,
enum {
#include "MandalaVars.h"
  regCnt
};

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//=============================================================================
typedef enum { wtHdg=0,wtLine,  wtCnt } _wpt_type;
#define wt_str_def "Hdg","Line"
typedef struct {
  Vect    LLA;  //lat,lon,agl
  _wpt_type type;
  uint8_t cmd[9]; //TODO: implement wpt commands
  uint    cmdSize;
}_waypoint;
//----------------------
typedef enum { rwtApproach=0,rwtParachute,  rwtCnt } _rw_type;
#define rwt_str_def "Approach","Parachute"
typedef struct {
  Vect    LLA;  //start pos [lat lon agl]
  Vect    dNED;
  _rw_type type;
  //calculated
  double hdg;
}_runway;
//=============================================================================
//=============================================================================
class Mandala
{
public:
  const char *    var_name[maxVars];
  const char *    var_descr[maxVars];
  double          var_span[maxVars];
  double          var_round[maxVars];
  double          var_mult[maxVars];  //multiplier to pack and fit to bytes
  uint            var_bytes[maxVars]; //size of packed member
  uint            var_size[maxVars];  //size of whole packed var
  uint            var_array[maxVars];
  void *          var_ptr[maxVars];
  _var_type       var_type[maxVars];
  uint8_t*        var_sig[maxVars];

  uint            var_bits[maxVars];    // number of bitfield bits
  const char      *var_bits_name[maxVars][32];  // bit descriptions
  const char      *var_bits_descr[maxVars][32];  // bit descriptions

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
  // derivatives calc by calcDGPS
  bool    derivatives_init;
  Vect    last_vNED,last_aXYZ;
  double  last_course;
  double  gps_lat_s,gps_lon_s; //change detect

#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define SIGDEF(aname,adescr,...)                              VARDEF( ,aname, , , )
#define VARDEF(atype,aname,aspan,abytes,adescr)               const char *name_##aname,*descr_##aname;
#include "MandalaVars.h"

  // enum indexes idx_VARNAME
#define VARDEF(atype,aname,aspan,abytes,adescr) idx_##aname,
  enum {
    idx_vars_start=-1,
#include "MandalaVars.h"
    idx_vars_top
  };
#define CFGDEF(atype,aname,aspan,abytes,around,adescr) idx_cfg_##aname,
  enum {
    idx_cfg_start=idxCFG-1,
#include "MandalaVars.h"
    idx_cfg_top
  };
#define SIGDEF(aname,adescr, ... ) idx_##aname,
  enum {
    idx_sig_start=idxSIG-1,
#include "MandalaVars.h"
    idx_sig_top
  };


  // variable definitions: vartype VARNAME;
#define VARDEF(atype,aname,aspan,abytes,adescr)               _var_##atype aname;
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr)        _var_##atype aname [ asize ];
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) VARDEFA(atype,cfg_##aname,asize,aspan,abytes,adescr)
#include "MandalaVars.h"


#define SIGDEF(aname,adescr, ... )   uint8_t aname [ maxVars+1 ];
#include "MandalaVars.h"


  const char      *mode_names[fmCnt];
  const char      *mode_descr[fmCnt];

  const char      *reg_names[regCnt];
  const char      *reg_descr[regCnt];


//=============================================================================
  Mandala();
public:
  uint archive(uint8_t *buf,uint size,uint var_idx);
  uint extract(const uint8_t *buf,uint size,uint var_idx); //return buf size released
  uint extract(const uint8_t *buf,uint size); //overloaded - first byte=var_idx

  uint size(void);          // size (bytes) of all archived mandala vars
  uint size(const uint8_t *signature);

  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void dump(const Vect &v,const char *str="");
  void dump(const uint var_idx);
  void print_report(FILE *stream);

  // flags
  uint status_set(uint mask,bool value=true);
  uint status_clear(uint mask);
  bool status_get(uint mask);

  // math operations
  double boundAngle(double v,double span=180.0);
  Vect boundAngle(const Vect &v,double span=180.0);
  uint snap(uint v, uint snapv=10);
  void filterValue(double v,double *vLast,double S,double L);
  double hyst(double err,double hyst);
  double limit(const double v,const double vL=1.0);
  double limit(const double v,const double vMin,const double vMax);
  double ned2hdg(const Vect &ned,bool back=false); //return heading to NED frm (0,0,0)
  double ned2dist(const Vect &ned); //return distance to to NED frm (0,0,0)
  const Vect lla2ned(const Vect &lla);  // return NED from Lat Lon AGL

  void calcDGPS(const double dt=(1.0/(double)GPS_FREQ)); //calculate GPS derivatives
  void calc(void); // calculate vars dependent on current and desired UAV position

  const Vect llh2ned(const Vect llh);
  const Vect llh2ned(const Vect llh,const Vect home_llh);
  const Vect rotate(const Vect &v_in,const double theta);
  const Vect rotate(const Vect &v_in,const Vect &theta);
  const Vect LLH_dist(const Vect &llh1,const Vect &llh2,const double lat,const double lon);
  const Vect ECEF_dist(const Vect &ecef1,const Vect &ecef2,const double lat,const double lon);
  const Vect ECEF2Tangent(const Vect &ECEF,const double latitude,const double longitude);
  const Vect Tangent2ECEF(const Vect &Local,const double latitude,const double longitude);
  const Vect ECEF2llh(const Vect &ECEF);
  const Vect llh2ECEF(const Vect &llh);
  double sqr(double x);
private:
  uint extract_sig(const uint8_t *buf,uint size,const uint8_t *signature);
  uint archive_sig(uint8_t *buf,uint size,const uint8_t *signature);

  uint archive_f(uint8_t *buf,const double v,const uint var_idx);
  uint archive_u(uint8_t *buf,const uint v,const uint var_idx);
  uint archive_s(uint8_t *buf,const int v,const uint var_idx);
  double extract_f(const uint8_t *buf,const uint var_idx);
  uint extract_u(const uint8_t *buf,const uint var_idx);
  int extract_s(const uint8_t *buf,const uint var_idx);

  uint limit_u(const uint v,const uint vL=255);
  int limit_s(const int v,const int vL=127);

  // some special protocols
  uint archive_flightplan(uint8_t *buf,uint bufSize);  //pack wypoints to buf, return size
  uint extract_flightplan(const uint8_t *buf,uint cnt);//read packed waypoints from buf
  uint archive_downstream(uint8_t *buf,uint maxSize);    //pack telemetry DownlinkStream (128 vars)
  uint extract_downstream(const uint8_t *buf,uint cnt);  //read telemetry DownlinkStream

};
//=============================================================================
#endif // MANDALA_H
