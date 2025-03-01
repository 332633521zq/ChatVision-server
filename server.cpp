#include "server.h"
#include "relationalbroker.h"
#include "session.h"
#include <iostream>

Server::Server(boost::asio::io_context& io_context, short port)
    : _io_context(io_context)
    , _port(port)
    , _acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "server start" << std::endl;

    RelationalBroker::InitDataBase();
    StartAccept();
}

void Server::StartAccept()
{
    std::shared_ptr<Session> new_session = std::make_shared<Session>(_io_context, this);
    _acceptor
        .async_accept(new_session->GetSocket(),
                      std::bind(&Server::HandleAccept, this, new_session, std::placeholders::_1));
}

void Server::HandleAccept(std::shared_ptr<Session> new_session,
                          const boost::system::error_code& error)
{
    if (!error) {
        new_session->Start();
        _sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
    } else {
        std::cout << "session accept failed,error is" << error << std::endl;
    }

    StartAccept();
}

std::shared_ptr<Session> Server::FindSessionByUuid(std::string uuid)
{
    auto session_iter = _sessions.find(uuid);
    if (session_iter == _sessions.end()) {
        return nullptr;
    }
    return session_iter->second;
}

void Server::ClearSession(std::string uuid)
{
    _sessions.erase(uuid);
}
