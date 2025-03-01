#include "usermanager.h"

UserManager::UserManager()
{
    _user_broker = std::make_unique<UserBroker>();
}

UserManager::~UserManager() {}

std::string UserManager::GetUuidByUid(unsigned int uid)
{
    if (_connects.find(uid) == _connects.end())
        return "";
    return _connects.at(uid);
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

nlohmann::json UserManager::GainUserInformation(unsigned int uid)
{
    return _user_broker->FindUser(uid);
}
