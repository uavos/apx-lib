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
  Tcplink(const char *name="UDP");
  ~Tcplink();
  const char    *name;

  bool connect(const char *host,uint port);
  void close();

  bool writePacket(const uint8_t *buf,uint cnt);
  uint readPacket(uint8_t *buf,uint sz);

private:
  int   fd;
  bool err_mute;
  struct  sockaddr_in bind_addr;
  struct  sockaddr_in dest_addr;
  struct  sockaddr_in sender_addr;

  bool write(const uint8_t *buf,uint cnt);
  uint read(uint8_t *buf,uint sz);

  //line read
  uint line_cnt;
  char line_buf[256];
  uint16_t packet_sz;
  uint16_t packet_crc16;
  bool readline(void);

};
//=============================================================
#endif
