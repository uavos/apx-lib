//=============================================================
#ifndef tcp_server_H
#define tcp_server_H
//=============================================================
#include "tcp_client.h"
//=============================================================
class _tcp_server : public _tcp_client
{
public:
  _tcp_server(const char *name="server");
  ~_tcp_server();
protected:
  bool connect_task();
private:
  int server_fd;
};
//=============================================================
#endif
