#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/attributes.hpp>

#include <chrono>

#include "Connection.h"
#include "AwaitingHost.h"

HostConnection::HostConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::socket socket_in) : ioc_(ioc), socket(std::move(socket_in)), enlisted_(false), timer_(ioc_) {
}

void HostConnection::go() {
    auto self(shared_from_this());
    boost::asio::spawn([this, self](boost::asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            boost::asio::async_read(socket, boost::asio::buffer(&len, sizeof(len)), yield);
            name_.resize(len);
            boost::asio::async_read(socket, boost::asio::buffer(&name_[0], len), yield);
            std::cerr << "host request " << name_ << std::endl;
            // enlist as waiting
            enlisted_ = AwaitingHost::get().enlist(self);
            auto result = static_cast<int>(enlisted_);
            boost::asio::async_write(socket, boost::asio::buffer(&result, sizeof(result)), yield);
            if (enlisted_) {
                // keep it going until either write fails or a client connects to the host
                int zero = 0;
                boost::system::error_code ec;
                for (;;) {                    
                    timer_.expires_after(std::chrono::seconds(5));
                    timer_.async_wait(yield[ec]);
                    if (ec == boost::asio::error::operation_aborted){
                        return; // bridge is up
                    }
                    boost::asio::async_write(socket, boost::asio::buffer(&zero, sizeof(zero)), yield);
                }
            }
            // if enlist fails this coroutine will exit and the host is destroyed
        }
        catch (std::exception& e) {
            socket.close();
            return;
        }
    });
}

void HostConnection::stop_timer() {
    timer_.cancel();
}

HostConnection::~HostConnection() {
    if (enlisted_) {
        AwaitingHost::get().remove(name_);
    }
}

ClientConnection::ClientConnection(boost::asio::io_context&, boost::asio::ip::tcp::socket socket_in) : socket(std::move(socket_in)) {
}

void ClientConnection::go() {
    auto self(shared_from_this());
    boost::asio::spawn([this, self](boost::asio::yield_context yield) {
        // recv hostname
        try {
            int len;
            boost::asio::async_read(socket, boost::asio::buffer(&len, sizeof(len)), yield);
            name_.resize(len);
            boost::asio::async_read(socket, boost::asio::buffer(&name_[0], len), yield);
            std::cerr << "client request " << name_ << std::endl;
        }
        catch (std::exception& e) {
            socket.close();
            return;
        }
        // try matching with host
        auto bridge = AwaitingHost::get().match_host(name_, self);
        try {
            int result = bridge ? 1 : 0;
            boost::asio::async_write(socket, boost::asio::buffer(&result, sizeof(result)), yield);
        }
        catch (std::exception& e) {
            socket.close();
            return;
        }
        if (bridge) bridge->go();
    });
}


