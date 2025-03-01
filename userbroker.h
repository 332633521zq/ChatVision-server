#pragma once

#include "relationalbroker.h"
#include <nlohmann/json.hpp>

using namespace nlohmann;

class UserBroker : public RelationalBroker
{
public:
    UserBroker();
    nlohmann::json GainAllUid();
    json FindUser(unsigned int user_id);
    std::string findValueOfField(const mysqlpp::StoreQueryResult &user,
                                 const std::string fieldName,
                                 int rowIndex);
    json storeQueryResultToJson_Users(const mysqlpp::StoreQueryResult &user,
                                      const std::string msgType);
};
