#include "usermanager.h"
#include "ConstValue.h"
#include <chrono>
#include <format>

UserManager::UserManager()
{
    _user_broker = std::make_unique<UserBroker>();
}

UserManager::~UserManager() {}

std::string UserManager::GetUuidByUid(unsigned int uid)
{
    // 同时开多个客户端（uid相同）时，客户端只有最先建立的连接能受到消息，
    // 因为map查找只会返回找到的第一个键对应的值，而插入操作时用的insert()函数,
    // 不会覆盖之前插入的key相同的键值对(相同的key在理论上不合法)，而是直接丢弃

    if (_connects.find(uid) == _connects.end())
        return "";
    return _connects.at(uid);
}

unsigned int UserManager::GetUidByUuid(std::string uuid)
{
    auto find_item = std::find_if(_connects.begin(), _connects.end(), [uuid](const auto& item) {
        return item.second == uuid;
    });
    if (find_item != _connects.end()) {
        return find_item->first;
    }
    return 0;
}

void UserManager::ClearUuid(const std::string uuid)
{
    unsigned int uid = GetUidByUuid(uuid);
    if (uid) {
        _connects.erase(uid);
    }
}

std::string UserManager::LastOfflineTime(unsigned int uid)
{
    return _user_broker->LastOfflineTime(uid);
}

void UserManager::DisconnectUser(std::string uuid)
{
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::cout << std::format("{:%Y-%m-%d %H:%M:%S}", now_seconds) << std::endl;
    _user_broker->UpdateOnlineState(GetUidByUuid(uuid),
                                    false,
                                    std::format("{:%Y-%m-%d %H:%M:%S}", now_seconds));
    ClearUuid(uuid);
}

void UserManager::ConnectUser(unsigned int uid, std::string uuid)
{
    AddToConnectsList(uid, uuid);
    _user_broker->UpdateOnlineState(GetUidByUuid(uuid), true);
}

void UserManager::AddToConnectsList(unsigned int uid, std::string uuid)
{
    if (_connects.find(uid) == _connects.end()) {
        _connects.insert(std::make_pair(uid, uuid));
        std::cout << "AddToConnects: \t uid:" << uid << "\t uuid" << uuid << std::endl;
        return;
    }
    // std::cout << "uid not exists\n" << std::endl;
}

nlohmann::json UserManager::GetUserInformation(unsigned int uid)
{
    return _user_broker->FindUser(uid);
}

nlohmann::json UserManager::GetFollowingUsersInfo(unsigned int uid)
{
    // 获取关注的 UserID 列表
    std::vector<unsigned int> user_ids = _user_broker->GetFollowingUserIDs(uid);

    // 根据 UserID 列表获取用户信息
    nlohmann::json users_info;
    for (auto uid : user_ids) {
        std::cout << "following user id:" << uid << std::endl;
        if (_user_broker->FindUser(uid) == NULL)
            continue;
        users_info.push_back(_user_broker->FindUser(uid));
    }

    return users_info;
}

nlohmann::json UserManager::GetFollowerUsersInfo(unsigned int uid)
{
    // 获取关注的 UserID 列表
    std::vector<unsigned int> user_ids = _user_broker->GetFollowerUserIDs(uid);

    // 根据 UserID 列表获取用户信息
    nlohmann::json users_info;
    for (auto i : user_ids) {
        if (_user_broker->FindUser(i) == NULL)
            continue;
        users_info.push_back(_user_broker->FindUser(i));
    }

    return users_info;
}

nlohmann::json UserManager::GetBlackListUsersInfo(unsigned int uid)
{
    // 获取关注的 UserID 列表
    std::vector<unsigned int> user_ids = _user_broker->GetBlackListUserIDs(uid);

    // 根据 UserID 列表获取用户信息
    nlohmann::json users_info;
    for (auto i : user_ids) {
        if (_user_broker->FindUser(i) == NULL)
            continue;
        users_info.push_back(_user_broker->FindUser(i));
    }

    return users_info;
}

std::vector<unsigned int> UserManager::FindChattedUsers(const unsigned int& uid)
{
    return _user_broker->FindChattedUsers(uid);
}

void UserManager::FirstChat(const unsigned int& uid, const unsigned int& object_id)
{
    _user_broker->FirstChat(uid, object_id);
}

bool UserManager::IsChatPermitted(const unsigned int& uid, const unsigned int& object_id)
{
    return _user_broker->GetChatPermission(object_id, uid);
}

void UserManager::AddFollowRelation(unsigned int& follower_id, unsigned int& followee_id)
{
    _user_broker->AddFollowRelation(follower_id, followee_id);
    if (_user_broker->IsChatted(follower_id, followee_id)) {
        auto relation = _user_broker->GetRelation(follower_id, followee_id);
        _user_broker->UpdateChatPermission(follower_id, followee_id, relation, true);
        relation = _user_broker->GetRelation(followee_id, follower_id);
        _user_broker->UpdateChatPermission(followee_id, follower_id, relation);
    }
}

void UserManager::DeleteFollowRelation(unsigned int& follower_id, unsigned int& followee_id)
{
    _user_broker->DeleteFollowRelation(follower_id, followee_id);
    if (_user_broker->IsChatted(follower_id, followee_id)) {
        auto relation = _user_broker->GetRelation(follower_id, followee_id);
        _user_broker->UpdateChatPermission(follower_id, followee_id, relation);
        relation = _user_broker->GetRelation(followee_id, follower_id);
        _user_broker->UpdateChatPermission(followee_id, follower_id, relation);
    }
}

void UserManager::AddToBlacklist(unsigned int& uid, unsigned int& blocked_id)
{
    _user_broker->AddToBlacklist(uid, blocked_id);
    if (_user_broker->IsChatted(uid, blocked_id)) {
        auto relation = _user_broker->GetRelation(uid, blocked_id);
        _user_broker->UpdateChatPermission(uid, blocked_id, relation, false);
        relation = _user_broker->GetRelation(blocked_id, uid);
        _user_broker->UpdateChatPermission(blocked_id, uid, relation);
    }
}

void UserManager::RemoveFromBlacklist(unsigned int& uid, unsigned int& blocked_id)
{
    _user_broker->RemoveFromBlacklist(uid, blocked_id);
    if (_user_broker->IsChatted(uid, blocked_id)) {
        auto relation = _user_broker->GetRelation(uid, blocked_id);
        _user_broker->UpdateChatPermission(uid, blocked_id, relation, true);
        relation = _user_broker->GetRelation(blocked_id, uid);
        _user_broker->UpdateChatPermission(blocked_id, uid, relation);
    }
}

void UserManager::AnswerFirstChat(const unsigned int& uid, const unsigned int& object_id)
{
    auto relation = _user_broker->GetRelation(uid, object_id);
    bool permission = _user_broker->GetChatPermission(uid, object_id);
    if ((relation >= RELATION_STRANGER) && !permission) {
        std::cout << "AnswerFirstChat:" << std::endl;
        _user_broker->UpdateChatPermission(uid, object_id, true);
    }
}

json UserManager::GetChattedUsersInfo(unsigned int uid)
{
    json chatted_users;
    auto chatted_uid = FindChattedUsers(uid);
    for (auto i : chatted_uid) {
        chatted_users.push_back(_user_broker->FindUser(i));
    }
    std::cout << "chatted users:" << chatted_users << std::endl;
    return chatted_users;
}
