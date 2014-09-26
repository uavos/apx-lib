#ifndef time_ms_H
#define time_ms_H
//==============================================================================
#if (! defined _TIME_H_) && (! defined _TIME_H)
#include <inttypes.h>
#include <sys/types.h>
extern uint time;
extern "C" unsigned int time_task(void);
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
#define time_task() (time)
#endif
#endif
//==============================================================================
#endif
