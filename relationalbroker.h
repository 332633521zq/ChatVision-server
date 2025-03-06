#pragma once

#include "singleton.h"
#include <mysql++/mysql++.h>

class RelationalBroker
{
public:
    RelationalBroker();
    static void InitDataBase();

    static mysqlpp::StoreQueryResult Query(std::string command);
    static void Update(std::string command);
    static void CreateUserTable(unsigned int uid);
    static void CreateUsersTable(unsigned int uid);
    static void InsertIntoOnlineState(int user_num);

private:
    static std::unique_ptr<mysqlpp::Connection> m_connection;
};
