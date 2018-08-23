#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/attributes.hpp>
#include "Connection.h"
#include "AwaitingHost.h"

HostConnection::HostConnection(boost::asio::ip::tcp::socket socket_in) : socket(std::move(socket_in)) {
}

void HostConnection::go() {
    auto self(shared_from_this());
    boost::asio::spawn([this, self](boost::asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            boost::asio::async_read(socket, boost::asio::buffer(&len, sizeof(len)), yield);
            name.resize(len);
            boost::asio::async_read(socket, boost::asio::buffer(&name[0], len), yield);
            std::cerr << "host request " << name << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "host err1 " << e.what() << std::endl;
            socket.close();
            return;
        }
        // enlist as waiting (ok message should be sent by awaiting too, synchronously too)
        bool ok = AwaitingHost::get().enlist(shared_from_this());
    });
    // (const boost::coroutines::attributes&) boost::coroutines::attributes(2 * 1024 * 1024)
}


ClientConnection::ClientConnection(boost::asio::ip::tcp::socket socket_in) : socket(std::move(socket_in)) {
}

void ClientConnection::go() {
    auto self(shared_from_this());
    boost::asio::spawn([this, self](boost::asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            boost::asio::async_read(socket, boost::asio::buffer(&len, sizeof(len)), yield);
            name.resize(len);
            boost::asio::async_read(socket, boost::asio::buffer(&name[0], len), yield);
            std::cerr << "client request " << name << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "client err1 " << e.what() << std::endl;
            socket.close();
            return;
        }
        // try matching with host
        auto bridge = AwaitingHost::get().match_host(name, self);
        try {
            int result = bridge ? 1 : 0;
            boost::asio::async_write(socket, boost::asio::buffer(&result, sizeof(result)), yield);
        }
        catch (std::exception& e) {
            std::cerr << "client err2 " << e.what() << std::endl;
            socket.close();
            return;
        }
        if (bridge) bridge->go();
    });
    // (const boost::coroutines::attributes&) boost::coroutines::attributes(2 * 1024 * 1024)
}


