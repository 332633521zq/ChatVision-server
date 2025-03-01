#pragma once

#include "msgnode.h"
#include <memory>

class Session;
class RecvNode;
class LogicSystem;

class LogicNode
{
    friend class LogicSystem;

public:
    LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recvnode);

private:
    std::shared_ptr<Session> _session;
    std::shared_ptr<RecvNode> _recvnode;
};
