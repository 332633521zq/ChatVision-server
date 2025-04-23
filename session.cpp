#include "session.h"
#include "logicnode.h"
#include "logicsystem.h"
#include "server.h"
#include "usermanager.h"
// #include <iomanip>
#include <iostream>

Session::Session(boost::asio::io_context& io_context, Server* server)
    : _socket(io_context)
    , _server(server)
{
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LENGTH);
}

Session::~Session()
{
    UserManager::GetInstance()->DisconnectUser(_uuid);
    std::cout << "~Session " << _uuid << " destruct\n\n" << std::endl;
}

void Session::RemoveOldSession(std::string uuid)
{
    std::cout << "RemoveOldSession" << std::endl;
    (_server->FindSessionByUuid(uuid))->Close();
    _server->ClearSession(uuid);
}

tcp::socket& Session::GetSocket()
{
    return _socket;
}

std::string& Session::GetUuid()
{
    return _uuid;
}

void Session::StartReadHead()
{
    outstanding_ops_++;
    _recv_head_node->Clear();
    boost::asio::async_read(_socket,
                            boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LENGTH),
                            std::bind(&Session::HandleReadHead,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2,
                                      SharedSelf()));
}

void Session::StartReadContent()
{
    outstanding_ops_++;
    _recv_msg_node->Clear();
    boost::asio::async_read(_socket,
                            boost::asio::buffer(_recv_msg_node->_data,
                                                _recv_msg_node->_total_len - HEAD_TOTAL_LENGTH),
                            std::bind(&Session::HandleReadContent,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2,
                                      SharedSelf()));
}

void Session::StartWrite(std::shared_ptr<SendNode>& msgnode)
{
    outstanding_ops_++;
    boost::asio::async_write(_socket,
                             boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                             std::bind(&Session::HandleWrite,
                                       this,
                                       std::placeholders::_1,
                                       SharedSelf()));
}

void Session::Start()
{
    memset(_data, 0, MAX_LENGTH);
    StartReadHead();
}

void Session::Close()
{
    if (_b_close)
        return;
    _b_close = true;
    boost::system::error_code ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // 通知对端关闭

    if (outstanding_ops_ == 0) {
        _socket.close(ec); // 无未完成操作，直接关闭
    }
}

std::shared_ptr<Session> Session::SharedSelf()
{
    return shared_from_this();
}

void Session::PrintRecvData(char* data, int length)
{
    std::stringstream ss;
    std::string result = "0x";
    for (int i = 0; i < length; i++) {
        std::string hexstr;
        ss << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << std::endl;
        ss >> hexstr;
        result += hexstr;
    }
    std::cout << "receive raw data is : " << result << std::endl;
}

void Session::HandleWrite(const boost::system::error_code& error,
                          std::shared_ptr<Session> _self_shared)
{
    outstanding_ops_--;
    if (!error) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty()) {
            auto& msgnode = _send_que.front();
            StartWrite(msgnode);
        }
    } else {
        std::cout << "handle write failed, error is" << error.what() << std::endl;
        _server->ClearSession(_uuid);
    }
}

void Session::HandleReadHead(const boost::system::error_code& error,
                             size_t bytes_transfered,
                             std::shared_ptr<Session> _self_shared)
{
    outstanding_ops_--;
    if (!error) {
        if (bytes_transfered < HEAD_TOTAL_LENGTH) {
            std::cout << "imcompleted msg head " << std::endl;
            Close();
            _server->ClearSession(_uuid);
        }

        // 获取头部msg-id数据
        short msg_id = 0;
        memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LENGTH);
        msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
        if (msg_id > MAX_LENGTH) {
            std::cout << "invailid msg id length is " << msg_id << std::endl;
            Close();
            _server->ClearSession(_uuid);
            return;
        }

        short msg_len = 0;
        memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);
        msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
        std::cout << "msg_len is " << msg_len << std::endl;

        // 头部长度非法
        if (msg_len > MAX_LENGTH) {
            std::cout << "invailid data length is " << msg_len << std::endl;
            _b_head_parse = false;
            _server->ClearSession(_uuid);
            return;
        }

        _recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
        StartReadContent();
    } else {
        std::cout << "handle read failed, error is " << error.what() << std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

void Session::HandleReadContent(const boost::system::error_code& error,
                                size_t bytes_transfered,
                                std::shared_ptr<Session> _self_shared)
{
    outstanding_ops_--;
    if (!error) {
        if (bytes_transfered < _recv_msg_node->_total_len - HEAD_TOTAL_LENGTH) {
            std::cout << "imcompleted msg content " << std::endl;
            Close();
            _server->ClearSession(_uuid);
        }
        _recv_msg_node->_data[_recv_msg_node->_total_len - HEAD_TOTAL_LENGTH] = '\0';

        std::cout << "_recv_msg_node->_data:" << _recv_msg_node->_data << std::endl;
        LogicSystem::GetInstance()->PostMsgToQue(
            std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

        StartReadHead();
    }
}

void Session::Send(char* msg, int max_length, short msgid)
{
    std::lock_guard<std::mutex> lock(_send_lock);

    int send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "sessions" << _uuid << "send que fulled, size is" << MAX_SENDQUE << std::endl;
        return;
    }

    _send_que.push(std::make_shared<SendNode>(msg, max_length, msgid));

    if (_send_que.size() > 0) {
        return;
    }

    auto& msgnode = _send_que.front();
    std::cout << "msgnode data is: " << msgnode->_data << std::endl;
    StartWrite(msgnode);
}

void Session::Send(std::string msg, short msgid)
{
    // std::cout << "Session::Send(std::string msg, short msgid) \n" << msg << std::endl;
    std::lock_guard<std::mutex> lock(_send_lock);

    int send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "sessions" << _uuid << "send que fulled, size is" << MAX_SENDQUE << std::endl;
        return;
    }

    _send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));

    if (send_que_size > 0) {
        return;
    }

    auto& msgnode = _send_que.front();
    StartWrite(msgnode);
}
