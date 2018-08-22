#pragma once
#include <asio.hpp>
#include <iostream>
#include "Connection.h"

class Acceptor {
public:
    Acceptor(asio::io_service& io_context, unsigned short host_port, unsigned short client_port)
        : io_context_(io_context),
        host_acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), host_port)),
        client_acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), client_port)) { }

    // TODO refactor
    bool accept_hosts() {
        std::cerr << "start accepting hosts" << std::endl;
        asio::spawn([this](asio::yield_context yield) {
            for (;;) {
                asio::ip::tcp::socket socket(io_context_);
                try {
                    asio::socket_base::reuse_address option(true);
                    host_acceptor_.set_option(option);
                    host_acceptor_.async_accept(socket, yield);
                    auto host = std::make_shared<HostConnection>(std::move(socket));
                    host->go();
                }
                catch (std::exception& e) {
                    socket.close();
                    std::cerr << "host acceptor " << e.what() << std::endl;
                    return;
                }
            }
        });
        return true;
    }

    bool accept_clients() {
        std::cerr << "start accepting clients" << std::endl;
        asio::spawn([this](asio::yield_context yield) {
            for (;;) {
                asio::ip::tcp::socket socket(io_context_);
                try {
                    asio::socket_base::reuse_address option(true);
                    client_acceptor_.set_option(option);
                    client_acceptor_.async_accept(socket, yield);
                    auto client = std::make_shared<ClientConnection>(std::move(socket));
                    client->go();
                }
                catch (std::exception& e) {
                    socket.close();
                    std::cerr << "client acceptor " << e.what() << std::endl;
                    return;
                }
            }
        });
        return true;
    }

private:
    asio::io_context& io_context_;
    asio::ip::tcp::acceptor host_acceptor_;
    asio::ip::tcp::acceptor client_acceptor_;
};