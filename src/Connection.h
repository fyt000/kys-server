#pragma once

#include <boost/asio.hpp>

// TODO, add base class and refactor go

class HostConnection : public std::enable_shared_from_this<HostConnection> {
public:
    HostConnection(boost::asio::ip::tcp::socket socket_in);
    void go();
    std::string get_name() {
        return name;
    }
    boost::asio::ip::tcp::socket socket;

private:
    std::string name;
};

class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(boost::asio::ip::tcp::socket socket_in);
    void go();
    boost::asio::ip::tcp::socket socket;
private:
    std::string name;
};
