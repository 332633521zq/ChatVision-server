#pragma once
#include "session.h"
#include "singleton.h"
#include <boost/asio.hpp>
#include <map>

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_context& io_context, short port);
    std::shared_ptr<Session> FindSessionByUuid(std::string uuid);
    void ClearSession(std::string uuid);

private:
    void HandleAccept(std::shared_ptr<Session> new_session, const boost::system::error_code& error);
    void StartAccept();

    boost::asio::io_context& _io_context;
    short _port;
    tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<Session>> _sessions;
};
