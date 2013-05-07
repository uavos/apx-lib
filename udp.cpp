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
  if (fd<=0) printf("%s: Open Socket Failed.\n",name);
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

  if (::bind(fd,(const sockaddr*)&bind_addr,sizeof(bind_addr))<0){
    printf("%s: Bind Failed.\n",name);
    ::close(fd);
    return false;
  }
  return true;
}
//==============================================================================
void Udp::close()
{
  if(fd)::close(fd);
}
//==============================================================================
void Udp::write(const uint8_t *buf,uint cnt,const char *host,uint port)
{
  if(!fd)return;
  dest_addr.sin_addr.s_addr = inet_addr(host);
  dest_addr.sin_port        = port?htons(port):bind_addr.sin_port;

  if (sendto(fd,buf,cnt,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr))!=(int)cnt){
    if(!err_mute)printf("%s: Sending Packet Failed.\n",name);
    err_mute=true;
  }
}
//==============================================================================
uint Udp::read(uint8_t *buf,uint sz)
{
  if(!fd){
    usleep(100000);
    return 0;
  }
  int addr_size = sizeof(sender_addr);
  int cnt=recvfrom(fd,buf,sz,0,(sockaddr*)&sender_addr,(socklen_t *)&addr_size);
  if(cnt<0){
    if(!err_mute)printf("%s: Reading Packet Failed.\n",name);
    err_mute=true;
    return 0;
  }
  return cnt;
}
//==============================================================================
//==============================================================================
