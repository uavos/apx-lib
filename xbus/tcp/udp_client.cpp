/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "udp_client.h"
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

namespace xbus {
namespace tcp {

udp_client::udp_client(const char *_name, bool silent)
    : _name(_name)
    , _silent(silent)
{
}
udp_client::~udp_client()
{
    close();
}

void udp_client::set_dest(const char *host, uint16_t port)
{
    _dest_addr.sin_family = AF_INET;
    _dest_addr.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;
    _dest_addr.sin_port = htons(port);

    if (!_silent) {
        printf("%s:dest to %s:%u\n", _name, inet_ntoa(_dest_addr.sin_addr), ntohs(_dest_addr.sin_port));
        fflush(stdout);
    }
}

bool udp_client::bind(in_addr_t addr, uint16_t port)
{
    const char *err = nullptr;
    do {
        _fd = ::socket(AF_INET, SOCK_DGRAM, 0);

        if (_fd <= 0) {
            err = "create socket";
            break;
        }

        //set options
        int optval = 1;
        setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        //bind to port
        sockaddr_in a = {};
        a.sin_family = AF_INET; // IPv4
        a.sin_addr.s_addr = addr;
        a.sin_port = htons(port); // port

        if (::bind(_fd, (const sockaddr *) &a, sizeof(a)) < 0) {
            err = "bind";
            break;
        }

        printf("%s:listening on port %u\n", _name, ntohs(a.sin_port));
        fflush(stdout);

        return true;

    } while (0);

    if ((!_silent) && err && err[0]) {
        printf("%s:error: %s (%d)\n", _name, err, errno);
        fflush(stdout);
    }

    close();
    return false;
}

bool udp_client::is_connected(void)
{
    return _fd >= 0;
}

void udp_client::close()
{
    if (_fd >= 0)
        ::close(_fd);
    _fd = -1;
}

size_t udp_client::read(void *buf, size_t size)
{
    int cnt = ::recvfrom(_fd, buf, size, MSG_WAITALL, (sockaddr *) &_remote_addr, &_remote_addr_len);
    if (cnt >= 0)
        return cnt;

    if (!_silent) {
        printf("%s:receive failed (%d)\n", _name, errno);
        fflush(stdout);
    }

    close();
    return 0;
}

bool udp_client::write(const void *buf, size_t size)
{
    if (!size)
        return true;

    if (!is_connected()) {
        return false;
    }
    if (::sendto(_fd, buf, size, 0, (sockaddr *) &_dest_addr, sizeof(_dest_addr)) == (ssize_t) size)
        return true;

    if (!_silent) {
        printf("%s:send failed (%d)\n", _name, errno);
        fflush(stdout);
    }

    close();
    return false;
}

bool udp_client::dataAvailable()
{
    if (!is_connected())
        return false;

    int bytes_available;
    if (::ioctl(_fd, FIONREAD, &bytes_available) < 0) {
        if (!_silent) {
            printf("%s:ioctl failed (%d)\n", _name, errno);
            fflush(stdout);
        }
        return false;
    }

    return bytes_available > 0;
}

} // namespace tcp
} // namespace xbus
