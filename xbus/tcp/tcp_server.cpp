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
#include "tcp_server.h"
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

#include <pthread.h>

using namespace xbus::tcp;

Server::Server(const char *name)
    : Client(name)
    , _server_fd(-1)
{
    tcpdebug = true;
}
Server::~Server()
{
    if (_server_fd > 0) {
        ::close(_server_fd);
    }
    /*pthread_mutex_lock(&_mutex);
    for (size_t i = 0; i < _tid_cnt; ++i) {
        pthread_join(_tid[i], NULL);
    }
    pthread_mutex_unlock(&_mutex);*/
}

bool Server::is_connected(void)
{
    return _server_fd > 0 && _client_cnt > 0;
}

static void *_thread_trampoline(void *arg)
{
    static_cast<Server *>(arg)->run();
    return nullptr;
}
bool Server::create_thread()
{
    pthread_mutex_lock(&_mutex);
    bool rv = pthread_create(&_tid[_tid_cnt], NULL, _thread_trampoline, this) == 0;
    if (rv)
        _tid_cnt++;
    pthread_mutex_unlock(&_mutex);

    if (rv)
        return true;

    printf("%s:error create thread\n", name);
    return false;
}

bool Server::connect()
{
    const char *err = nullptr;
    do {
        _server_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (_server_fd <= 0) {
            if (!err_mute)
                printf("%s:error:Open Socket Failed\n", name);
            err = "open";
            break;
        }

        // read timeout
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(_server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);

        //set options
        int optval = 1;
        setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        //bind & listen
        if (::bind(_server_fd, (const sockaddr *) &_host.addr, sizeof(_host.addr)) < 0) {
            if (!err_mute)
                printf("%s:error: Bind Failed (%i).\n", name, errno);
            err = "bind";
            break;
        }
        if (::listen(_server_fd, 1) < 0) {
            if (!err_mute)
                printf("%s:error: Listen Failed (%i).\n", name, errno);
            err = "listen";
            break;
        }

        setsockopt(_server_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#ifndef __APPLE__
        setsockopt(_server_fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif

        if (!create_thread()) {
            err = "thread";
            break;
        }
        printf("%s:listening on port %u\n", name, ntohs(_host.addr.sin_port));

        return true;

    } while (0);

    if (tcpdebug && err && err[0])
        printf("%s:error: %s\n", name, err);

    close();
    return false;
}

void Server::run()
{
    struct sockaddr_storage serverStorage;
    socklen_t addr_size = sizeof serverStorage;
    int fd = accept(_server_fd, (struct sockaddr *) &serverStorage, &addr_size);
    create_thread();

    // read timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);

    int optval = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#ifndef __APPLE__
    setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif

    int client_idx = -1;

    char line_buf[256];
    do {
        if (!readline(fd, line_buf, sizeof(line_buf)))
            break;
        if (strncmp(line_buf, "GET ", 4) != 0 || strncmp(line_buf + 4, _host.path, strlen(_host.path)) != 0) {
            printf("%s:request error: %s\n", name, line_buf);
            break;
        }
        for (;;) {
            if (!readline(fd, line_buf, sizeof(line_buf)))
                break;
            if (strlen(line_buf) == 0)
                break;
        }

        // response header
        strcpy(line_buf, "HTTP/1.0 200 OK\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        strcpy(line_buf, "Content-Type: application/octet-stream\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        sprintf(line_buf, "Server: %s\r\n", _host.path + 1);
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        strcpy(line_buf, "\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;

        // register client
        pthread_mutex_lock(&_mutex);
        client_idx = _client_cnt;
        _client_fd[client_idx] = fd;
        _client_cnt++;
        uint client_cnt = _client_cnt;
        pthread_mutex_unlock(&_mutex);

        printf("%s:client %u connected (%s)\n", name, client_cnt, _host.path);

        for (;;) {
            uint8_t buf[xbus::size_packet_max];
            ssize_t cnt = Client::read_packet(fd, buf, sizeof(buf));
            if (cnt < 0)
                break;
            if (cnt == 0)
                continue;
            pthread_mutex_lock(&_mutex);
            _rx_queue.write_packet(buf, cnt);
            pthread_mutex_unlock(&_mutex);
        }

    } while (0);

    if (client_idx >= 0) {
        pthread_mutex_lock(&_mutex);
        for (size_t i = client_idx + 1; i < _client_cnt; ++i) {
            _client_fd[i - 1] = _client_fd[i];
        }
        _client_cnt--;
        pthread_mutex_unlock(&_mutex);
        printf("%s:client %i disconnected\n", name, client_idx + 1);
    }

    ::close(fd);
}

size_t Server::read_packet(void *buf, size_t size)
{
    pthread_mutex_lock(&_mutex);
    size_t cnt = _rx_queue.read_packet(buf, size);
    pthread_mutex_unlock(&_mutex);
    return cnt;
}
bool Server::write_packet(const void *buf, size_t size)
{
    bool rv = false;
    pthread_mutex_lock(&_mutex);
    for (size_t i = 0; i < _client_cnt; ++i) {
        if (Client::write_packet(_client_fd[i], buf, size))
            rv = true;
    }
    pthread_mutex_unlock(&_mutex);
    return rv;
}

/*bool Server::connect_task()
{
    const char *err = nullptr;
    switch (init_stage) {
    case 0:
        return false; //idle
    case 1: {         //connecting
        time_tcp_s = time(0);
        if (server_fd >= 0) {
            init_stage++;
            break;
        }
#ifdef __APPLE__
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
#else
        server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#endif
        if (server_fd < 0) {
            if (!err_mute)
                printf("[%s]Error: Open Socket Failed.\n", name);
            err_mute = true;
            err = "socket";
            break;
        }
        //set options
        int optval = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        //bind & listen
        if (::bind(server_fd, (const sockaddr *) &host.addr, sizeof(host.addr)) < 0) {
            if (!err_mute)
                printf("[%s]Error: Bind Failed (%i).\n", name, errno);
            err_mute = true;
            err = "bind";
            break;
        }
        if (::listen(server_fd, 1) < 0) {
            if (!err_mute)
                printf("[%s]Error: Listen Failed (%i).\n", name, errno);
            err_mute = true;
            err = "listen";
            break;
        }
        //set options
        optval = 1;
        setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#ifndef __APPLE__
        optval = 1;
        setsockopt(server_fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif
        set_non_blocking();
        if (tcpdebug)
            printf("[%s]Listening on port %u\n", name, ntohs(host.addr.sin_port));
        init_stage++;
    } break;
    case 2: { //wait incoming connection
        time_tcp_s = time(0);
        //printf("[%s]errno (%i).\n",name,errno);
        fd = ::accept(server_fd, (struct sockaddr *) nullptr, nullptr);
        //printf("[%s]d (%i).\n",name,fd);
        if (fd >= 0) {
            if (tcpdebug)
                printf("[%s]Client socket connected.\n", name);
            //set options
            int optval = 1;
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#ifndef __APPLE__
            optval = 1;
            setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
#endif
            init_stage++;
            time_tcp_s = time(0);
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;
        err = "accept";
    } break;
    case 3: //request from client
        if (!readline())
            break;
        if (strncmp(line_buf, "GET ", 4) == 0 && strcmp(line_buf + 4, host.path) > 0) {
            init_stage++;
            break;
        }
        err = "header";
        break;
    case 4: //rest of header
        if (!readline())
            break;
        if (strlen(line_buf) == 0)
            init_stage++;
        break;
    case 5: //response header
        err = "send header";
        strcpy(line_buf, "HTTP/1.0 200 OK\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        strcpy(line_buf, "Content-Type: application/octet-stream\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        sprintf(line_buf, "Server: %s\r\n", host.path + 1);
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        strcpy(line_buf, "\r\n");
        if (::send(fd, line_buf, strlen(line_buf), 0) <= 0)
            break;
        err = nullptr;
        init_stage = 100;
        printf("[%s]Client connected (%s)\n", name, host.path);
        return true;
    case 20:
        //error - reconnect
        if ((time(0) - time_tcp_s) < 1)
            break;
        ::close(fd);
        fd = -1;
        ::close(server_fd);
        server_fd = -1;
        init_stage = 1;
        return false;
    }
    if ((time(0) - time_tcp_s) > 5) {
        err = "timeout";
    }
    if (err) {
        if (tcpdebug && err[0])
            printf("[%s]Error (%s).\n", name, err);
        ::close(fd);
        fd = -1;
        ::close(server_fd);
        server_fd = -1;
        init_stage = 20;
        time_tcp_s = time(0);
    }
    return false;
}*/
