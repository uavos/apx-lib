//=============================================================================
// default vars list to use all mandala variables
#define VARDEF(atype,aname,aspan,abytes,adescr)         USEVAR( aname )
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr)  USEVAR( aname )
#define CFGDEF(atype,aname,aspan,abytes,around,adescr) VARDEF(atype,cfg_##aname,aspan,abytes,adescr)
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) VARDEFA(atype,cfg_##aname,asize,aspan,abytes,adescr)
#define SIGDEF(aname,adescr,...)                        USESIG( aname )
#include "MandalaVars.h"
//=============================================================================
