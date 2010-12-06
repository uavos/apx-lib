#ifndef MANDALA_H
#define MANDALA_H
//=============================================================================
//=============================================================================
#include "MandalaTypes.h"
#include "MandalaVars.h"
#include <string.h>
#include <sys/types.h>
//=============================================================================
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
typedef enum {   wtHdg=0,wtLine,wtRunway } _wptType;
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

  const char      *cmd_name[cmdCnt];
  const char      *cmd_alias[cmdCnt];
  const char      *cmd_descr[cmdCnt];
  int             cmd_size[cmdCnt];

  uint  var_void;

  uint8_t command[1024];
  uint commandSize;

  //    ---- Waypoints ----
  struct {
    Vect    NED;
    _wptType type;
    int     param;
    uint8_t command[9];
  } waypoints[100];




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


#define SIGDEF(aname, ... )   static const uint8_t aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ];
#include "MandalaVars.h"


  const char      *mode_names[fmCnt];
  const char      *mode_descr[fmCnt];

  const char      *reg_names[regCnt];
  const char      *reg_descr[regCnt];



  // common signatures
  uint8_t sig_cfg[idx_cfg_top-idxCFG+1];
  uint8_t sig_telemetry[idx_vars_top+1];
  uint size_packed_telemetry;

//=============================================================================
  Mandala();
private:
  uint archiveValue(uint8_t *ptr,uint i,double v);
  double extractValue(const uint8_t *ptr,uint i);
public:
  void extractMandala(const uint8_t *buf,const uint8_t *signature);
  uint archiveMandala(uint8_t *buf,const uint8_t *signature);
  uint archiveSize(const uint8_t *signature);
  uint size(void);
  bool checkCommandBuf(const uint8_t *data,uint cnt);
  void dump(const uint8_t *ptr,uint cnt,bool hex=true);
  void print_report(void);


  // math operations
  double boundAngle(double v,double span=180.0);
  Vect boundAngle(const Vect &v,double span=180.0);
  uint snap(uint v, uint snapv=10);
  void filterValue(double v,double *vLast,double S,double L);
  double hyst(double err,double hyst);
  double limit(double v,double vL=1.0);
  double limit(double v,double vMin,double vMax);

  const Vect llh2ned(const Vect llh);
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
