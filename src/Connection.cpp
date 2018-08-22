#include <asio.hpp>
#include "Connection.h"
#include "AwaitingHost.h"

HostConnection::HostConnection(asio::ip::tcp::socket socket_in) : socket(std::move(socket_in)) {
}

void HostConnection::go() {
    auto self(shared_from_this());
    asio::spawn([this, self](asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            asio::async_read(socket, asio::buffer(&len, sizeof(len)), yield);
            std::cerr << "name length " << len << std::endl;
            std::vector<char> name_buf(len + 1, '\0');
            asio::async_read(socket, asio::buffer(&name_buf[0], len), yield);
            name = std::string(&name_buf[0]);
            std::cerr << "Got host " << name << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "host err1 " << e.what() << std::endl;
            socket.close();
            return;
        }
        // enlist as waiting
        bool ok = AwaitingHost::getAwaitingHosts().enlist(shared_from_this());
        try {
            int result = ok ? 1 : 0;
            asio::async_write(socket, asio::buffer(&result, sizeof(result)), yield);
        }
        catch (std::exception& e) {
            std::cerr << "host err2 " << e.what() << std::endl;
            socket.close();
            if (ok)
                AwaitingHost::getAwaitingHosts().remove(name);
            return;
        }
    });
}


ClientConnection::ClientConnection(asio::ip::tcp::socket socket_in) : socket(std::move(socket_in)) {
}

void ClientConnection::go() {
    auto self(shared_from_this());
    asio::spawn([this, self](asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            asio::async_read(socket, asio::buffer(&len, sizeof(len)), yield);
            std::cerr << "name length " << len << std::endl;
            std::vector<char> name_buf(len + 1, '\0');
            asio::async_read(socket, asio::buffer(&name_buf[0], len), yield);
            name = std::string(&name_buf[0]);
            std::cerr << "Got client " << name << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "client err1 " << e.what() << std::endl;
            socket.close();
            return;
        }
        // try matching with host
        auto bridge = AwaitingHost::getAwaitingHosts().match_host(name, self);
        try {
            int result = bridge ? 1 : 0;
            asio::async_write(socket, asio::buffer(&result, sizeof(result)), yield);
        }
        catch (std::exception& e) {
            std::cerr << "client err2 " << e.what() << std::endl;
            socket.close();
            return;
        }
        bridge->go();
    });
}


