#pragma once

#include <asio.hpp>
#include <asio/spawn.hpp>

class HostConnection : public std::enable_shared_from_this<HostConnection> {
public:
    HostConnection(asio::ip::tcp::socket socket_in);
    void go();
    std::string getName() {
        return name;
    }
    asio::ip::tcp::socket socket;

private:
    std::string name;
};

class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(asio::ip::tcp::socket socket_in);
    void go();
    asio::ip::tcp::socket socket;
private:
    std::string name;
};
