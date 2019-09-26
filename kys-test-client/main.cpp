#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

// _WIN32_WINNT=0x0A00

using namespace boost;
using asio::ip::tcp;


int main(int argc, char* argv[])
{
    try
    {
        asio::io_context io_context;

{
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints1 = resolver.resolve("localhost", "31111");
        tcp::socket socket1(io_context);
        asio::connect(socket1, endpoints1);
        std::cerr << "got 1" << std::endl;

        tcp::resolver::results_type endpoints2 = resolver.resolve("localhost", "31112");
        tcp::socket socket2(io_context);
        asio::connect(socket2, endpoints2);
        std::cerr << "got 2" << std::endl;

        std::string hello("hello");
        int len = hello.size();
        asio::write(socket1, asio::buffer(&len, sizeof(len)));
        asio::write(socket1, asio::buffer(hello.data(), len));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        asio::write(socket2, asio::buffer(&len, sizeof(len)));
        asio::write(socket2, asio::buffer(hello.data(), len));

        int result;
        // should be 1 on both sides
        asio::read(socket1, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
        assert(result == 1);
        asio::read(socket2, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
        assert(result == 1);

        int rand_data = 42;
        asio::write(socket1, asio::buffer(&rand_data, sizeof(rand_data)));
        int rand_echo;
        asio::read(socket2, asio::buffer(&rand_echo, sizeof(rand_echo)));
        std::cout << rand_echo << std::endl;
        assert(rand_echo == 42);

        asio::write(socket2, asio::buffer(&rand_data, sizeof(rand_data)));
        rand_echo = 0; // try to that 42 back
        asio::read(socket1, asio::buffer(&rand_echo, sizeof(rand_echo)));
        std::cout << rand_echo << std::endl;
        assert(rand_echo == 42);
}

{
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints1 = resolver.resolve("localhost", "31111");
        tcp::socket socket1(io_context);
        asio::connect(socket1, endpoints1);
        std::cerr << "got 1" << std::endl;

        tcp::resolver::results_type endpoints2 = resolver.resolve("localhost", "31112");
        tcp::socket socket2(io_context);
        asio::connect(socket2, endpoints2);
        std::cerr << "got 2" << std::endl;

        std::string hello("hello");
        int len = hello.size();
        asio::write(socket2, asio::buffer(&len, sizeof(len)));
        asio::write(socket2, asio::buffer(hello.data(), len));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        asio::write(socket1, asio::buffer(&len, sizeof(len)));
        asio::write(socket1, asio::buffer(hello.data(), len));

        int result;
        // should be 1 and 0 on both sides
        asio::read(socket1, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
        assert(result == 1);
        asio::read(socket2, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
        assert(result == 0);
        std::cout << "sleeping for 6s\n";
        std::this_thread::sleep_for(std::chrono::seconds(6));
        // get another 0
        asio::read(socket1, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
        assert(result == 0);
}

{
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints1 = resolver.resolve("localhost", "31111");
        tcp::socket socket1(io_context);
        asio::connect(socket1, endpoints1);

        tcp::resolver::results_type endpoints2 = resolver.resolve("localhost", "31111");
        tcp::socket socket2(io_context);
        asio::connect(socket2, endpoints2);

        std::string hello("hello");
        int len = hello.size();
        asio::write(socket1, asio::buffer(&len, sizeof(len)));
        asio::write(socket1, asio::buffer(hello.data(), len));
    
        std::string world("world");
        len = world.size();
        asio::write(socket2, asio::buffer(&len, sizeof(len)));
        asio::write(socket2, asio::buffer(world.data(), len));

        std::this_thread::sleep_for(std::chrono::seconds(1));
        asio::ip::udp::endpoint endpoint(asio::ip::address_v4::from_string("127.0.0.1"), 31113);
        asio::ip::udp::socket socket3(io_context);
        socket3.open(asio::ip::udp::v4());
        int zero = 0;
        socket3.send_to(asio::buffer(&zero, sizeof(zero)), endpoint);
        socket3.receive_from(asio::buffer(&len, sizeof(len)), endpoint);
        for (int i = 0; i < len; i++) {
            int len;
            socket3.receive_from(asio::buffer(&len, sizeof(len)), endpoint);
            std::string name;
            name.resize(len);
            socket3.receive_from(asio::buffer(&name[0], len), endpoint);
            // should get hello, world
            std::cout << name << std::endl;
        }
}

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    std::cin.get();
    return 0;
}
