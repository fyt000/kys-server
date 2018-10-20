#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <iostream>
#include "Connection.h"

class Bridge : public std::enable_shared_from_this<Bridge> {
public:
    Bridge(std::shared_ptr<HostConnection> host, std::shared_ptr<ClientConnection> client) :
        host_(host), client_(client) {}

    void go() {
        auto self(shared_from_this());
        boost::asio::spawn([this, self](boost::asio::yield_context yield) {
            relay(host_->socket, client_->socket, yield);
        });
        boost::asio::spawn([this, self](boost::asio::yield_context yield) {
            relay(client_->socket, host_->socket, yield);
        });
    }

private:
    void relay(boost::asio::ip::tcp::socket& from_socket, boost::asio::ip::tcp::socket& to_socket,
        boost::asio::yield_context yield) {
        try {
            char data[8192];
            while (from_socket.is_open() && to_socket.is_open()) {
                std::size_t n = from_socket.async_read_some(boost::asio::buffer(data), yield);
                boost::asio::async_write(to_socket, boost::asio::buffer(data, n), yield);
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            from_socket.close();
            to_socket.close();
            return;
        }
    }

    // single threaded, no need for strand, fix it later
    // asio::io_context::strand strand_;
    std::shared_ptr<HostConnection> host_;
    std::shared_ptr<ClientConnection> client_;
};
