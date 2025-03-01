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

private:
    static std::unique_ptr<mysqlpp::Connection> m_connection;
};
