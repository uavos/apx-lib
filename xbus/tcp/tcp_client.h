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

    virtual size_t read(void *buf, size_t size);
    virtual bool write(const void *buf, size_t size);

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

    static ssize_t read(int fd, void *buf, size_t size);
    static bool write(int fd, const void *buf, size_t size);
};

} // namespace tcp
} // namespace xbus
