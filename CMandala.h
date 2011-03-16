#ifndef _CMANDALA_H_
#define _CMANDALA_H_
//=============================================================================
/*typedef unsigned int uint;
typedef union{
  float data[3];
  struct{float x,y,z;};
} Vect;
#define double  float
#define vt_float vt_double*/
//-----------------------------------------------------------------------------
#include "MandalaVars.h"

//-----------------------------------------------------------------------------
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
#define MODEDEF(aname,adescr) fm##aname,
enum {
  #include "MandalaVars.h"
  fmCnt
};
#define REGDEF(aname,adescr) reg##aname,
enum {
  #include "MandalaVars.h"
  regCnt
};
/*
//-----------------------------------------------------------------------------
// variable definitions: vartype VARNAME;
#define VARDEF(atype,aname,aspan,abytes,adescr)               atype aname;
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr)        atype aname [ asize ];
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)        VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) VARDEFA(atype,cfg_##aname,asize,aspan,abytes,adescr)
#include "MandalaVars.h"
*/

//-----------------------------------------------------------------------------
//variable parameters
#define VARDEF(atype,aname,aspan,abytes,adescr) vt_##atype,
static const _var_type var_type[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) aspan,
static const float var_span[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) abytes,
static const unsigned int var_bytes[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) asize,
static const unsigned int var_array[]={
  #include "MandalaVars.h"
};

#define VARDEF(atype,aname,aspan,abytes,adescr) static const float var_span_##aname=aspan;
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
// special signature vars..
#define SIGDEF(aname, adescr, ... )   static const unsigned char aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
static unsigned int archiveSize(const unsigned char *signature)
{
  unsigned int scnt=signature[0];
  signature++;
  unsigned int cnt=0;
  while (scnt--) {
    unsigned int i=*signature++;
    if (var_type[i]==vt_Vect)
      cnt+=var_bytes[i]*3*var_array[i];
    else cnt+=var_bytes[i]*var_array[i];
  }
  return cnt;
}
//-----------------------------------------------------------------------------
extern void archiveValue(unsigned char *buf,unsigned int var_idx,unsigned int array_idx,unsigned int vect_idx);
//-----------------------------------------------------------------------------
static unsigned int archiveMandala(unsigned char *buf,const unsigned char *signature)
{
  unsigned int scnt=signature[0];
  signature++;
  unsigned int cnt=0,ai,vi;
  while (scnt--) {
    unsigned int i=*signature++;
    switch (var_type[i]) {
      case vt_uint:
        for (ai=0;ai<var_array[i];ai++){
          archiveValue(buf,i,ai,0);
          buf+=var_bytes[i];
        }
        cnt+=var_bytes[i]*var_array[i];
        break;
      case vt_double:
        for (ai=0;ai<var_array[i];ai++){
          archiveValue(buf,i,ai,0);
          buf+=var_bytes[i];
        }
        cnt+=var_bytes[i]*var_array[i];
        break;
      case vt_Vect:
        for (ai=0;ai<var_array[i];ai++)
          for (vi=0;vi<3;vi++){
            archiveValue(buf,i,ai,vi);
            buf+=var_bytes[i];
          }
          cnt+=var_bytes[i]*3*var_array[i];
        break;
      default:
        break;
    }
  }
  return cnt;
}
//=============================================================================
//=============================================================================
#endif
