#include "logicsystem.h"
#include "filetools.h"
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
        std::cout << "\nrecv msg id is" << msg_node->_recvnode->GetMsgId() << std::endl;
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

    _fun_callback[MSG_FOLLOWING] = std::bind(&LogicSystem::FollowCallBack,
                                             this,
                                             std::placeholders::_1,
                                             std::placeholders::_2,
                                             std::placeholders::_3);

    _fun_callback[MSG_CANCEL_FOLLOW] = std::bind(&LogicSystem::CancelFollowCallBack,
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2,
                                                 std::placeholders::_3);

    _fun_callback[MSG_BLOCK] = std::bind(&LogicSystem::BlockCallBack,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3);

    _fun_callback[MSG_CANCEL_BLOCK] = std::bind(&LogicSystem::CancelBlockCallBack,
                                                this,
                                                std::placeholders::_1,
                                                std::placeholders::_2,
                                                std::placeholders::_3);

    _fun_callback[MSG_VIDEO_CHAT] = std::bind(&LogicSystem::AVChatCallBack,
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3);

    _fun_callback[MSG_AGREE_VIDEO] = std::bind(&LogicSystem::TransmitMsg,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    _fun_callback[MSG_VIDEO_CHAT_REFUSED] = std::bind(&LogicSystem::TransmitMsg,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      std::placeholders::_3);

    _fun_callback[MSG_AUDIO_CHAT] = std::bind(&LogicSystem::AVChatCallBack,
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3);

    _fun_callback[MSG_AGREE_AUDIO] = std::bind(&LogicSystem::TransmitMsg,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    _fun_callback[MSG_AUDIO_CHAT_REFUSED] = std::bind(&LogicSystem::TransmitMsg,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      std::placeholders::_3);

    _fun_callback[MSG_RANDOM_PUSH] = std::bind(&LogicSystem::RandomPushChatCallBack,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    _fun_callback[MSG_ONLINE_STATE] = std::bind(&LogicSystem::OnlineStateCallBack,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    _fun_callback[MSG_FILE] = std::bind(&LogicSystem::SendFileCallBack,
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

    std::string uuid = UserManager::GetInstance()->GetUuidByUid(uid);
    if (uuid != "") {
        std::cout << "remove uuid:" << uuid << std::endl;
        session->RemoveOldSession(uuid);
    }
    UserManager::GetInstance()->ConnectUser(uid, session->GetUuid());

    // 推送用户的基本信息
    nlohmann::json user_info = UserManager::GetInstance()->GetUserInformation(uid);
    msg["data"] = user_info;
    session->Send(msg.dump(), MSG_USER_INFO);

    // 推送用户的粉丝列表，关注列表，黑名单
    nlohmann::json following_list = UserManager::GetInstance()->GetFollowingUsersInfo(uid);
    msg["data"] = following_list;
    session->Send(msg.dump(), MSG_GET_FOLLOWINGS);

    nlohmann::json follower_list = UserManager::GetInstance()->GetFollowerUsersInfo(uid);
    msg["data"] = follower_list;
    session->Send(msg.dump(), MSG_GET_FOLLOWERS);

    nlohmann::json black_list = UserManager::GetInstance()->GetBlackListUsersInfo(uid);
    msg["data"] = black_list;
    session->Send(msg.dump(), MSG_GET_BLACKLIST);

    nlohmann::json chatted_userinfo = UserManager::GetInstance()->GetChattedUsersInfo(uid);
    msg["data"] = chatted_userinfo;
    session->Send(msg.dump(), MSG_CHATTED_USER);

    // 推送离线消息
    std::vector<unsigned int> chatted_users = UserManager::GetInstance()->FindChattedUsers(uid);

    if (chatted_users.size() > 0) {
        std::cout << "chatted_users.size() > 0" << std::endl;

        for (auto obj_id : chatted_users) {
            std::cout << "auto obj_id : chatted_users" << obj_id << std::endl;
            std::vector<std::string> msgs = FileTools::GetInstance()->GetOfflineTextMsg(uid, obj_id);
            if (msgs.size() != 0) {
                std::cout << "msgs.size() != 0" << std::endl;

                for (auto i : msgs) {
                    std::cout << "chat msg i:" << i << std::endl;
                    session->Send(i, MSG_TEXT_CHAT);
                }
            } else {
                std::cout << "vector msg is empty" << std::endl;
            }
        }
    } else {
        std::cout << "no chatted_users" << std::endl;
    }
}

// 用户聊天：
// 1.将聊天消息存储到本地服务器（路径：./[uid]-[object_id]/聊天日期）
// 2.通过object_id找到对方的uuid,进而找到对方的消息处理对象session，并通过该session将消息发送给对方
void LogicSystem::TextChatCallBack(std::shared_ptr<Session> session,
                                   const short &msg_id,
                                   const std::string &msg_data)
{
    std::cout << "TextChatCallBack---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    bool is_permitted = UserManager::GetInstance()->IsChatPermitted(uid, obj_id);

    if (is_permitted) {
        std::string obj_uuid = UserManager::GetInstance()->GetUuidByUid(obj_id);
        bool forward_state = obj_uuid == "" ? false : true;
        std::cout << "forward_state:" << forward_state << std::endl;
        if (forward_state) {
            auto obj_session = session->_server->FindSessionByUuid(obj_uuid);
            std::cout << "obj_id:" << obj_id << "\tobj_uuid:" << obj_uuid << std::endl;
            if (obj_session != nullptr) {
                obj_session->Send(msg.dump(), msg_id);
                // std::cout << "send msg:" << msg.dump() << std::endl;
            }
        } else {
            std::cout << "He/She is not online" << std::endl;
            // 先将消息存入服务器，等待对方上线
        }

        FileTools::GetInstance()->SaveTextMsg(uid, obj_id, msg_data, forward_state);
        UserManager::GetInstance()->AnswerFirstChat(uid, obj_id);
        UserManager::GetInstance()->FirstChat(uid, obj_id);
    } else {
        std::string answer = "sorry, you can't send messages to him/her.";
        msg["data"] = answer;
        session->Send(msg.dump(), MSG_TEXT_CHAT_REFUSED);
    }
}

void LogicSystem::TransmitMsg(std::shared_ptr<Session> session,
                              const short &msg_id,
                              const std::string &msg_data)
{
    std::cout << "TransmitMsg---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    std::string obj_uuid = UserManager::GetInstance()->GetUuidByUid(obj_id);
    if (obj_uuid != "") {
        auto obj_session = session->_server->FindSessionByUuid(obj_uuid);
        std::cout << "obj_id:" << obj_id << "\tobj_uuid:" << obj_uuid << std::endl;
        if (obj_session != nullptr) {
            obj_session->Send(msg.dump(), msg_id);
        }
    }
}

void LogicSystem::FollowCallBack(std::shared_ptr<Session> session,
                                 const short &msg_id,
                                 const std::string &msg_data)
{
    std::cout << "FollowCallBack---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    TransmitMsg(session, msg_id, msg_data);
    UserManager::GetInstance()->AddFollowRelation(uid, obj_id);
}

void LogicSystem::CancelFollowCallBack(std::shared_ptr<Session> session,
                                       const short &msg_id,
                                       const std::string &msg_data)
{
    std::cout << "CancelFollowCallBack---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    TransmitMsg(session, msg_id, msg_data);
    UserManager::GetInstance()->DeleteFollowRelation(uid, obj_id);
}

void LogicSystem::BlockCallBack(std::shared_ptr<Session> session,
                                const short &msg_id,
                                const std::string &msg_data)
{
    std::cout << "BlockCallBack---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    UserManager::GetInstance()->AddToBlacklist(uid, obj_id);
}

void LogicSystem::CancelBlockCallBack(std::shared_ptr<Session> session,
                                      const short &msg_id,
                                      const std::string &msg_data)
{
    std::cout << "CancelBlockCallBack---" << std::endl;

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    UserManager::GetInstance()->RemoveFromBlacklist(uid, obj_id);
}

void LogicSystem::AVChatCallBack(std::shared_ptr<Session> session,
                                 const short &msg_id,
                                 const std::string &msg_data)
{
    std::cout << "VideoChatCallBack---" << std::endl;

    // for (int i = 0; i < msg_data.length(); ++i) {
    //     std::cout << std::hex << (int) (unsigned char) msg_data[i] << " ";
    // }

    char first_char = msg_data[0];
    if (first_char != '{' && first_char != '[')
        return;

    nlohmann::json msg;
    if (!msg_data.empty()) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");

    std::string obj_uuid = UserManager::GetInstance()->GetUuidByUid(obj_id);
    if (obj_uuid != "") {
        auto obj_session = session->_server->FindSessionByUuid(obj_uuid);
        std::cout << "obj_id:" << obj_id << "\tobj_uuid:" << obj_uuid << std::endl;
        if (obj_session != nullptr) {
            obj_session->Send(msg.dump(), msg_id);
        }
    } else {
        session->Send(msg.dump(), MSG_NOT_ONLINE);
    }
}

void LogicSystem::RandomPushChatCallBack(std::shared_ptr<Session> session,
                                         const short &msg_id,
                                         const std::string &msg_data)
{
    std::cout << "RandomPushChatCallBack---" << std::endl;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    nlohmann::json user_info;
    std::unordered_set<unsigned int> random_users = GenerateRandomNumbers(20000000, 20000019, 5);
    for (auto user_id : random_users) {
        json tmp = UserManager::GetInstance()->GetUserInformation(user_id);
        std::string relation = std::to_string(
            UserManager::GetInstance()->GetRelation(msg["uid"], user_id));
        tmp["relation"] = relation;
        user_info[std::to_string(user_id)] = tmp;
        std::cout << user_info[std::to_string(user_id)] << std::endl;
    }
    msg["data"] = user_info.dump();
    session->Send(msg.dump(), MSG_RANDOM_PUSH);
}

void LogicSystem::OnlineStateCallBack(std::shared_ptr<Session> session,
                                      const short &msg_id,
                                      const std::string &msg_data)
{
    std::cout << "OnlineStateCallBack---" << std::endl;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int obj_id = msg.at("object_id");

    msg["data"] = UserManager::GetInstance()->GetOnlineState(obj_id);
    std::cout<<"onlinestate:"<<msg["data"]<<std::endl;
    session->Send(msg.dump(),MSG_ONLINE_STATE);
}

void LogicSystem::SendFileCallBack(std::shared_ptr<Session> session,
                                   const short &msg_id,
                                   const std::string &msg_data)
{
    std::cout << "SendFileCallBack---" << std::endl;

    nlohmann::json msg;
    if (msg_data.size() >= 4) {
        std::cout << "msg_data: " << msg_data << std::endl;
        msg = nlohmann::json::parse(msg_data);
    } else {
        std::cout << "msg size is 0" << std::endl;
        return;
    }

    unsigned int uid = msg.at("uid");
    unsigned int obj_id = msg.at("object_id");
    std::string str_data = msg.at("data");
    nlohmann::json data = nlohmann::json::parse(str_data);

    std::filesystem::path filename = data["filename"];
    size_t data_size = data["data_size"];
    std::string chunk_data=data["chunk_data"];

    FileTools::GetInstance()->SaveFileMsg(uid,obj_id,filename,chunk_data,data_size);    // 将文件存入本地

    TransmitMsg(session,MSG_FILE,msg_data);
}

// 生成指定区间范围内的整数随机数
std::unordered_set<unsigned int> LogicSystem::GenerateRandomNumbers(unsigned int min,
                                                                    unsigned int max,
                                                                    int count)
{
    std::unordered_set<unsigned int> randomNumbers;

    // 设置随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 生成随机数
    while (randomNumbers.size() < static_cast<size_t>(count)) {
        int randomNumber = min + (std::rand() % (max - min + 1)); // 生成 [min, max] 范围内的随机数
        randomNumbers.insert(randomNumber); // 插入随机数（自动去重）
    }

    return randomNumbers;
}
