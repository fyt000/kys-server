# kys-server
Server for kys-cpp-battle-mod https://github.com/scarsty/kys-cpp/tree/battle-mod

Basically a TCP relay between 2 clients, clients are matched by a string id. Using Boost::asio(and coroutine) as the networking library.

### host 

- not really a host, but rather Player1

1. connect to server on some port
2. send len + hostname
3. return ok if username is new else err
4. awaiting

### client 

- Player2

1. connect to server on some other port
2. send len + hostname
3. return ok if host exists else err
4. make bridge with host and remove host from awaiting_hosts

### bridge

forward stuff to each other

### acceptor

accept connection from host and client

### awaitinghost

map of waiting hosts
