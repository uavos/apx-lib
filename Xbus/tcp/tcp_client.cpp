//==============================================================================
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
//==============================================================================
_tcp_client::_tcp_client(const char *name)
    : silent(false)
    , tcpdebug(false)
    , name(name)
    , err_mute(false)
    , init_stage(0)
{
    memset(&host.addr, 0, sizeof(host.addr));
    host.path = "";
}
_tcp_client::~_tcp_client()
{
    close();
}
//==============================================================================
bool _tcp_client::connect(const char *host, uint port, bool block, const char *path)
{
    this->host.addr.sin_family = AF_INET;
    this->host.addr.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;
    this->host.addr.sin_port = htons(port);
    this->host.path = path;
    init_stage = 1;
    if (block) {
        time_t t0 = time(0);
        while ((time(0) - t0) < 5) {
            usleep(100000);
            connect_task();
            if (is_connected())
                return true;
        }
        printf("[%s]Connect error.\n", name);
        return false;
    }
    return true;
}
//==============================================================================
bool _tcp_client::is_connected(void)
{
    return fd >= 0 && init_stage >= 100;
}
//==============================================================================
void _tcp_client::close()
{
    if (fd >= 0)
        ::close(fd);
    fd = -1;
    init_stage = 20; //reconnect
}
//==============================================================================
uint _tcp_client::readTO(uint8_t *buf, uint sz, uint timeout_sec)
{
    time_t t0 = time(0);
    do {
        uint cnt = read(buf, sz);
        if (cnt)
            return cnt;
        usleep(1000);
    } while ((time(0) - t0) < (time_t) timeout_sec);
    return 0;
}
//==============================================================================
bool _tcp_client::readline(void)
{
    char *ptr = line_buf + line_cnt;
    uint sz = sizeof(line_buf) - line_cnt;
    uint rcnt = bytes_available((uint8_t *) ptr, sz);
    if (!rcnt)
        return false;
    while (rcnt--) {
        ::read(fd, ptr, 1);
        if (*ptr == '\r')
            continue;
        else if (*ptr == '\n') {
            line_cnt = 0;
            *ptr = 0;
            if (tcpdebug)
                printf("[%s]%s\n", name, line_buf);
            return true;
        }
        line_cnt++;
        ptr++;
        sz--;
    }
    if (!sz)
        line_cnt = 0;
    return false;
}
//==============================================================================
//==============================================================================
bool _tcp_client::write(const uint8_t *buf, uint cnt)
{
    if (!cnt)
        return true;
    if (!is_connected()) {
        return tx_fifo.write_packet(buf, cnt);
    }
    uint16_t sz = cnt;
    uint16_t crc16 = apx::crc32(buf, cnt);
    bool bErr = true;
    memcpy(&(tx_packet[0]), &sz, sizeof(sz));
    memcpy(&(tx_packet[sizeof(sz)]), &crc16, sizeof(crc16));
    memcpy(&(tx_packet[sizeof(sz) + sizeof(crc16)]), buf, cnt);
    cnt += sizeof(sz) + sizeof(crc16);
    bErr = ::send(fd, tx_packet, cnt, 0) != (int) cnt;
    if (bErr) {
        printf("[%s]Error: Writing Packet Failed (%i/%u).\n", name, cnt, sz);
        close();
        return false;
    }
    return true;
}
//==============================================================================
uint _tcp_client::read(uint8_t *buf, uint sz)
{
    if (init_stage < 100) {
        connect_task();
        return 0;
    }

    uint fcnt = tx_fifo.read_packet(buf, sz);
    if (fcnt)
        write(buf, fcnt);

    int rcnt = bytes_available(buf, sz);
    if (!rcnt)
        return 0;
    //printf("rx: %u\n",rcnt);
    if (packet_sz == 0) {
        if ((size_t) rcnt < (sizeof(packet_sz) + sizeof(packet_crc32)))
            return 0;
        ::read(fd, &packet_sz, sizeof(packet_sz));
        ::read(fd, &packet_crc32, sizeof(packet_crc32));
        rcnt -= (sizeof(packet_sz) + sizeof(packet_crc32));
        if (packet_sz > sz)
            packet_sz = sz;
    }
    if (rcnt < packet_sz)
        return 0;
    sz = packet_sz;
    packet_sz = 0;
    int cnt = ::read(fd, buf, sz);
    uint32_t crc32 = apx::crc32(buf, cnt);
    if (cnt <= 0 || packet_crc32 != crc32) {
        printf("[%s]Error: Reading Packet Failed (%i/%u).\n", name, cnt, sz);
        close();
        return 0;
    }
    return cnt;
}
//==============================================================================
uint _tcp_client::bytes_available(uint8_t *buf, uint sz)
{
    if (fd < 0) {
        init_stage = 20; //reconnect
        return 0;
    }
    // use the poll system call to be notified about socket status changes
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
    pfd.revents = 0;
    if (::poll(&pfd, 1, 0) > 0) {
        // if result > 0, this means that there is either data available on the
        // socket, or the socket has been closed
    } else
        return 0;
    int rcnt = ::recv(fd, buf, sz, MSG_PEEK);
    if (rcnt <= 0) {
        printf("[%s]Connection closed.\n", name);
        close();
        return 0;
    }
    return rcnt;
}
//==============================================================================
bool _tcp_client::connect_task()
{
    const char *err = nullptr;
    switch (init_stage) {
    case 0:
        return false; //idle
    case 1: {         //connecting
        time_tcp_s = time(0);
#ifdef __APPLE__
        fd = socket(AF_INET, SOCK_STREAM, 0);
#else
        fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#endif
        setNonblocking(fd);
        if (fd <= 0) {
            if (!err_mute)
                printf("[%s]Error: Open Socket Failed.\n", name);
            err_mute = true;
            err = "socket";
            break;
        }
        int status = ::connect(fd, (const sockaddr *) &host.addr, sizeof(host.addr));
        if (status == 0) {
            if (tcpdebug)
                printf("[%s]Connected immediately.\n", name);
            init_stage += 2;
            err_mute = false;
            break;
        }
        if (errno == 0 || errno == EINPROGRESS) {
            if (tcpdebug)
                printf("[%s]Connecting...\n", name);
            err_mute = false;
            init_stage++;
            break;
        }
        if (!err_mute)
            printf("[%s]Error: TCP Connect Failed (%i).\n", name, errno);
        err_mute = true;
        err = "connect";
    } break;
    case 2: { //wait socket connected
        fd_set write_fds;
        FD_ZERO(&write_fds);    //Zero out the file descriptor set
        FD_SET(fd, &write_fds); //Set the current socket file descriptor into the set
        struct timeval tv;      //Time value struct declaration
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        int rv = select(fd + 1, nullptr, &write_fds, nullptr, &tv);
        if (rv <= 0)
            break;
        int result;
        socklen_t len = sizeof(result);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &len) < 0 || result != 0) {
            err = "getsockopt";
            break;
        }
        init_stage++;
        if (tcpdebug)
            printf("[%s]Socket connected.\n", name);
        //set options
        int optval = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
        optval = 1;
#ifndef __APPLE__
        setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif
    } break;
    case 3: { //send request
        if (host.path[0] == 0) {
            if (tcpdebug)
                printf("[%s]Direct pipe.\n", name);
            init_stage = 100;
            return true;
        }
        if (tcpdebug)
            printf("[%s]Requesting...\n", name);
        sprintf(line_buf, "GET %s HTTP/1.0\r\nFrom: %s\r\n\r\n", host.path, name);
        if (::send(fd, (const uint8_t *) line_buf, strlen(line_buf), 0) <= 0) {
            err = "send";
            break;
        }
        line_cnt = 0;
        init_stage++;
        if (tcpdebug)
            printf("[%s]Request sent.\n", name);
        time_tcp_s = time(0);
    } break;
    case 4: { //check response
        if (!readline())
            break;
        if (!strstr(line_buf, "200 OK")) {
            printf("[%s]%s\n", name, line_buf);
            err = "header";
            break;
        }
        host.stream = false;
        host.server[0] = 0;
        init_stage++;
    } break;
    case 5: //header
        if (!readline())
            break;
        if (strstr(line_buf, "application/octet-stream")) {
            host.stream = true;
            break;
        }
        if (strstr(line_buf, "Server: ")) {
            const char *s = strstr(line_buf, "Server: ") + 8;
            strcpy(host.server, s);
            if (strchr(host.server, '.'))
                strchr(host.server, '.')[0] = 0;
            break;
        }
        if (strlen(line_buf) == 0) {
            if (host.stream) {
                if (!silent)
                    printf("[%s]Connected (%s)\n", name, host.server);
                init_stage = 100;
                return true;
            }
            err = "header";
        }
        printf("[%s]%s\n", name, line_buf);
        break;
    case 20:
        //error - reconnect
        if ((time(0) - time_tcp_s) < 1)
            break;
        init_stage = 1;
        break;
    }
    if ((time(0) - time_tcp_s) > 5) {
        err = "timeout";
    }
    if (err) {
        if (tcpdebug && err[0])
            printf("[%s]Error (%s).\n", name, err);
        ::close(fd);
        fd = -1;
        init_stage = 20;
        time_tcp_s = time(0);
    }
    return false;
}
//==============================================================================
int _tcp_client::setNonblocking(int fd)
{
    int flags;
    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}
//==============================================================================
