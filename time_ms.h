#ifndef time_ms_H
#define time_ms_H
//==============================================================================
#ifndef _TIME_H
extern uint time;
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
#endif
//==============================================================================
#endif
