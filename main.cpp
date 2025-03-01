#include "boost/asio.hpp"
#include "server.h"
#include <boost/signals2.hpp>
#include <iostream>

int main()
{
    try {
        boost::asio::io_context io_context;
        std::cout << "main" << std::endl;
        Server s(io_context, 10086);
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
