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
typedef unsigned int uint;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed long  int32_t;

//-----------------------------------------------------------------------------
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
// enum indexes idx_VARNAME
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) VARDEF(atype,aname,aspan,abytes,adescr)
#define VARDEF(atype,aname,aspan,abytes,adescr) idx_##aname,
enum {
  idx_vars_start=-1,
  #include "MandalaVars.h"
  idx_vars_top
};
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
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
#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  enum{ var_bytes_##aname=(aspan<0)?(-abytes):(abytes) }; \
  enum{ var_array_##aname=asize }; \
  enum{ var_max_##aname=(aspan>0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))): \
  ( (aspan<0)?((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0))):0 ) }; \
  static const float var_span_##aname=(aspan<0)?(-aspan):(aspan);
#include "MandalaVars.h"

/*#define VARDEF(atype,aname,aspan,abytes,adescr) vt_##atype,
static const _var_type var_type[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) aspan,
static const float var_span[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) abytes,
static const uint var_bytes[]={
  #include "MandalaVars.h"
};
#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) asize,
static const uint var_array[]={
  #include "MandalaVars.h"
};

#define VARDEF(atype,aname,aspan,abytes,adescr) static const float var_span_##aname=aspan;
#include "MandalaVars.h"
*/
//-----------------------------------------------------------------------------
// special signature vars..
#define SIGDEF(aname, adescr, ... )   static const uint8_t aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
extern uint archive_size(const uint8_t *signature);
extern uint archive_sig(uint8_t *buf,const uint8_t *signature);
//-----------------------------------------------------------------------------
#define archive_f(abuf,avalue,aname) archive_f_impl(abuf,avalue,var_bytes_##aname,var_span_##aname,var_max_##aname)
extern uint archive_f_impl(uint8_t *buf,float v,const int sbytes,const float span,const uint32_t max);
//-----------------------------------------------------------------------------
extern uint archive_var(uint8_t *buf,uint var_idx);
//=============================================================================
#endif
