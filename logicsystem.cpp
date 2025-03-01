#include "logicsystem.h"
#include "filetools.h"
#include "msgbroker.h"
#include "server.h"
#include "usermanager.h"

using namespace nlohmann;

LogicSystem::LogicSystem()
{
    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::DealMsg()
{
    for (;;) {
        std::unique_lock<std::mutex> unique_lk(_mutex);

        // 队列为空，则用条件变量等待（挂起线程，加锁解锁需要消耗cpu资源）
        while (_msg_que.empty() && !_b_stop) {
            _consume.wait(unique_lk); // 线程挂起，释放资源，解锁，等待
        }

        // 服务器即将停止，取出逻辑队列所有数据及时处理并退出循环
        if (_b_stop) {
            while (!_msg_que.empty()) {
                auto msg_node = _msg_que.front();
                std::cout << "recv msg id is" << msg_node->_recvnode->GetMsgId() << std::endl;
                auto call_back_iter = _fun_callback.find(msg_node->_recvnode->GetMsgId());
                if (call_back_iter == _fun_callback.end()) {
                    _msg_que.pop();
                    continue;
                }
                call_back_iter->second(msg_node->_session,
                                       msg_node->_recvnode->GetMsgId(),
                                       std::string(msg_node->_recvnode->_data,
                                                   msg_node->_recvnode->_total_len));
                _msg_que.pop();
            }
            break;
        }

        // 没有停服，且队列中有数据
        auto msg_node = _msg_que.front();
        std::cout << "recv msg id is" << msg_node->_recvnode->GetMsgId() << std::endl;
        auto call_back_iter = _fun_callback.find(msg_node->_recvnode->GetMsgId());
        if (call_back_iter == _fun_callback.end()) {
            _msg_que.pop();
            continue;
        }
        call_back_iter->second(msg_node->_session,
                               msg_node->_recvnode->GetMsgId(),
                               std::string(msg_node->_recvnode->_data,
                                           msg_node->_recvnode->_total_len));
        _msg_que.pop();
    }
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> unique_lk(_mutex);
    _msg_que.push(msg);

    if (_msg_que.size() == 1) {
        _consume.notify_one(); // 唤醒逻辑线程
    }
}

LogicSystem::~LogicSystem()
{
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
}

void LogicSystem::RegisterCallBacks()
{
    _fun_callback[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallBack,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    _fun_callback[MSG_LOGIN] = std::bind(&LogicSystem::LoginCallBack,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3);

    _fun_callback[MSG_TEXT_CHAT] = std::bind(&LogicSystem::TextChatCallBack,
                                             this,
                                             std::placeholders::_1,
                                             std::placeholders::_2,
                                             std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<Session> session,
                                     const short &msg_id,
                                     const std::string &msg_data)
{
    std::cout << "HelloWorldCallBack--receive msg: " << msg_data << std::endl;
    // std::string return_str = "server has received msg, msg data is" + msg_data;
    json parse_msg = json::parse(msg_data);
    parse_msg["data"] = "server has received msg, msg data is " + parse_msg["data"].dump();
    session->Send(parse_msg.dump(), msg_id);
}

// 用户登陆：
// 1. 绑定uid与uuid
// 2. 从数据库获取该用户的个人信息，并发送给用户
void LogicSystem::LoginCallBack(std::shared_ptr<Session> session,
                                const short &msg_id,
                                const std::string &msg_data)
{
    std::cout << "LoginCallBack---" << std::endl;

    nlohmann::json msg;
    msg = nlohmann::json::parse(msg_data);
    unsigned int uid = msg.at("uid");

    UserManager::GetInstance()->AddToConnectsList(uid, session->GetUuid());

    // 等数据库部分写好
    // nlohmann::json user_info = UserManager::GetInstance()->GainUserInformation(uid);
    // session->Send(user_info.dump(), msg_id);

    session->Send(msg.dump(), msg_id);
}

// 用户聊天：
// 1.将聊天消息存储到本地服务器（路径：./[uid]-[object_id]/聊天日期）
// 2.通过object_id找到对方的uuid,进而找到对方的消息处理对象session，并通过该session将消息发送给对方
void LogicSystem::TextChatCallBack(std::shared_ptr<Session> session,
                                   const short &msg_id,
                                   const std::string &msg_data)
{
    std::cout << "TextChatCallBack---" << std::endl;

    nlohmann::json msg;
    msg = nlohmann::json::parse(msg_data);

    MsgBroker::StoreChatMsg(msg);

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");
    // std::string data = msg.at("data");

    FileTools::GetInstance()->SaveTextMsg(uid, obj_id, msg_data);

    // 目标对象视角的uid和object_id
    msg["uid"] = obj_id;
    msg["object_id"] = uid;

    std::string obj_uuid = UserManager::GetInstance()->GetUuidByUid(obj_id);
    if (obj_uuid != "") {
        auto obj_session = session->_server->FindSessionByUuid(obj_uuid);
        std::cout << "obj_id:" << obj_id << "\tobj_uuid:" << obj_uuid << std::endl;
        obj_session->Send(msg.dump(), msg_id);
    } else {
        std::cout << "He/She is not online" << std::endl;
        // 先将消息存入服务器，等待对方上线
    }
}
