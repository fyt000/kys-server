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
        asio::read(socket2, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;

        int rand_data = 42;
        asio::write(socket1, asio::buffer(&rand_data, sizeof(rand_data)));
        int rand_echo;
        asio::read(socket2, asio::buffer(&rand_echo, sizeof(rand_echo)));
        std::cout << rand_echo << std::endl;

        asio::write(socket2, asio::buffer(&rand_data, sizeof(rand_data)));
        rand_echo = 0;
        asio::read(socket1, asio::buffer(&rand_echo, sizeof(rand_echo)));
        std::cout << rand_echo << std::endl;

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
        asio::read(socket2, asio::buffer(&result, sizeof(result)));
        std::cout << result << std::endl;
}


    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    std::cin.get();
    return 0;
}