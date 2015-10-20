//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <arpa/inet.h>
#include "udp.h"
//==============================================================================
Udp::Udp(const char *name)
  : name(name),err_mute(false)
{
  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(fd<=0) printf("[%s]Error: Open Socket Failed.\n",name);
  memset(&bind_addr,0,sizeof(bind_addr));
  memset(&dest_addr,0,sizeof(dest_addr));
  memset(&sender_addr,0,sizeof(sender_addr));
  bind_addr.sin_family=AF_INET;
  dest_addr.sin_family=AF_INET;
}
Udp::~Udp()
{
  close();
}
//==============================================================================
bool Udp::bind(const char *host,uint port)
{
  bind_addr.sin_addr.s_addr = inet_addr(host);
  bind_addr.sin_port        = htons(port);

  if(::bind(fd,(const sockaddr *)&bind_addr,sizeof(bind_addr))<0) {
    printf("[%s]Error: UDP Bind Failed.\n",name);
    ::close(fd);
    fd=-1;
    return false;
  }
  return true;
}
//==============================================================================
void Udp::close()
{
  if(fd>0)::close(fd);
}
//==============================================================================
void Udp::write(const uint8_t *buf,uint cnt,const char *host,uint port)
{
  if(fd<=0)return;
  dest_addr.sin_addr.s_addr = inet_addr(host);
  dest_addr.sin_port        = port?htons(port):bind_addr.sin_port;

  if(sendto(fd,buf,cnt,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr))!=(int)cnt) {
    if(!err_mute)printf("[%s]Error: Sending Packet Failed.\n",name);
    err_mute=true;
  }
}
//==============================================================================
uint Udp::read(uint8_t *buf,uint sz)
{
  if(fd<=0) {
    usleep(100000);
    return 0;
  }
  int cnt = 0;
  int addr_size = sizeof(sender_addr);

  fd_set readfds;
  struct timeval receive_timeout;
  // clear the set ahead of time
  FD_ZERO(&readfds);
  // add our descriptors to the set
  FD_SET(fd, &readfds);
  receive_timeout.tv_sec = 1;
  receive_timeout.tv_usec = 0;
  int rv = select(fd+1, &readfds, NULL, NULL, &receive_timeout);
  if(rv == -1) {
    //perror("select\n"); // error occurred in select()
    cnt=0;
  } else if(rv == 0) {
    //printf("UDP:Timeout occurred.\n");
    cnt=0;
  } else {
    // one or both of the descriptors have data
    if(FD_ISSET(fd, &readfds)) {
      cnt=recvfrom(fd,buf,sz,0,(sockaddr *)&sender_addr,(socklen_t *)&addr_size);
      if(cnt<0) {
        if(!err_mute)printf("[%s]Error: Reading Packet Failed.\n",name);
        err_mute=true;
        return 0;
      }
    }

  }
  return cnt;
}
//==============================================================================
//==============================================================================
Tcp::Tcp(const char *name)
  : name(name),err_mute(false)
{
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd<=0) printf("[%s]Error: Open Socket Failed.\n",name);
  memset(&bind_addr,0,sizeof(bind_addr));
  memset(&dest_addr,0,sizeof(dest_addr));
  memset(&sender_addr,0,sizeof(sender_addr));
  bind_addr.sin_family=AF_INET;
  dest_addr.sin_family=AF_INET;
}
Tcp::~Tcp()
{
  close();
}
//==============================================================================
bool Tcp::connect(const char *host,uint port)
{
  bind_addr.sin_addr.s_addr = inet_addr(host);
  bind_addr.sin_port        = htons(port);

  if(::connect(fd,(const sockaddr *)&bind_addr,sizeof(bind_addr))<0) {
    printf("[%s]Error: TCP Connect Failed.\n",name);
    ::close(fd);
    fd=-1;
    return false;
  }
  const char req[]="GET /datalink HTTP/1.0\r\n";
  write((const uint8_t *)req,strlen(req));
  if(fd<0)return false;
  uint8_t buf[256];
  memset(buf,0,sizeof(buf));

  /*while(1){
    int rcnt=::readln(fd,buf,sizeof(buf));
  }*/
  return true;
}
//==============================================================================
void Tcp::close()
{
  if(fd>0)::close(fd);
}
//==============================================================================
void Tcp::write(const uint8_t *buf,uint cnt)
{
  if(fd<=0)return;
  int sent = 0;
  while(sent<cnt) {
    int wcnt=::send(fd,buf+sent,cnt-sent,0);
    if(wcnt==-1) {
      if(!err_mute)printf("[%s]Error: Sending Packet Failed.\n",name);
      err_mute=true;
      close();
      return;
    }
    sent+=wcnt;
  }
}
//==============================================================================
uint Tcp::read(uint8_t *buf,uint sz)
{
  if(fd<=0) {
    usleep(100000);
    return 0;
  }
  int cnt = 0;
  int addr_size = sizeof(sender_addr);

  fd_set readfds;
  struct timeval receive_timeout;
  // clear the set ahead of time
  FD_ZERO(&readfds);
  // add our descriptors to the set
  FD_SET(fd, &readfds);
  receive_timeout.tv_sec = 1;
  receive_timeout.tv_usec = 0;
  int rv = select(fd+1, &readfds, NULL, NULL, &receive_timeout);
  if(rv == -1) {
    //perror("select\n"); // error occurred in select()
    cnt=0;
  } else if(rv == 0) {
    //printf("UDP:Timeout occurred.\n");
    cnt=0;
  } else {
    // one or both of the descriptors have data
    if(FD_ISSET(fd, &readfds)) {
      cnt=recvfrom(fd,buf,sz,0,(sockaddr *)&sender_addr,(socklen_t *)&addr_size);
      if(cnt<0) {
        if(!err_mute)printf("[%s]Error: Reading Packet Failed.\n",name);
        err_mute=true;
        return 0;
      }
    }

  }
  return cnt;
}
//==============================================================================
int Tcp::readline(char **out)
{
  /*int buf_size = 128;
  int bytesloaded = 0;
  int ret;
  char buf;
  char *buffer = (char*)malloc(buf_size);
  char *newbuf;

  if(NULL == buffer)
    return -1;

  while(1) {
    // read a single byte
    ret = ::read(fd, &buf, 1);
    if(ret < 1) {
      // error or disconnect
      free(buffer);
      return -1;
    }

    buffer[bytesloaded] = buf;
    bytesloaded++;

    // has end of line been reached?
    if(buf == '\n')
      break; // yes

    // is more memory needed?
    if(bytesloaded >= buf_size) {
      buf_size += 128;
      newbuf = (char*)realloc(buffer, buf_size);

      if(NULL == newbuf) {
        free(buffer);
        return -1;
      }

      buffer = newbuf;
    }
  }

  // if the line was terminated by "\r\n", ignore the
  // "\r". the "\n" is not in the buffer

  if((bytesloaded) && (buffer[bytesloaded-1] == '\r'))
    bytesloaded--;
  *out = buffer; // complete line
  return bytesloaded; // number of bytes in the line, not counting the line break*/
}
//==============================================================================
//==============================================================================
