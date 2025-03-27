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
    unsigned int GetUidByUuid(std::string uuid);
    void AddToConnectsList(unsigned int uid, std::string uuid);

    void AddFollowRelation(unsigned int& follower_id, unsigned int& followee_id);
    void DeleteFollowRelation(unsigned int& follower_id, unsigned int& followee_id);
    void AddToBlacklist(unsigned int& uid, unsigned int& blocked_id);
    void RemoveFromBlacklist(unsigned int& uid, unsigned int& blocked_id);

    nlohmann::json GetUserInformation(unsigned int uid);
    nlohmann::json GetFollowingUsersInfo(unsigned int uid);
    nlohmann::json GetFollowerUsersInfo(unsigned int uid);
    nlohmann::json GetBlackListUsersInfo(unsigned int uid);
    nlohmann::json GetChattedUsersInfo(unsigned int uid);

    void ClearUuid(std::string uuid);
    void DisconnectUser(std::string uuid);
    void ConnectUser(unsigned int uid, std::string uuid);
    std::string LastOfflineTime(unsigned int uid);
    std::vector<unsigned int> FindChattedUsers(const unsigned int& uid);
    void FirstChat(const unsigned int& uid, const unsigned int& object_id);
    bool IsChatPermitted(const unsigned int& uid, const unsigned int& object_id);
    void AnswerFirstChat(const unsigned int& uid, const unsigned int& object_id);
    unsigned int GetRelation(const unsigned int& uid, const unsigned int& object_id);

private:
    UserManager();

    std::unique_ptr<UserBroker> _user_broker;
    std::unordered_map<unsigned int, std::string>
        _connects; // 将已连接到服务器的用户uid与对应处理器session的uuid绑定
    std::vector<unsigned int> _all_uid; // 存储所有用户的uid
};
