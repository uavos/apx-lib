#pragma once

#include "tcp_client.h"

namespace xbus {
namespace tcp {

class Server : public Client
{
public:
    Server(const char *name = "server");
    ~Server();

protected:
    //bool connect_task();

private:
    int server_fd;
};

} // namespace tcp
} // namespace xbus
