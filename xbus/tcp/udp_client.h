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

#include <atomic>
#include <inttypes.h>
#include <netinet/in.h>
#include <termios.h>

#include <tcp_ports.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

namespace xbus {
namespace tcp {

class udp_client
{
public:
    udp_client(const char *name = "tcp", bool silent = false);
    ~udp_client();

    void set_dest(const char *host, uint16_t port);

    bool bind(in_addr_t addr, uint16_t port);
    bool bind(uint16_t port) { return bind(INADDR_ANY, port); }

    void close();

    size_t read(void *buf, size_t size);
    bool write(const void *buf, size_t size);

    bool is_connected();
    bool dataAvailable();

private:
    std::atomic<int> _fd{-1};

protected:
    const char *_name;
    bool _silent{};

    bool _debug{};

    sockaddr_in _remote_addr{};
    socklen_t _remote_addr_len{sizeof(_remote_addr)};

    sockaddr_in _dest_addr{};
};

} // namespace tcp
} // namespace xbus
