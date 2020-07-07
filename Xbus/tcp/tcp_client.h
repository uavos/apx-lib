#pragma once

#include <inttypes.h>
#include <netinet/in.h>
#include <termios.h>

#include <tcp_ports.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

namespace xbus {
namespace tcp {

class Client
{
public:
    Client(const char *name = "tcp");
    ~Client();

    void set_host(const char *host, uint port, const char *path = "/datalink");

    int set_non_blocking();

    bool connect();
    void close();

    size_t read_packet(void *buf, size_t size);
    bool write_packet(const void *buf, size_t size);

    bool is_connected(void);

    bool silent;
    bool tcpdebug;

protected:
    const char *name;
    int fd{-1};
    bool err_mute;
    struct
    {
        sockaddr_in addr;
        const char *path;
        bool stream;
        char server[64];
    } host;

    //line read
    bool readline(char *line_buf, size_t max_size);
};

} // namespace tcp
} // namespace xbus
