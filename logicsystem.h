#pragma once

#include "ConstValue.h"
#include "session.h"
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <unordered_set>

#include "logicnode.h"
#include "singleton.h"

typedef std::function<void(std::shared_ptr<Session>, const short& msg_id, const std::string& msg_data)>
    FunCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
    void RegisterCallBacks();
    void DealMsg();

    std::unordered_set<unsigned int> GenerateRandomNumbers(unsigned int min,
                                                           unsigned int max,
                                                           int count);

    void HelloWorldCallBack(std::shared_ptr<Session>,
                            const short& msg_id,
                            const std::string& msg_data);
    void LoginCallBack(std::shared_ptr<Session> session,
                       const short& msg_id,
                       const std::string& msg_data);
    void TextChatCallBack(std::shared_ptr<Session> session,
                          const short& msg_id,
                          const std::string& msg_data);
    void FollowCallBack(std::shared_ptr<Session> session,
                        const short& msg_id,
                        const std::string& msg_data);
    void CancelFollowCallBack(std::shared_ptr<Session> session,
                              const short& msg_id,
                              const std::string& msg_data);
    void BlockCallBack(std::shared_ptr<Session> session,
                       const short& msg_id,
                       const std::string& msg_data);
    void CancelBlockCallBack(std::shared_ptr<Session> session,
                             const short& msg_id,
                             const std::string& msg_data);
    void VideoChatCallBack(std::shared_ptr<Session> session,
                           const short& msg_id,
                           const std::string& msg_data);
    void RefuseVideoChatCallBack(std::shared_ptr<Session> session,
                                 const short& msg_id,
                                 const std::string& msg_data);
    void RandomPushChatCallBack(std::shared_ptr<Session> session,
                                const short& msg_id,
                                const std::string& msg_data);

private:
    LogicSystem();

    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex; // 逻辑线程和网络线程都可能会访问该队列，需要加锁
    std::condition_variable
        _consume; // 当队列_msg_que为空，则挂起该线程，_consume用于标志_msg_que的状态
    std::thread _worker_thread; //从队列中取数据进行处理的工作线程
    bool _b_stop = false; // 当_b_stop == true,取出_msg_que中所有数据，处理完毕后关闭该线程

    std::map<short, FunCallBack> _fun_callback; // 将msg_id和回调函数绑定
};
