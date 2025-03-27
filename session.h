#pragma once

#include "ConstValue.h"
#include "msgnode.h"
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <mutex>
#include <queue>

using boost::asio::ip::tcp;

class Server;

// 处理客户端消息收发的会话类
class Session : public std::enable_shared_from_this<Session>
{
    friend class LogicSystem;

public:
    Session(boost::asio::io_context& io_context, Server* server);
    ~Session();

    tcp::socket& GetSocket();
    std::string& GetUuid();

    void Start();
    void Send(char* msg, int max_length, short msgid); // 用于发送数据，以及往发送队列里添加数据
    void Send(std::string msg, short msgid);
    void Close();
    void RemoveOldSession(std::string uuid);

    std::shared_ptr<Session> SharedSelf(); // 保证智能指针的引用计数

private:
    void PrintRecvData(char* data, int length);
    void HandleRead(const boost::system::error_code& error,
                    size_t bytes_transfered,
                    std::shared_ptr<Session> _self_shared);
    void HandleWrite(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared);
    void StartRead();

    void StartWrite(std::shared_ptr<SendNode>& msgnode);

    tcp::socket _socket;
    std::string _uuid;
    char _data[MAX_LENGTH];
    Server* _server;

    bool _b_close{false}; // 服务器进程是否停止
    std::atomic<int> outstanding_ops_{0};
    std::queue<std::shared_ptr<SendNode>> _send_que; // 发送队列
    std::mutex _send_lock;

    std::shared_ptr<RecvNode> _recv_msg_node; // 收到的消息结构
    bool _b_head_parse;                       // 头部是否被解析

    std::shared_ptr<MsgNode> _recv_head_node; // 收到的头部结构
};
