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

using namespace xbus::tcp;

Server::Server(const char *name)
    : Client(name)
    , server_fd(-1)
{
    //tcpdebug=true;
}
Server::~Server()
{
    if (server_fd > 0)
        ::close(server_fd);
    server_fd = -1;
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
