#pragma once
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <chrono>
#include "Connection.h"
#include "AwaitingHost.h"

class Acceptor {
public:
    Acceptor(boost::asio::io_service& io_context, unsigned short host_port, unsigned short client_port, unsigned short udp_port)
        : io_context_(io_context),
        host_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), host_port)),
        client_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), client_port)),
        udp_socket_(io_context_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), udp_port)) { }

    // TODO refactor
    bool accept_hosts() {
        std::cerr << "start accepting hosts" << std::endl;
        boost::asio::spawn([this](boost::asio::yield_context yield) {
            for (;;) {
                boost::asio::ip::tcp::socket socket(io_context_);
                try {
                    boost::asio::socket_base::reuse_address option(true);
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
        boost::asio::spawn([this](boost::asio::yield_context yield) {
            for (;;) {
                boost::asio::ip::tcp::socket socket(io_context_);
                try {
                    boost::asio::socket_base::reuse_address option(true);
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


    // adhoc might not be the word here
    // currently only handles request for waiting host names
    void adhoc_responder() {
        boost::asio::spawn([this](boost::asio::yield_context yield) {
            for (;;) {
                int req_type;
                boost::asio::ip::udp::endpoint remote_endpoint;
                try {
                    udp_socket_.async_receive_from(boost::asio::buffer(&req_type, sizeof(req_type)), remote_endpoint, yield);
                    std::cerr << "adhoc request " << req_type << std::endl;
                    switch(req_type) {
                        // TODO use enum class
                        case 0: {
                            auto names = AwaitingHost::get().waiting_hosts();
                            auto size = names.size();
                            udp_socket_.async_send_to(boost::asio::buffer(&size, sizeof(size)), remote_endpoint, yield);
                            for (auto& str : names) {
                                auto size = str.size();
                                udp_socket_.async_send_to(boost::asio::buffer(&size, sizeof(size)), remote_endpoint, yield);
                                udp_socket_.async_send_to(boost::asio::buffer(&str[0], str.size()), remote_endpoint, yield);
                            }
                        }
                    }
                } catch (std::exception& e) {
                    std::cerr << "adhoc err " << e.what() << std::endl;
                }
            }
        });
    }

    // cleanup stuff
    // this creates a lot of threading problems, race conditions
    void periodic_cleanup() {
        boost::asio::spawn([this](boost::asio::yield_context yield) {
            for (;;) {
                boost::asio::deadline_timer t(io_context_, boost::posix_time::seconds(30));
                t.async_wait(yield);
                std::cerr << "cleanning up" << std::endl;
                AwaitingHost::get().host_cleanup();
                std::cerr << "cleanning up done" << std::endl;
            }
        });
    }

private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor host_acceptor_;
    boost::asio::ip::tcp::acceptor client_acceptor_;
    boost::asio::ip::udp::socket udp_socket_;
};