//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <fcntl.h>
#include "tcplink.h"
#include "crc.h"
//==============================================================================
Tcplink::Tcplink(const char *name)
  : name(name),err_mute(false)
{
  memset(&host_addr,0,sizeof(host_addr));
}
Tcplink::~Tcplink()
{
  close();
}
//==============================================================================
bool Tcplink::connect(const char *host,uint port)
{
  host_addr.sin_family=AF_INET;
  host_addr.sin_addr.s_addr = inet_addr(host);
  host_addr.sin_port        = htons(port);
  return reconnect();
}
//==============================================================================
bool Tcplink::reconnect(bool silent)
{
  fd = socket(AF_INET, SOCK_STREAM,0); //|SOCK_NONBLOCK
  if(fd<=0){
    printf("[%s]Error: Open Socket Failed.\n",name);
    return false;
  }
  if(::connect(fd,(const sockaddr *)&host_addr,sizeof(host_addr))<0) {
    if(!silent)printf("[%s]Error: TCP Connect Failed.\n",name);
    ::close(fd);
    fd=-1;
    return false;
  }
  while(1){
    const char req[]="GET /datalink HTTP/1.0\r\n";
    if(!::write(fd,(const uint8_t *)req,strlen(req)))break;

    bool bOK=false,bType=false,bErrHdr=false;
    while(readline()){
      bErrHdr=true;
      if(!bOK){
        if(strstr(line_buf,"200 OK")) bOK=true;
        else break;
        continue;
      }
      if((!bType) && strstr(line_buf,"application/octet-stream")) bType=true;
      else if(strstr(line_buf,"Server: "));
      else if(line_cnt==2){
        bErrHdr=false;
        //end of header
        if(!(bOK&&bType))break;
        packet_sz=0;
        return true;
      }else if(!silent)printf("[%s]: %s",name,line_buf);
    }
    if(silent)break;
    if(bErrHdr) printf("[%s]: %s",name,line_buf);
    printf("[%s]: Response header error.\n",name);
    break;
  }
  close();
  return false;
}
//==============================================================================
void Tcplink::close()
{
  if(fd>0)::close(fd);
  fd=-1;
}
//==============================================================================
bool Tcplink::write_raw(const uint8_t *buf,uint cnt)
{
  if(fd<=0)return false;
  int sent = 0;
  while(sent<cnt) {
    int wcnt=::send(fd,buf+sent,cnt-sent,0);
    if(wcnt==-1) {
      if(!err_mute)printf("[%s]Error: Sending Packet Failed.\n",name);
      err_mute=true;
      close();
      return false;
    }
    sent+=wcnt;
  }
  return true;
}
//==============================================================================
uint Tcplink::read_raw(uint8_t *buf,uint sz)
{
  if(fd<=0) {
    usleep(100000);
    return 0;
  }
  int cnt = 0;
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
      cnt=recv(fd,buf,sz,0);
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
bool Tcplink::readline(void)
{
  ssize_t n, rc;
  char    c, *ptr;
  ptr = line_buf;
  line_cnt=0;
  for(n = 1; n < sizeof(line_buf); n++) {
    again:
    if((rc = ::read(fd,&c,1)) == 1) {
      *ptr++ = c;
      if(c  == '\n') break; // newline is stored, like fgets()
    } else if(rc == 0) {
      *ptr = 0;
      line_cnt=n-1;// EOF, n - 1 bytes were read
      return line_cnt;
    } else{
      if (errno == EINTR)
        goto again;
      return false; // error, errno set by read()
    }
  }
  *ptr  = 0; // null terminate like fgets()
  line_cnt=n;
  return line_cnt;
}
//==============================================================================
//==============================================================================
bool Tcplink::write(const uint8_t *buf,uint cnt)
{
  uint16_t sz=cnt;
  uint16_t crc16=CRC_16_IBM(buf,cnt,0xFFFF);
  while(1){
    if(!write_raw((uint8_t*)&sz,sizeof(sz)))break;
    if(!write_raw((uint8_t*)&crc16,sizeof(crc16)))break;
    if(!write_raw(buf,cnt))break;
    return true;
  }
  close();
  return false;
}
//==============================================================================
uint Tcplink::read(uint8_t *buf,uint sz)
{
  if(fd<=0) {
    reconnect(true);
    usleep(100000);
    return 0;
  }
  // use the poll system call to be notified about socket status changes
  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN | POLLHUP | POLLRDNORM;
  pfd.revents = 0;
  if(poll(&pfd,1,0)>0){
    // if result > 0, this means that there is either data available on the
    // socket, or the socket has been closed

  }else return 0;
  /*int bytes_available=0;
  if(ioctl(fd,FIONREAD,&bytes_available)<0){
    close();
    return 0;
  }
  if(bytes_available<0){
    return 0;
  }
  if(bytes_available==0)return 0;*/
  int bytes_available=recv(fd,buf,sz,MSG_PEEK|MSG_DONTWAIT);
  if(bytes_available<=0){
    printf("[%s]Connection closed.\n",name);
    close();
    return 0;
  }
  if(packet_sz==0){
    if(bytes_available<(sizeof(packet_sz)+sizeof(packet_crc16)))return 0;
    ::read(fd,&packet_sz,sizeof(packet_sz));
    ::read(fd,&packet_crc16,sizeof(packet_crc16));
    bytes_available-=(sizeof(packet_sz)+sizeof(packet_crc16));
    if(packet_sz>sz)packet_sz=sz;
  }
  if(bytes_available<packet_sz)return 0;
  sz=packet_sz;
  packet_sz=0;
  int cnt=::read(fd,buf,sz);
  if(cnt<=0 || packet_crc16!=CRC_16_IBM(buf,cnt,0xffff)){
    printf("[%s]Error: Reading Packet Failed (%i/%u).\n",name,cnt,sz);
    close();
    return 0;
  }
  return cnt;
}
//==============================================================================
