//===========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
//------------------------------------------------------------------------------
#include "xsens.h"
#include "Mandala.h"
//==============================================================================
void Xsens::init(const char *portname)
{
  if(!open(portname)){
    printf("WARNING: IMU DISABLED");
    return;
  }
  pthread_create(&thrRx, 0, &Xsens::start_threadRx,this);
}
//==============================================================================
void Xsens::threadRx()
{
  Mandala lvar;
  int msqid = getMsgId();
  _bus_msg msg;
  msg.data[0]=0;        //src
  msg.type=mtypeBusRx;
  while (1) {
    if(!readData())continue;
    for(uint i=0;i<3;i++){
      lvar.acc[i]=data.acc[i];
      lvar.gyro[i]=data.gyro[i]*R2D;
      lvar.mag[i]=data.mag[i];
    }
    lvar.AT=data.temperature;
    msg.data[1]=Mandala::idx_imu;
    msgsnd(msqid,&msg,lvar.archiveVar(&msg.data[2],msg.data[1])+2,0);
    msg.data[1]=Mandala::idx_AT;
    msgsnd(msqid,&msg,lvar.archiveVar(&msg.data[2],msg.data[1])+2,0);
  } // forever
  pthread_exit(NULL);
}
//==============================================================================
typedef struct {  //used to send message only
  uint8_t   preamble; // =0xFA
  uint8_t   busID;    // =0xFF (master)
  uint8_t   msgID;    // message id
  uint8_t   length;   // data length <=254 or 255 if extLen
  uint8_t   data[254];
}__attribute__((packed)) _xsens_msg_hdr;
typedef struct {  //used to send message only
  uint8_t   preamble; // =0xFA
  uint8_t   busID;    // =0xFF (master)
  uint8_t   msgID;    // message id
  uint8_t   length;   // =255
  uint8_t   extLenH;   // ext data length <=2048
  uint8_t   extLenL;
  uint8_t   data[2048];
}__attribute__((packed)) _xsens_msg_hdr_ext;
//==============================================================================
bool Xsens::readMessage(bool wait)
{
  uint8_t *ptr=msg.data,v,crc=0;
  uint stage=0,cnt=0;
  const char *err="";
  //## BIG-ENDIAN! ##
  do {
    if (getRxCnt())
      while (::read(fd,&v,1)>0) {
        //printf("%.2X ",v);fflush(stdout);
        crc+=v;
        switch (stage) {
          case 0: if (v==0xFA)stage++;crc=0;continue; //preamble
          case 1: if ((v==0xFF)||(v==0x01))stage++;else stage=0;continue; //BID
          case 2: stage++;msg.msgID=v;continue;//printf("msg %u from %u\n",v,bID);
          case 3: //length
            if (v==0xFF)stage++; //ext len
            else stage+=1+2;  //std lenght
            msg.msgSize=v;
            cnt=v;
            ptr=msg.data;
            //printf("xsens msg %u\n",msg.msgID);
            continue;
          case 4: msg.msgSize=v<<8;stage++;continue;
          case 5: msg.msgSize|=v;err="length";if (msg.msgSize>2048)break; cnt=msg.msgSize;stage++;continue;
          case 6: //data
            if (cnt--) *ptr++=v;
            else {
              if (crc) { //checksum = 0
                err="crc";
                break;
              }
              stage=0;
              //message received
              return true;
            }
            continue;
        }
        //error
        fprintf(stderr,"xsens: error (%s)\n",err);
        fflush(stderr);
        err="";
        //stage=0;
      }
    //error
    //err="no data";
    //printf("xsens: error (%s)\n",err);
    //err="";
    stage=0;
  } while (wait);
  return false;
}
//==============================================================================
bool Xsens::readData(void)
{
  if (!readMessage())return false;
  if (msg.msgID!=0x32)return false;
  // MTData message received...
  uint32_t *ptr=(uint32_t*)msg.data;
  data.temperature=extractFloat(ptr++);
  data.acc[0]=extractFloat(ptr++);
  data.acc[1]=extractFloat(ptr++);
  data.acc[2]=extractFloat(ptr++);
  data.gyro[0]=extractFloat(ptr++);
  data.gyro[1]=extractFloat(ptr++);
  data.gyro[2]=extractFloat(ptr++);
  data.mag[0]=extractFloat(ptr++);
  data.mag[1]=extractFloat(ptr++);
  data.mag[2]=extractFloat(ptr++);
  return true;
}
//==============================================================================
double Xsens::extractFloat(void* ptr)
{
  float v;
  uint8_t* dest=(uint8_t*)&v;
  uint8_t* src=(uint8_t*)ptr;
  dest[0] = src[3];
  dest[1] = src[2];
  dest[2] = src[1];
  dest[3] = src[0];
  return v;
}
//==============================================================================
Xsens::Xsens()
{
  memset(&msg,0,sizeof(msg));
  memset(&data,0,sizeof(data));
}
Xsens::~Xsens()
{
  if(fd)close(fd);
}
//==============================================================================
bool Xsens::open(const char *portname)
{
  fd = ::open(portname, O_RDWR | O_NOCTTY );// | O_NONBLOCK | O_NDELAY);
  if (fd <0) {
    fprintf(stderr,"unable to open %s - %s\n",portname,strerror(errno));
    return false;
  }

  int baudrate=B115200;
  struct termios tio_serial;
  bzero(&tio_serial, sizeof(tio_serial));
  tio_serial.c_cflag = CS8 | CLOCAL | CREAD;
  tio_serial.c_iflag = IGNBRK | IGNPAR;
  tio_serial.c_oflag = 1;
  tio_serial.c_lflag = 0;
  tio_serial.c_cc[VMIN] = 0;
  tio_serial.c_cc[VTIME] = 10;
  cfsetspeed(&tio_serial,baudrate);
  //cfsetospeed(&tio_serial,baudrate);
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &tio_serial);

  return setup();
}
bool Xsens::setup()
{
  //configure xsens
  static const uint8_t x_config[]={0x30};
  //static const uint8_t x_reset[]={0x40};
  static const uint8_t x_measure[]={0x10};
  //static const uint8_t x_baud[]={0x18,2}; //921600=128 460800=0 230400=1 115200=2
  static const uint8_t x_period[]={0x04,0x04,0x80};  // 100Hz=0x0480 512Hz=225
  static const uint8_t x_skip[]={0xD4,0,0};
  static const uint8_t x_mode[]={0xD0,0x00,0x03};  //TEMP+CALIB
  static const uint8_t x_settings[]={0xD2,0x80,0x00,0x0C,0x04};
  printf("xsens: setup...\n");
  sendMessage(x_config,sizeof(x_config));
  sendMessage(x_config,sizeof(x_config));
  //sendMessage(x_baud,sizeof(x_baud));sendMessage(x_reset,sizeof(x_reset));

  sendMessage(x_mode,sizeof(x_mode));
  sendMessage(x_settings,sizeof(x_settings));

  sendMessage(x_period,sizeof(x_period));
  sendMessage(x_skip,sizeof(x_skip));

  sendMessage(x_measure,sizeof(x_measure));

  printf("xsens: setup finished.\n");
  return true;
}
//==============================================================================
//==============================================================================
bool Xsens::sendMessage(const uint8_t *buf,uint cnt,bool ack)
{
  _xsens_msg_hdr msg;
  memset(&msg,0,sizeof(msg));
  msg.preamble=0xFA;
  msg.busID=0xFF;
  msg.msgID=*buf;
  msg.length=--cnt;
  uint crc=cnt+*buf+0xFF;
  for (uint i=0;i<cnt;i++) {
    msg.data[i]=buf[i+1];
    crc+=buf[i+1];
  }
  msg.data[cnt]=0x100-(crc&0xFF);
  write(fd,(uint8_t*)&msg,cnt+5);
  //printf("crc: %u\n",msg->data[cnt]);
  //var.dump(msgBuf,cnt+5);
  if (!ack)return true;
  //sleep(1);
  while (readMessage(true)) {
    if ((int)this->msg.msgID==(buf[0]+1)) {
      return true;
    }
    //printf("xsens: wrong reply.\n");
  }
  printf("xsens: write message error 0x%.2X (%u)\n",*buf,*buf);
  return false;
}
//==============================================================================
unsigned int Xsens::getRxCnt(void)
{
  unsigned int cnt;
  ioctl(fd, FIONREAD, &cnt);
  return cnt;

  fd_set   fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval  tv;
  tv.tv_usec = 5;
  tv.tv_sec = 0;
  int rc = select(fd+1,&fds,0,0,NULL);
  if (rc<=0)return 0;
  return rc;
}
//==============================================================================
