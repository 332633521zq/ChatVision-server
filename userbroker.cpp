#include "userbroker.h"
#include "ConstValue.h"
#include <chrono>
#include <ctime>

UserBroker::UserBroker() {}

json UserBroker::FindUser(unsigned int user_id)
{
    std::string command;
    command = "select * from Users where UserID = " + std::to_string(user_id) + ";";
    mysqlpp::StoreQueryResult user = RelationalBroker::Query(command);
    return storeQueryResultToJson_Users(user);
}

std::string UserBroker::findValueOfField(const mysqlpp::StoreQueryResult &user,
                                         const std::string fieldName,
                                         int rowIndex)
{
    std::string fieldValue;

    if (!user.empty() && rowIndex >= 0 && rowIndex < user.size()) {
        // 使用指定行的字段值
        const mysqlpp::Row &currentRow = user[rowIndex];

        // 使用字段名获取字段的索引
        int fieldIndex = user.field_num(fieldName);

        // 检查字段是否存在
        if (fieldIndex != -1) {
            // 获取字段值
            fieldValue = currentRow[fieldIndex].data();
            // 输出字段的值
            // std::cout << "Value of field " << fieldName << ": " << fieldValue << std::endl;
        } else {
            std::cerr << "Field '" << fieldName << "' not found in the result set." << std::endl;
        }
    } else {
        std::cerr << "Result set is empty or row index is out of range." << std::endl;
    }

    return fieldValue;
}

json UserBroker::storeQueryResultToJson_Users(const mysqlpp::StoreQueryResult &user)
{
    json jsonArray;

    // 遍历每一行并将字段值存储到 JSON 对象中
    for (size_t i = 0; i < user.size(); ++i) {
        json userData;
        userData["uid"] = findValueOfField(user, "UserID", i);
        userData["nickname"] = findValueOfField(user, "U_Nickname", i);
        userData["avatar_path_"] = findValueOfField(user, "U_Avater", i);
        userData["gender"] = findValueOfField(user, "U_Gender", i);
        userData["area"] = findValueOfField(user, "U_Area", i);
        userData["signature"] = findValueOfField(user, "U_Signature", i);
        userData["memo"]
            = findValueOfField(user,
                               "U_Nickname",
                               i); // 注意：此处 fieldName 使用了 "U_Nickname"，而不是 "memo"

        jsonArray = userData;
    }

    // qDebug() << jsonArray.dump();
    return jsonArray;
}

void UserBroker::UpdateOnlineState(unsigned int uid, bool is_online, std::string datetime)
{
    std::string command = "update OnlineState set IsOnline=" + std::to_string(is_online)
                          + ", LastOffline='" + datetime + "' where (UserID=" + std::to_string(uid)
                          + ");";
    std::cout << command << std::endl;
    RelationalBroker::Query(command);
}

void UserBroker::UpdateOnlineState(unsigned int uid, bool is_online)
{
    std::string command = "update OnlineState set IsOnline=" + std::to_string(is_online)
                          + " where (UserID=" + std::to_string(uid) + ");";
    // "update OnlineState set IsOnline=false, LastOffline="2020-02-02 06:06:06" where (UserID=20000000);"
    RelationalBroker::Query(command);
}

std::string UserBroker::LastOfflineTime(unsigned int uid)
{
    std::string command = "select LastOffline from OnlineState where (UserID=" + std::to_string(uid)
                          + ");";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0) {
        mysqlpp::Row row = result[0];
        std::string lastOffline = row["LastOffline"].c_str();
        return lastOffline;
    }
    return "";
}

bool UserBroker::IsOnline(const unsigned int &uid)
{
    std::string command = "select IsOnline from OnlineState where (UserID=" + std::to_string(uid)
            + ");";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);
    return (const int)result[0]["IsOnline"] == 1;
}

// blacklist  obj_blacklist   follower following er&&ing
// 0:拉黑  1:被拉黑   2:陌生人   3:被关注   4:关注    5:互关
unsigned int UserBroker::GetRelation(unsigned int uid, unsigned int object_id)
{
    std::string uid_str = std::to_string(uid);
    std::string objid_str = std::to_string(object_id);

    std::string command = "select UserID from" + uid_str + "_Blacklist where (UserID=" + objid_str
                          + ");";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0)
        return RELATION_BLOCK;

    command = "select UserID from" + objid_str + "_Blacklist where (UserID=" + uid_str + ");";
    result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0)
        return RELATION_BLOCKED;

    bool following = false, follower = false;
    command = "select UserID from" + uid_str + "_Following where (UserID=" + objid_str + ");";
    result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0)
        following = true;

    command = "select UserID from" + uid_str + "_Follower where (UserID=" + objid_str + ");";
    result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0)
        follower = true;

    if (following && follower)
        return RELATION_INTERACT;
    if (following)
        return RELATION_FOLLOWING;
    if (follower)
        return RELATION_FOLLOWER;
    return RELATION_STRANGER;
}

bool UserBroker::IsChatted(unsigned int uid, unsigned int object_id)
{
    std::string command = "select UserID from " + std::to_string(uid)
                          + "_Chatted where(UserID=" + std::to_string(object_id) + ");";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);
    if (result && result.num_rows() > 0)
        return true;
    return false;
}

// 如果发送者uid在发送第一场聊天之前就已经被接收者object_id拉黑，则跳过该函数，且将被拉黑的情况返回给uid
void UserBroker::FirstChat(const unsigned int &uid, const unsigned int &object_id)
{
    unsigned int relation = GetRelation(uid, object_id);
    bool is_continue = relation > RELATION_BLOCK ? true : false;
    if (IsChatted(uid, object_id) || !is_continue) {
        return;
    }
    std::string command = "insert into " + std::to_string(uid)
                          + "_Chatted (UserID,Relation,IsContinue)VALUES("
                          + std::to_string(object_id) + "," + std::to_string(relation) + ","
                          + std::to_string(is_continue) + ")";
    RelationalBroker::Query(command);

    relation = GetRelation(object_id, uid);
    is_continue = relation > RELATION_FOLLOWER ? true : false;
    command = "insert into " + std::to_string(object_id)
              + "_Chatted (UserID,Relation,IsContinue)VALUES(" + std::to_string(uid) + ","
              + std::to_string(relation) + "," + std::to_string(is_continue) + ")";
    RelationalBroker::Query(command);
}

void UserBroker::UpdateChatPermission(unsigned int uid,
                                      unsigned int object_id,
                                      unsigned int relation,
                                      bool is_continue_chat)
{
    std::string command = "update " + std::to_string(uid)
                          + "_Chatted set Relation = " + std::to_string(relation)
                          + ",IsContinue = " + std::to_string(is_continue_chat)
                          + "where UserID=" + std::to_string(object_id) + ";";
    RelationalBroker::Query(command);
}

void UserBroker::UpdateChatPermission(unsigned int uid,
                                      unsigned int object_id,
                                      unsigned int relation)
{
    std::string command = "update " + std::to_string(uid)
                          + "_Chatted set Relation = " + std::to_string(relation)
                          + " where UserID=" + std::to_string(object_id) + ";";
    RelationalBroker::Query(command);
}

void UserBroker::UpdateChatPermission(unsigned int uid,
                                      unsigned int object_id,
                                      bool is_continue_chat)
{
    std::string command = "update " + std::to_string(uid)
                          + "_Chatted set IsContinue = " + std::to_string(is_continue_chat)
                          + " where UserID=" + std::to_string(object_id) + ";";
    // std::cout << "command: " << command << std::endl;
    RelationalBroker::Query(command);
}

std::vector<unsigned int> UserBroker::FindChattedUsers(const unsigned int &uid)
{
    std::string command = "select UserID from " + std::to_string(uid) + "_Chatted;";
    mysqlpp::StoreQueryResult users = RelationalBroker::Query(command);
    std::vector<unsigned int> result;
    if (users && users.num_rows() > 0) {
        for (size_t i = 0; i < users.size(); ++i) {
            mysqlpp::Row row = users[i];
            result.push_back(row["UserID"]);
        }
    }
    return result;
}

bool UserBroker::GetChatPermission(const unsigned int &uid, const unsigned int &object_id)
{
    if (IsChatted(uid, object_id)) {
        std::string command = "select IsContinue from " + std::to_string(uid)
                              + "_Chatted where UserID=" + std::to_string(object_id) + " ;";
        mysqlpp::StoreQueryResult is_continue = RelationalBroker::Query(command);
        if (is_continue && is_continue.num_rows() > 0) {
            mysqlpp::Row &row = is_continue[0];
            return row["IsContinue"];
        }
    } else if (GetRelation(uid, object_id) >= RELATION_STRANGER) {
        return true;
    }
    return false;
}

void UserBroker::AddFollowRelation(unsigned int &follower_id, unsigned int &followee_id)
{
    auto nowtime = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(nowtime);
    std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now_seconds);

    std::string command = "insert into " + std::to_string(followee_id)
                          + "_Followers (UserID,BuildTime)VALUES (" + std::to_string(follower_id)
                          + ",'" + timestamp + "');";
    RelationalBroker::Query(command);

    command = "insert into " + std::to_string(follower_id) + "_Following (UserID,BuildTime)VALUES ("
              + std::to_string(followee_id) + ",'" + timestamp + "');";
    RelationalBroker::Query(command);

    // 检查是否已经存在互相关注的关系
    command = "select UserID from " + std::to_string(follower_id)
              + "_Followers where UserID = " + std::to_string(followee_id) + ";";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);

    // 如果存在互相关注的关系，更新 Interact 表
    if (result && result.num_rows() > 0) {
        // 在 follower_id 的 Interact 表中添加 followee_id
        command = "insert into " + std::to_string(follower_id)
                  + "_Interact (UserID, BuildTime) VALUES (" + std::to_string(followee_id) + ", '"
                  + timestamp + "');";
        RelationalBroker::Query(command);

        // 在 followee_id 的 Interact 表中添加 follower_id
        command = "insert into " + std::to_string(followee_id)
                  + "_Interact (UserID, BuildTime) VALUES (" + std::to_string(follower_id) + ", '"
                  + timestamp + "');";
        RelationalBroker::Query(command);
    }
}

void UserBroker::DeleteFollowRelation(unsigned int &follower_id, unsigned int &followee_id)
{
    std::string command = "delete from " + std::to_string(followee_id)
                          + "_Followers where UserID = " + std::to_string(follower_id) + ";";
    RelationalBroker::Query(command);

    command = "delete from " + std::to_string(follower_id)
              + "_Following where UserID = " + std::to_string(followee_id) + ";";
    RelationalBroker::Query(command);

    command = "select UserID from " + std::to_string(follower_id)
              + "_Followers where UserID = " + std::to_string(followee_id) + ";";
    mysqlpp::StoreQueryResult result = RelationalBroker::Query(command);

    // 如果不存在互相关注的关系，更新 Interact 表
    if (!result || result.num_rows() == 0) {
        // 从 follower_id 的 Interact 表中删除 followee_id
        command = "delete from " + std::to_string(follower_id)
                  + "_Interact where UserID = " + std::to_string(followee_id) + ";";
        RelationalBroker::Query(command);

        // 从 followee_id 的 Interact 表中删除 follower_id
        command = "delete from " + std::to_string(followee_id)
                  + "_Interact where UserID = " + std::to_string(follower_id) + ";";
        RelationalBroker::Query(command);
    }
}

void UserBroker::AddToBlacklist(unsigned int &uid, unsigned int &blocked_id)
{
    // 获取当前时间戳
    auto nowtime = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(nowtime);
    std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now_seconds);

    std::string command = "insert into " + std::to_string(uid)
                          + "_Blacklist (UserID, BuildTime) VALUES (" + std::to_string(blocked_id)
                          + ", '" + timestamp + "');";

    if (!RelationalBroker::Query(command)) {
        std::cerr << "添加到黑名单失败: " << command << std::endl;
    }
}

void UserBroker::RemoveFromBlacklist(unsigned int &uid, unsigned int &blocked_id)
{
    std::string command = "delete from " + std::to_string(uid)
                          + "_Blacklist where UserID = " + std::to_string(blocked_id) + ";";

    if (!RelationalBroker::Query(command)) {
        std::cerr << "从黑名单移除失败: " << command << std::endl;
    }
}

// 从 uid_Following 表中查找所有 UserID
std::vector<unsigned int> UserBroker::GetFollowingUserIDs(unsigned int &uid)
{
    std::vector<unsigned int> userIDs;

    std::string following_query = "SELECT UserID FROM " + std::to_string(uid) + "_Following;";
    mysqlpp::StoreQueryResult res = RelationalBroker::Query(following_query);

    if (!res || res.num_rows() == 0) {
        std::cerr << "未找到关注用户或查询失败" << std::endl;
        return userIDs; // 返回空列表
    }

    for (size_t i = 0; i < res.num_rows(); ++i) {
        unsigned int followingUID = res[i]["UserID"];
        userIDs.push_back(followingUID);
    }

    return userIDs;
}

std::vector<unsigned int> UserBroker::GetFollowerUserIDs(unsigned int &uid)
{
    std::vector<unsigned int> userIDs;

    std::string follower_query = "SELECT UserID FROM " + std::to_string(uid) + "_Followers;";
    mysqlpp::StoreQueryResult res = RelationalBroker::Query(follower_query);

    if (!res || res.num_rows() == 0) {
        std::cerr << "未找到关注用户或查询失败" << std::endl;
        return userIDs; // 返回空列表
    }

    for (size_t i = 0; i < res.num_rows(); ++i) {
        unsigned int followerUID = res[i]["UserID"];
        userIDs.push_back(followerUID);
    }

    return userIDs;
}

std::vector<unsigned int> UserBroker::GetBlackListUserIDs(unsigned int &uid)
{
    std::vector<unsigned int> userIDs;

    std::string block_query = "SELECT UserID FROM " + std::to_string(uid) + "_Blacklist;";
    mysqlpp::StoreQueryResult res = RelationalBroker::Query(block_query);

    if (!res || res.num_rows() == 0) {
        std::cerr << "未找到关注用户或查询失败" << std::endl;
        return userIDs; // 返回空列表
    }

    for (size_t i = 0; i < res.num_rows(); ++i) {
        unsigned int uid = res[i]["UserID"];
        userIDs.push_back(uid);
    }

    return userIDs;
}
