#pragma once

#include "relationalbroker.h"
#include <nlohmann/json.hpp>

using namespace nlohmann;

class UserBroker : public RelationalBroker
{
public:
    UserBroker();
    json FindUser(unsigned int user_id);
    std::string findValueOfField(const mysqlpp::StoreQueryResult &user,
                                 const std::string fieldName,
                                 int rowIndex);
    json storeQueryResultToJson_Users(const mysqlpp::StoreQueryResult &user);

    void UpdateOnlineState(unsigned int uid, bool is_online, std::string datetime); // 下线
    void UpdateOnlineState(unsigned int uid, bool is_online);                       // 上线
    std::string LastOfflineTime(unsigned int uid);
    bool IsOnline(const unsigned int &uid);
    bool IsChatted(unsigned int uid, unsigned int object_id);
    void FirstChat(const unsigned int &uid, const unsigned int &object_id);
    unsigned int GetRelation(unsigned int uid, unsigned int object_id);
    bool GetChatPermission(const unsigned int &uid, const unsigned int &object_id);
    void UpdateChatPermission(unsigned int uid,
                              unsigned int object_id,
                              unsigned int relation, // 0:拉黑 1:陌生人 2:被关注 3:关注 4:互关
                              bool is_continue_chat);
    void UpdateChatPermission(unsigned int uid, unsigned int object_id, bool is_continue_chat);
    void UpdateChatPermission(unsigned int uid, unsigned int object_id, unsigned int relation);

    std::vector<unsigned int> FindChattedUsers(const unsigned int &uid);
    void AddFollowRelation(unsigned int &follower_id, unsigned int &followee_id);
    void DeleteFollowRelation(unsigned int &follower_id, unsigned int &followee_id);
    void AddToBlacklist(unsigned int &uid, unsigned int &blocked_id);
    void RemoveFromBlacklist(unsigned int &uid, unsigned int &blocked_id);

    std::vector<unsigned int> GetFollowingUserIDs(unsigned int &uid);
    std::vector<unsigned int> GetFollowerUserIDs(unsigned int &uid);
    std::vector<unsigned int> GetBlackListUserIDs(unsigned int &uid);
};
