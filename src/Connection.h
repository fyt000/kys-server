#pragma once

#include <boost/asio.hpp>

class HostConnection : public std::enable_shared_from_this<HostConnection> {
    // Once enlisted a host connection name is only cleaned up if the connection dies
public:
    HostConnection(boost::asio::ip::tcp::socket socket_in);
    ~HostConnection();
    void go();
    std::string get_name() {
        return name_;
    }
    void stop_timer();
    boost::asio::ip::tcp::socket socket;

private:
    std::string name_;
    bool enlisted_;
    boost::asio::steady_timer timer_;
};

class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(boost::asio::ip::tcp::socket socket_in);
    void go();
    boost::asio::ip::tcp::socket socket;
private:
    std::string name_;
};
