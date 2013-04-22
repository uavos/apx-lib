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
//#define dump(A,B,C)       {if(C){printf("%s: ",A);for(uint i=0;i<(C);i++)printf("%.2X ",((const uint8_t*)(B))[i]);printf("\n");}}
#define dump(A,B,C)
//==============================================================================
Uart::Uart()
{
  fd=-1;
  pname=DEFAULT_PORTNAME;
  brate=DEFAULT_BAUDRATE;
  esc_state=0;
  esc_crc=0;
  esc_cnt=0;
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

  flush();
  usleep(200000);

  return true;
}
//==============================================================================
void Uart::close()
{
  ::close(fd);
  fd=-1;
}
//==============================================================================
bool Uart::isOpen(void)
{
  return (fd>=0)&&(fcntl(fd, F_GETFL) != -1 || errno != EBADF);
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
  write(&v,1);
}
//==============================================================================
void Uart::write(const uint8_t *buf,uint cnt)
{
  int pcnt=cnt;
  int wcnt=0;
  while (wcnt<pcnt) {
    int rcnt=::write(fd,buf+wcnt,pcnt-wcnt);
    if(rcnt<=0)continue;
    wcnt+=rcnt;
  }
  dump("S",buf,cnt);
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
/*
  fd_set   fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  int rc = select(fd+1,&fds,0,0,NULL);
  if (rc<=0)return 0;
  return rc;*/
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
{
  while(1){
    unsigned char v;
    if (!read(&v,1))return 0;
    switch (esc_state) {
      case 0:
        if (v==0x55)esc_state=3;
        //else break;
        continue;
      case 1: //data
        if (v==0x55) {
          esc_state=2;
          continue;
        }
      case_DATA:
          if (esc_cnt>=sizeof(esc_rx))break;
          esc_rx[esc_cnt++]=v;
          esc_crc+=v;
          continue;
      case 2: //escape
        if (v==0x02) {
          v=0x55;
          esc_state=1;
          goto case_DATA;
        }
        if (v==0x03) {
          if(esc_cnt==0)break; //no data
          esc_cnt--;
          esc_crc-=esc_rx[esc_cnt];
          if ((esc_crc&0xFF)!=esc_rx[esc_cnt])break;
          //frame received...
          esc_state=0;
          if(esc_cnt>max_len)break;
          memcpy(buf,esc_rx,esc_cnt);
          return esc_cnt;
        }
        if (v==0x55) {
          esc_state=3;
          continue;
        }
        //fall to case below..
      case 3: // start..
        if (v==0x01) {
          esc_state=1;
          esc_crc=0;
          esc_cnt=0;
          continue;
        }
        break;
    }
    //error
    esc_state=0;
  }
}
//==============================================================================
uint Uart::read(uint8_t *buf,uint cnt)
{
  int rcnt=::read(fd,buf,cnt);
  if(rcnt<0)return 0;
  //dump("R",buf,rcnt);
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
