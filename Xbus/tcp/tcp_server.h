#pragma once

#include "tcp_client.h"

#include <Xbus/XbusPacket.h>
#include <containers/QueueBuffer.h>
#include <pthread.h>

namespace xbus {
namespace tcp {

class Server : public Client
{
public:
    explicit Server(const char *name = "server");
    ~Server();

    bool connect() override;

    size_t read_packet(void *buf, size_t size) override;
    bool write_packet(const void *buf, size_t size) override;

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

    QueueBuffer<xbus::size_packet_max * 4> _rx_queue;
};

} // namespace tcp
} // namespace xbus
