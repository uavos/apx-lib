#ifndef _CMANDALA_H_
#define _CMANDALA_H_
//=============================================================================
#ifndef INTTYPES
#define INTTYPES
typedef unsigned int uint;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed long  int32_t;
#endif
#ifndef MANDALATYPES
#define MANDALATYPES
typedef double Vect [3];
#endif
//=============================================================================
extern uint archive(uint8_t *buf,uint var_idx);
extern uint extract(uint8_t *buf,uint cnt); //first byte = var_idx
//overload
extern uint archive_sig(uint8_t *buf,const uint8_t *signature);
extern uint extract_var(uint8_t *buf,uint cnt,uint var_idx);
//=============================================================================
//=============================================================================
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
//-----------------------------------------------------------------------------
//variable parameters
#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  var_type_##aname=vt_##atype, \
  var_bytes_##aname=(aspan<0)?(-abytes):(abytes), \
  var_array_##aname=asize, \
  var_size_##aname=((asize)*(abytes)*((vt_##atype==vt_Vect)?3:1)), \
  var_max_##aname=(aspan>0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))): \
  ( (aspan<0)?((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0))):0 ), \

enum{
#include "MandalaVars.h"
};

#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
static const float var_span_##aname=(aspan<0)?(-aspan):(aspan);
#include "MandalaVars.h"

//-----------------------------------------------------------------------------
// variable typedefs
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) typedef atype var_typedef_##aname [asize];
#define VARDEF(atype,aname,aspan,abytes,adescr) typedef atype var_typedef_##aname;
#include "MandalaVars.h"

//-----------------------------------------------------------------------------
// special signature vars..
#define SIGDEF(aname, adescr, ... )   static const uint8_t aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// main structure of all used variables
typedef struct{
  #define USEVAR(aname) var_typedef_##aname aname;
  #include <mandala_vars.h>
  #undef USEVAR
}Mandala;
extern Mandala var;


//=============================================================================
#endif
