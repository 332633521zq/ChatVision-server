#include "logicnode.h"

LogicNode::LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recvnode)
    : _session(session)
    , _recvnode(recvnode)
{}
