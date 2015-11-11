#ifndef time_ms_H
#define time_ms_H
//#include <iostream>
//==============================================================================
#if (! defined _TIME_H_) && (! defined _TIME_H)

#ifdef CHIBIOS
#include "ch.h"
#define time    ((uint) chVTGetSystemTimeX()/MS2ST(1) )
#define time_task() (void)

#else //_HARDWARE_H_

#ifndef _HARDWARE_H_
#include <inttypes.h>
#include <sys/types.h>
extern uint time;
extern "C" unsigned int time_task(void);
#endif //_HARDWARE_H_

#endif //CHIBIOS

#else

#define time getSysTimeMS()
#include <sys/time.h>
#include <inttypes.h>
#include <sys/types.h>
static uint getSysTimeMS()
{
  struct timeval  tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec)*1000+(tv.tv_usec)/1000;
}

#ifndef time_task
#define time_task() (void)
#endif

#endif
//==============================================================================
#endif
