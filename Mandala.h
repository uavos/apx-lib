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
//=============================================================================
#include "MandalaTypes.h"
#include "MandalaVars.h"
#include <string.h>
#include <sys/types.h>
//=============================================================================
#define printf(...) fprintf(stdout, __VA_ARGS__ )
//=============================================================================
//commands enum
#define CMDDEF(aname,aargs,aalias,adescr) cmd##aname,
enum {
#include "MandalaVars.h"
  cmdCnt
};
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
//=============================================================================
typedef enum { wtHdg=0,wtLine } _wpt_type;
typedef struct {
  Vect    LLA;  //lat,lon,agl
  _wpt_type type;
  uint8_t cmd[9]; //TODO: implement wpt commands
  uint    cmdSize;
}_waypoint;
//----------------------
typedef enum { rwApproach=0,rwParachute } _rw_type;
typedef struct {
  Vect    LLA;  //start pos [lat lon agl]
  Vect    dNED;
  _rw_type type;
  //calculated
  double hdg;
}_runway;
//=============================================================================
class Mandala
{
public:
  const char *    var_name[maxVars];
  const char *    var_descr[maxVars];
  double          var_span[maxVars];
  double          var_round[maxVars];
  uint            var_bytes[maxVars];
  uint            var_array[maxVars];
  void *          var_ptr[maxVars];
  _var_type       var_type[maxVars];
  uint8_t*        var_sig[maxVars];

  const char      *cmd_name[cmdCnt];
  const char      *cmd_alias[cmdCnt];
  const char      *cmd_descr[cmdCnt];
  int             cmd_size[cmdCnt];

  uint  var_void;

  //---- Waypoints ----
  _waypoint waypoints[100];
  _runway   runways[10];




  // names, descriptions;

#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define SIGDEF(aname,...)                                     VARDEF( ,aname, , , )
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
#define SIGDEF(aname, ... ) idx_##aname,
  enum {
    idx_sig_start=idxSIG-1,
#include "MandalaVars.h"
    idx_sig_top
  };


  // variable definitions: vartype VARNAME;
#define VARDEF(atype,aname,aspan,abytes,adescr)               atype aname;
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr)        atype aname [ asize ];
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) VARDEFA(atype,cfg_##aname,asize,aspan,abytes,adescr)
#include "MandalaVars.h"


#define SIGDEF(aname, ... )   uint8_t aname [ maxVars+1 ];
#include "MandalaVars.h"


  const char      *mode_names[fmCnt];
  const char      *mode_descr[fmCnt];

  const char      *reg_names[regCnt];
  const char      *reg_descr[regCnt];


//=============================================================================
  Mandala();
private:
  uint archiveValue(uint8_t *ptr,uint i,double v);
  double extractValue(const uint8_t *ptr,uint i);
public:
  uint extractMandala(const uint8_t *buf,const uint8_t *signature);
  uint extractVar(const uint8_t *buf,uint var_idx);
  uint archiveMandala(uint8_t *buf,const uint8_t *signature);
  uint archiveVar(uint8_t *buf,uint var_idx);
  uint archiveSize(const uint8_t *signature);
  uint size(void);          // size (bytes) of all archived mandala vars
  void setSignature(uint i,const uint8_t *signature);
  bool checkCommand(const uint8_t *data,uint cnt);
  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void dump(const Vect &v,const char *str="");
  void print_report(void);

  //some special protocols
  uint archiveFlightPlan(uint8_t *buf,uint bufSize); //pack wypoints to buf, return size
  void extractFlightPlan(uint8_t *buf,uint bufSize); //read packed waypoints from buf

  // math operations
  double boundAngle(double v,double span=180.0);
  Vect boundAngle(const Vect &v,double span=180.0);
  uint snap(uint v, uint snapv=10);
  void filterValue(double v,double *vLast,double S,double L);
  double hyst(double err,double hyst);
  double limit(double v,double vL=1.0);
  double limit(double v,double vMin,double vMax);

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
};
//=============================================================================
#endif // MANDALA_H
