//=============================================================
#ifndef tcp_client_H
#define tcp_client_H
//=============================================================
#include <inttypes.h>
#include <termios.h>
#include <netinet/in.h>
//=============================================================
class _tcp_client
{
public:
  _tcp_client(const char *name="tcp");
  ~_tcp_client();

  bool connect(const char *host,uint port,bool block=false,const char *path="/datalink");
  void close();

  bool write(const uint8_t *buf,uint cnt);
  uint read(uint8_t *buf,uint sz);

  bool is_connected(void);

  bool silent;
  bool tcpdebug;
  
protected:
  const char *name;
  int  fd;
  bool err_mute;
  struct{
    sockaddr_in addr;
    const char *path;
    bool stream;
    char server[256];
  }host;


  virtual bool connect_task();
  uint init_stage;
  time_t time_s;
  uint bytes_available(uint8_t *buf,uint sz);

  //line read
  uint line_cnt;
  char line_buf[256];
  uint16_t packet_sz;
  uint16_t packet_crc16;
  bool readline(void);

};
//=============================================================
#endif
