//==============================================================================
#ifndef xsens_H
#define xsens_H
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>
#include "bus_ipc.h"
//==============================================================================
typedef struct {
  uint      msgID;  //must clear to release buffer
  uint      msgSize;
  uint8_t   data[2048];
}_xsens_message;
//==============================================================================
class Xsens
{
public:
  Xsens();
  ~Xsens();
  void init(const char *portname="/dev/ttyS2");
private:
  bool open(const char *portname);
  bool readData(void);
  struct{
    double acc[3];
    double gyro[3];
    double mag[3];
    double temperature;
  }data;
  int   fd;
  unsigned int getRxCnt(void);
  _xsens_message msg;
  bool setup();
  bool readMessage(bool wait=true);
  bool sendMessage(const uint8_t *buf,uint cnt,bool ack=true);
  double extractFloat(void* ptr);
  //--- thread ----
  pthread_t  thrRx;
  static void *start_threadRx(void *obj) {reinterpret_cast<Xsens*>(obj)->threadRx();pthread_exit(NULL);}
  void threadRx();
  //---------------
};
//==============================================================================
#endif
