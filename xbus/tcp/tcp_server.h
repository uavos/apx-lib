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

#include "tcp_client.h"

#include <XbusPacket.h>
#include <fifo.hpp>
#include <pthread.h>

namespace xbus {
namespace tcp {

class tcp_server : public tcp_client
{
public:
    explicit tcp_server(const char *name = "server");
    ~tcp_server();

    bool connect() override;
    void close() override;

    size_t read(void *buf, size_t size) override;
    bool write(const void *buf, size_t size) override;

    bool is_connected(void) override;

    void run();
    bool create_thread();

private:
    int _server_fd{-1};

    int _client_fd[50];
    size_t _client_cnt{0};

    pthread_t _tid[50];
    size_t _tid_cnt{0};
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

    apx::fifo_packet_static<xbus::size_packet_max * 4> _rx_fifo;
};

} // namespace tcp
} // namespace xbus
