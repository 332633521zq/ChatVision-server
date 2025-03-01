#include "relationalbroker.h"
#include <iostream>
std::unique_ptr<mysqlpp::Connection> RelationalBroker::m_connection = NULL;

RelationalBroker::RelationalBroker()
{
    //init connection
    std::unique_ptr<mysqlpp::Connection> conn = std::make_unique<mysqlpp::Connection>(false);
    if (conn->connect("ChatWorld", "localhost", "root", "9785"))
        std::cout << "Connect Sucessfully" << std::endl;
    else
        std::cout << "Connect Unsucessfully" << std::endl;
    m_connection = std::move(conn);
}

void RelationalBroker::InitDataBase()
{
    RelationalBroker();
    //create table
    try {
        Query("CREATE TABLE Users ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "U_Nickname VARCHAR(20), "
              "U_Avater VARCHAR(50),"
              "U_Gender CHAR(1),"
              "U_Area VARCHAR(20),"
              "U_Signature VARCHAR(30)"
              ")"
              "AUTO_INCREMENT = 5;");

        Query("INSERT INTO Users "
              "(UserID, U_Nickname, U_Avater, U_Gender, U_Area, U_Signature)VALUES"
              "(20000000, '85', 'path', '女', '重庆', '罪恶没有假期，正义便无暇休憩'),"
              " (20000001, '坐看云起时', 'path', '女', '重庆', '以雷霆击碎黑暗'),  "
              "(20000002, 'hahaha', 'path', '女', '重庆', '我叫hahaha');");

        Query("CREATE TABLE 20000000_Following ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");
        Query("CREATE TABLE 20000000_Followers ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");

        Query("CREATE TABLE 20000001_Following ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");
        Query("CREATE TABLE 20000001_Followers ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");

        Query("CREATE TABLE 20000002_Following ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");
        Query("CREATE TABLE 20000002_Followers ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "BuildTime DATETIME);");

    } catch (mysqlpp::Exception e) {
        std::cout << e.what();
    }
}

mysqlpp::StoreQueryResult RelationalBroker::Query(std::string command)
{
    std::cerr << "正在查询数据库的信息 " << std::endl;
    mysqlpp::StoreQueryResult res;
    try {
        mysqlpp::Query query = m_connection->query();
        query << command;
        res = query.store();
    } catch (mysqlpp::Exception e) {
        std::cerr << "Error selecting tasks: " << e.what() << std::endl;
    }
    //显示数据
    mysqlpp::StoreQueryResult::const_iterator it; // 迭代器
    size_t numFields = res.num_fields();
    std::cerr << "成功查询数据库的信息！" << std::endl;
    for (it = res.begin(); it != res.end(); ++it) {
        for (size_t j = 0; j < numFields; ++j) {
            mysqlpp::Row row = *it;
            // 获取每个字段的值并进行相应处理
            std::cout << row[j] << "\t";
        }
        std::cout << std::endl;
    }
    return res;
}

void RelationalBroker::Update(std::string command)
{
    try {
        mysqlpp::Query query = m_connection->query();
        query << command;
        query.store();
    } catch (std::exception e) {
        std::cerr << "Error selecting tasks: " << e.what() << std::endl;
    }
}
