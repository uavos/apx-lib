
#include "tcp_client.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#include <crc.h>

#ifndef MSG_MORE
#define MSG_MORE 0
#endif

using namespace xbus::tcp;

Client::Client(const char *name)
    : silent(false)
    , tcpdebug(false)
    , name(name)
    , err_mute(false)
{
    memset(&_host.addr, 0, sizeof(_host.addr));
    _host.path = "";
}
Client::~Client()
{
    close();
}

void Client::set_host(const char *host, uint port, const char *path)
{
    _host.addr.sin_family = AF_INET;
    _host.addr.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;
    _host.addr.sin_port = htons(port);
    _host.path = path;
    printf("%s:connecting to %s:%u%s\n", name, inet_ntoa(_host.addr.sin_addr), ntohs(_host.addr.sin_port), _host.path);
}

bool Client::connect()
{
    const char *err = nullptr;
    do {
        _client_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (_client_fd <= 0) {
            if (!err_mute)
                printf("%s:error:Open Socket Failed\n", name);
            err = "open";
            break;
        }

        if (tcpdebug)
            printf("%s:connecting...\n", name);

        int status = ::connect(_client_fd, (const sockaddr *) &_host.addr, sizeof(_host.addr));
        if (status == 0) {
            if (tcpdebug)
                printf("%s:connected\n", name);
        } else {
            err = "connect";
            break;
        }

        // read timeout
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(_client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);

        //set options
        int optval = 1;
        setsockopt(_client_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#ifndef __APPLE__
        setsockopt(_fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif

        //send request
        if (_host.path[0] == 0) {
            if (tcpdebug)
                printf("%s:direct pipe\n", name);
            return true;
        }

        if (tcpdebug)
            printf("%s:requesting '%s'...\n", name, _host.path);

        char line_buf[256];
        sprintf(line_buf, "GET %s HTTP/1.0\r\nFrom: %s\r\n\r\n", _host.path, name);
        if (::send(_client_fd, (const uint8_t *) line_buf, strlen(line_buf), 0) <= 0) {
            err = "send";
            break;
        }
        if (tcpdebug)
            printf("%s:request sent\n", name);

        if (!readline(_client_fd, line_buf, sizeof(line_buf)))
            break;
        if (!strstr(line_buf, "200 OK")) {
            printf("%s:header:%s\n", name, line_buf);
            err = "header";
            break;
        }
        _host.stream = false;
        _host.server[0] = 0;

        //read response header
        for (;;) {
            if (!readline(_client_fd, line_buf, sizeof(line_buf)))
                break;
            if (strstr(line_buf, "application/octet-stream")) {
                _host.stream = true;
                continue;
            }
            if (strstr(line_buf, "Server: ")) {
                const char *s = strstr(line_buf, "Server: ") + 8;
                strcpy(_host.server, s);
                if (strchr(_host.server, '.'))
                    strchr(_host.server, '.')[0] = 0;
                continue;
            }
            if (strlen(line_buf) == 0) {
                if (_host.stream) {
                    if (!silent) {
                        printf("%s:connected %s@%s:%u%s\n", name, _host.server, inet_ntoa(_host.addr.sin_addr), ntohs(_host.addr.sin_port), _host.path);
                    }
                    return true;
                }
                err = "header";
            }
            printf("%s:%s\n", name, line_buf);
        }
    } while (0);

    if (tcpdebug && err && err[0])
        printf("%s:Error (%s)\n", name, err);

    close();
    return false;
}

bool Client::is_connected(void)
{
    return _client_fd >= 0 && _host.stream;
}

void Client::close()
{
    if (_client_fd >= 0)
        ::close(_client_fd);
    _client_fd = -1;
}

bool Client::readline(int fd, char *line_buf, size_t max_size)
{
    char *ptr = line_buf;
    size_t rcnt = max_size - 1;
    while (rcnt--) {
        ssize_t rv = ::recv(fd, ptr, 1, 0);
        if (rv <= 0)
            return false;

        if (*ptr == '\r')
            continue;
        else if (*ptr == '\n') {
            *ptr = 0;
            if (tcpdebug)
                printf("%s:rx:%s\n", name, line_buf);
            return true;
        }
        ptr++;
    }
    return false;
}

size_t Client::read_packet(void *buf, size_t size)
{
    ssize_t cnt = read_packet(_client_fd, buf, size);
    if (cnt >= 0)
        return cnt;
    close();
    return 0;
}
ssize_t Client::read_packet(int fd, void *buf, size_t size)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
    pfd.revents = 0;
    do {
        if (::poll(&pfd, 1, 1000) <= 0) {
            ssize_t rv = ::recv(fd, buf, size, MSG_PEEK);
            if (rv == 0)
                break;
            if (rv < 0) {
                rv = errno;
                if (rv != EAGAIN && rv != EWOULDBLOCK) {
                    printf("tcp:rx: %li\n", rv);
                    break;
                }
                return 0; // timeout, no data
            }
        }

        uint16_t packet_sz;
        uint32_t packet_crc32;

        if (::recv(fd, &packet_sz, sizeof(packet_sz), MSG_WAITALL) != sizeof(packet_sz)) {
            printf("tcp:error: packet_sz (%i)\n", errno);
            break;
        }
        if (packet_sz == 0 || packet_sz > size) {
            printf("tcp:error: packet size (%u)\n", packet_sz);
            break;
        }

        if (::recv(fd, &packet_crc32, sizeof(packet_crc32), MSG_WAITALL) != sizeof(packet_crc32)) {
            printf("tcp:error: packet_crc32 (%i)\n", errno);
            break;
        }

        if (::recv(fd, buf, packet_sz, MSG_WAITALL) != packet_sz) {
            printf("tcp:error: packet (%i)\n", errno);
            break;
        }

        uint32_t crc32 = apx::crc32(buf, packet_sz);
        if (packet_crc32 != crc32) {
            printf("tcp:error: CRC (%u)\n", packet_sz);
            break;
        }
        return packet_sz;

    } while (0);

    printf("tcp:connection lost\n");
    return -1;
}

bool Client::write_packet(const void *buf, size_t size)
{
    if (!size)
        return true;

    if (!is_connected()) {
        return false;
    }
    if (write_packet(_client_fd, buf, size))
        return true;
    close();
    return false;
}
bool Client::write_packet(int fd, const void *buf, size_t size)
{
    uint16_t packet_sz = size;
    uint32_t packet_crc32 = apx::crc32(buf, size);

    do {
        if (::send(fd, &packet_sz, sizeof(packet_sz), MSG_MORE | MSG_DONTWAIT) != sizeof(packet_sz))
            break;
        if (::send(fd, &packet_crc32, sizeof(packet_crc32), MSG_MORE | MSG_DONTWAIT) != sizeof(packet_crc32))
            break;

        if (::send(fd, buf, size, MSG_DONTWAIT) != (ssize_t) size)
            break;

        return true;
    } while (0);

    printf("tcp:send failed (%d)\n", errno);
    return false;
}

int Client::set_non_blocking()
{
    int flags;
    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(_client_fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(_client_fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(_client_fd, FIOBIO, &flags);
#endif
}
