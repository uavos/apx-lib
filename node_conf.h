#ifndef _NODE_CONF_H_
#define _NODE_CONF_H_
#include "node.h"
//=============================================================================
// NODE LOCAL CONF
//-----------------------------------------------------------------------------
typedef struct {
  #define NCMD(...)
  #define NCONF(atype,aname,adescr,...) _ft_##atype aname;
  #define NCONFA(atype,aname,asize,adescr,...) NCONF(atype,aname[asize],adescr, __VA_ARGS__ )
  #include <config_default.h>
  #ifndef LOADER
  #include <config.h>
  #endif
  #undef NCONF
  #undef NCONFA
  #undef NCMD
}__attribute__((packed)) _node_conf;
//-----------------------------------------------------------------------------
//node user commands
enum{
  apc_sys_dummy=apc_user-1,
#ifndef LOADER
#define NCONF(...)
#define NCONFA(...)
#define NCMD(aname,adescr) apc_##aname,
#include <config.h>
#undef NCONF
#undef NCONFA
#undef NCMD
#endif
};
extern uint8_t conf_error;
extern _node_conf conf;
//=============================================================================
#endif

