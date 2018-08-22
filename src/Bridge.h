#pragma once
#include <memory>
#include <asio.hpp>
#include <iostream>
#include "Connection.h"

class Bridge : public std::enable_shared_from_this<Bridge> {
public:
    Bridge(std::shared_ptr<HostConnection> host, std::shared_ptr<ClientConnection> client) :
        host_(host), client_(client) {}

    void go() {
        auto self(shared_from_this());
        asio::spawn([this, self](asio::yield_context yield) {
            try {
                char data[1024];
                while (host_->socket.is_open() && client_->socket.is_open()) {
                    std::size_t n = host_->socket.async_read_some(asio::buffer(data), yield);
                    asio::async_write(client_->socket, asio::buffer(data, n), yield);
                }
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                host_->socket.close();
                client_->socket.close();
                return;
            }
        });
        asio::spawn([this, self](asio::yield_context yield) {
            try {
                char data[1024];
                while (host_->socket.is_open() && client_->socket.is_open()) {
                    std::size_t n = client_->socket.async_read_some(asio::buffer(data), yield);
                    asio::async_write(host_->socket, asio::buffer(data, n), yield);
                }
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                host_->socket.close();
                client_->socket.close();
                return;
            }
        });
    }
private:
    // single threaded, no need for strand, fix it later
    // asio::io_context::strand strand_;
    std::shared_ptr<HostConnection> host_;
    std::shared_ptr<ClientConnection> client_;
};