//=============================================================
#ifndef tcplink_H
#define tcplink_H
//=============================================================
#include <inttypes.h>
#include <termios.h>
#include <netinet/in.h>
//=============================================================
class Tcplink
{
public:
  Tcplink(const char *name="TCP");
  ~Tcplink();
  const char    *name;

  bool connect(const char *host,uint port);
  void close();

  bool write(const uint8_t *buf,uint cnt);
  uint read(uint8_t *buf,uint sz);

protected:
  int  fd;
  bool err_mute;
  struct  sockaddr_in host_addr;
  
  bool reconnect(bool silent=false);

  bool write_raw(const uint8_t *buf,uint cnt);
  uint read_raw(uint8_t *buf,uint sz);

  //line read
  uint line_cnt;
  char line_buf[256];
  uint16_t packet_sz;
  uint16_t packet_crc16;
  bool readline(void);

};
//=============================================================
#endif
