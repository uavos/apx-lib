//=============================================================
#ifndef udp_H
#define udp_H
//=============================================================
#include <inttypes.h>
#include <termios.h>
#include <netinet/in.h>
//=============================================================
class Udp
{
public:
  Udp(const char *name="UDP");
  ~Udp();
  const char    *name;
  
  bool bind(const char *host,uint port);
  void close();

  void write(const uint8_t *buf,uint cnt,const char *host,uint port=0);
  uint read(uint8_t *buf,uint sz);
private:
  int   fd;
  struct  sockaddr_in bind_addr;
  struct  sockaddr_in dest_addr;
  struct  sockaddr_in sender_addr;
};
//=============================================================
#endif
