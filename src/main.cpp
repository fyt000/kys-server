#include <iostream>
#include <boost/asio.hpp>

#include "Acceptor.h"

/*
protocol

host - not really a host, but rather Player1
1. connect to 8120
2. send len + hostname
3. return ok if username is new else err
4. awaiting

client - Player2
1. connect to 8121
2. send len + hostname
3. return ok if host exists else err
4. make bridge with host and remove host from awaiting_hosts

bridge
forward stuff to each other

acceptor
accept connection from host and client
*/

int main() {
    
    boost::asio::io_context io_context;

    try {
        Acceptor acceptor(io_context, 31111, 31112, 31113);
        acceptor.accept_hosts();
        acceptor.accept_clients();
        acceptor.adhoc_responder();
        acceptor.periodic_cleanup();

        // asio::io_service::work work(io_context);
        io_context.run();
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}