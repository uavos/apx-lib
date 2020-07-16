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

    struct host_s
    {
        sockaddr_in addr;
        const char *path;
        bool stream;
        char server[64];
    };

    void set_host(const char *host, uint port, const char *path);
    inline const host_s &host() const { return _host; }

    int set_non_blocking();

    virtual bool connect();
    void close();

    virtual size_t read_packet(void *buf, size_t size);
    virtual bool write_packet(const void *buf, size_t size);

    virtual bool is_connected(void);

    bool silent;
    bool tcpdebug;

private:
    int _client_fd{-1};

protected:
    const char *name;
    bool err_mute;
    host_s _host;

    //line read
    bool readline(int fd, char *line_buf, size_t max_size);

    static ssize_t read_packet(int fd, void *buf, size_t size);
    static bool write_packet(int fd, const void *buf, size_t size);
};

} // namespace tcp
} // namespace xbus
