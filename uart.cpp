//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <time.h>
#include "uart.h"
//==============================================================================
#define DEFAULT_PORTNAME   "/dev/ttyS0"
#define DEFAULT_BAUDRATE   B115200
//==============================================================================
Uart::Uart()
{
  fd=-1;
  pname=DEFAULT_PORTNAME;
  brate=DEFAULT_BAUDRATE;
}
Uart::~Uart()
{
  close();
}
//==============================================================================
bool Uart::open(const char *portname,int baudrate,const char *name,int timeout,uint parity)
{
  if(!portname[0])portname=pname;
  if(!name[0])name=portname;
  if(!baudrate)baudrate=brate;
  this->name=name;

  fd = ::open(portname, O_RDWR | O_NOCTTY );// | O_NONBLOCK | O_NDELAY);
  if (fd <0) {
    fprintf(stderr,"%s: unable to open %s - %s\n",name,portname,strerror(errno));
    return false;
  }

  //fcntl(fd, F_SETFL, FNDELAY);

  //fcntl(fd, F_SETFL, FASYNC);
  //fcntl(fd, F_SETOWN, getpid());
  //fcntl(fd, F_SETFL, O_NONBLOCK);
  struct termios tio_serial;
  bzero(&tio_serial, sizeof(tio_serial));
  tio_serial.c_cflag = CS8 | CLOCAL | CREAD | parity;
  tio_serial.c_iflag = IGNBRK | IGNPAR;//(parity?INPCK:IGNPAR);
  tio_serial.c_oflag = 1;
  tio_serial.c_lflag = 0;
  tio_serial.c_cc[VMIN] = 0;
  tio_serial.c_cc[VTIME] = timeout;
  cfsetspeed(&tio_serial, baudrate);
  //cfsetispeed(&tio_serial, baudrate);
  //cfsetospeed(&tio_serial, baudrate);
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &tio_serial);

  brate=baudrate;
  pname=portname;

  return true;
}
//==============================================================================
void Uart::close()
{
  ::close(fd);
}
//==============================================================================
bool Uart::isOpen(void)
{
  return fd>=0;
}
//==============================================================================
int Uart::handle(void)
{
  return fd;
}
//==============================================================================
//==============================================================================
void Uart::write(const uint8_t v)
{
  if(::write(fd,&v,1))return;
}
//==============================================================================
void Uart::write(const uint8_t *buf,uint cnt)
{
  unsigned short i=0;
  while (i<cnt) {
    i+=::write(fd,buf+i,cnt-i);
  }
}
//==============================================================================
uint8_t Uart::getCRC(const uint8_t *buf,uint cnt)
{
  uint crc=0;
  while (cnt--)crc += *buf++;
  return crc&0x00FF;
}
//==============================================================================
unsigned int Uart::getRxCnt(void)
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
unsigned int Uart::getTxCnt(void)
{
  unsigned int cnt;
  ioctl(fd, TIOCOUTQ, &cnt);
  return cnt;
}
//==============================================================================
uint Uart::isBusy(void)
{
  if (unsigned short v=getTxCnt()) {
    printf("[%s]Tx busy: %u\n",name,v);
    return v;
  }
  return 0;
}
//==============================================================================
void Uart::flush(void)
{
  tcflush(fd,TCIOFLUSH);
  tcdrain(fd);
}
//==============================================================================
void Uart::writeEscaped(const uint8_t *tbuf,uint dcnt)
{
  uint8_t *buf=txBuf,v;
  uint max=sizeof(txBuf)-6,bcnt=0;
  buf[bcnt++]=0x55;
  buf[bcnt++]=0x01;
  for (uint i=0;i<dcnt;i++) {
    v=tbuf[i];
    buf[bcnt++]=v;
    if (v==0x55)buf[bcnt++]=0x02;
    if(bcnt>=max)return;
  }
  v=getCRC(tbuf,dcnt);
  buf[bcnt++]=v;
  if (v==0x55)buf[bcnt++]=0x02;
  buf[bcnt++]=0x55;
  buf[bcnt++]=0x03;
  write(buf,bcnt);
}
//==============================================================================
uint Uart::readEscaped(uint8_t *buf,uint max_len)
// 0x55..0x01..DATA(0x55.0x02)..CRC..0x55..0x03
{
  uint cnt=0,stage=0,bcnt=0,crc=0;
  unsigned char v,*ptr=buf;
  while (1)//GetRxCnt())
  {
    if (!read(&v,1)) {
      if (bcnt) {
        printf("Received %u bytes.\n",bcnt);
        //dump(buf,bcnt);
      }//else usleep(10000);
      //printf("nc\n");
      return 0;
    }
    //printf("rd %.2X.\n",v);
    bcnt++;
    switch (stage) {
      case 0:
        if (v==0x55)stage=3;
        continue;
      case 1: //data
        if (v==0x55) {
          stage=2;
          continue;
        }
      case_DATA:
          if (++cnt>max_len)break;
          *ptr++=v;
          crc+=v;
          continue;
      case 2: //escape
        if (v==0x02) {
          v=0x55;
          stage=1;
          goto case_DATA;
        }
        if (v==0x03) {
          ptr--;
          crc-=*ptr;
          if ((crc&0x00FF)!=(*ptr))break;
          //frame received...
          cnt--;
          stage=0;
          return cnt;
        }
        if (v==0x55) {
          stage=3;
          continue;
        }
        //fall to case below..
      case 3: // start..
        if (v==0x01) {
          stage=1;
          ptr=buf;
          crc=0;
          cnt=0;
          continue;
        }
        break;
    }
    //error
    //printf("UART read error.");
    stage=0;
  }
  return 0;
}
//==============================================================================
uint Uart::read(uint8_t *buf,uint cnt)
{
  int rcnt=::read(fd,buf,cnt);
  if(rcnt<0)return 0;
  return rcnt;
}
//==============================================================================
uint8_t Uart::read_char(void)
{
  uint8_t v;
  while(!this->read(&v,1));
  return v;
}
//==============================================================================
//==============================================================================
