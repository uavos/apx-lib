#ifndef time_ms_H
#define time_ms_H
//==============================================================================
#ifdef CHIBIOS
#include "ch.h"
#define time    ((uint) chVTGetSystemTimeX()/MS2ST(1) )
#define time_task() (void)

#else

#include <iostream>
#define time getSysTimeMS()
#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>
static inline uint getSysTimeMS()
{
  struct timeval  tv;
  gettimeofday(&tv, nullptr);
  return (tv.tv_sec)*1000+(tv.tv_usec)/1000;
}

#ifndef time_task
#define time_task() (void)
#endif

#endif
//==============================================================================
#endif
