#pragma once

#include "nlohmann/json.hpp"
#include "singleton.h"
#include "userbroker.h"
#include <map>
#include <string>
#include <vector>

class UserManager : public Singleton<UserManager>
{
    friend class Singleton<UserManager>;

public:
    ~UserManager();
    std::string GetUuidByUid(unsigned int uid);
    void AddToConnectsList(unsigned int uid, std::string uuid);

    void AddFollowRelation(unsigned int uid, unsigned int follower_id);
    void DeleteFollowRelation(unsigned int uid, unsigned int follower_id);

    nlohmann::json GainUserInformation(unsigned int uid);

private:
    UserManager();

    std::unique_ptr<UserBroker> _user_broker;
    std::map<unsigned int, std::string>
        _connects; // 将已连接到服务器的用户uid与对应处理器session的uuid绑定
    std::vector<unsigned int> _all_uid; // 存储所有用户的uid
};
