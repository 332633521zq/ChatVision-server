#include "boost/asio.hpp"
#include "ioservicepool.h"
#include "server.h"
#include <boost/signals2.hpp>
#include <iostream>

int main()
{
    try {
        // system("sudo ifconfig enp92s0 192.168.1.100 netmask 255.255.255.0");
        auto pool = IOServicePool::GetInstance();
        boost::asio::io_context io_context;
        std::cout << "main" << std::endl;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](auto, auto) { io_context.stop(); });

        Server s(io_context, 10086);
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
