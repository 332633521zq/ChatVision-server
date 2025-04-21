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

void RelationalBroker::InsertIntoOnlineState(int user_num)
{
    unsigned int start_uid = 20000000;
    std::string command;
    for (int i = 0; i < user_num; i++) {
        command = "INSERT INTO OnlineState "
                  "(UserID, IsOnline, LastOffline)VALUES("
                  + std::to_string(start_uid + i) + ", false, '1900-01-01 00:00:00');";
        Query(command);
    }
}

void RelationalBroker::CreateUserTable(unsigned int uid)
{
    Query("CREATE TABLE " + std::to_string(uid)
          + "_Following ("
            "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
            "BuildTime DATETIME);");
    Query("CREATE TABLE " + std::to_string(uid)
          + "_Followers ("
            "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
            "BuildTime DATETIME);");
    Query("CREATE TABLE " + std::to_string(uid)
          + "_Blacklist ("
            "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
            "BuildTime DATETIME);");
    Query("CREATE TABLE " + std::to_string(uid)
          + "_Interact ("
            "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
            "BuildTime DATETIME);");
    Query("CREATE TABLE " + std::to_string(uid)
          + "_Chatted ("
            "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
            "Relation INT UNSIGNED,"
            "IsContinue BOOL);");
}

void RelationalBroker::CreateUsersTable(unsigned int user_num)
{
    unsigned int start_uid = 20000000;
    for (int i = 0; i < user_num; i++) {
        CreateUserTable(start_uid + i);
    }
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

        Query(
            "INSERT INTO Users "
            "(UserID, U_Nickname, U_Avater, U_Gender, U_Area, U_Signature)VALUES"
            "(20000000, '85', 'qrc:/image/avatar/3.jpg', '女', '重庆', "
            "'罪恶没有假期，正义便无暇休憩'),"
            "(20000001, '坐看云起时', 'qrc:/image/avatar/24.jpg', '女', '重庆', "
            "'以雷霆击碎黑暗'),  "
            "(20000002, '盎司及', 'qrc:/image/avatar/2.jpg', '男', '河北', '盎司及多家凯撒'),  "
            "(20000003, 'hahaha', 'qrc:/image/avatar/30.jpg', '女', '重庆', '我叫hahaha'),"
            "(20000004, '代码敲到手抽筋', 'qrc:/image/avatar/31.jpg', '男', '北京', "
            "'热爱生活，热爱编程'),"
            "(20000005, '熬夜冠军', 'qrc:/image/avatar/32.jpg', '女', '上海', "
            "'追逐梦想，永不止步'),"
            "(20000006, '可乐不加冰', 'qrc:/image/avatar/33.jpg', '男', '广州', '技术改变世界'),"
            "(20000007, '吃瓜群众', 'qrc:/image/avatar/34.jpg', '女', '深圳', "
            "'代码如诗，逻辑如画'),"
            "(20000008, '香菜终结者', 'qrc:/image/avatar/35.jpg', '男', '杭州', "
            "'探索未知，勇往直前'),"
            "(20000009, '量子波动速读', 'qrc:/image/avatar/36.jpg', '女', '成都', "
            "'热爱美食，热爱生活'),"
            "(20000010, '404NotFound', 'qrc:/image/avatar/37.jpg', '男', '武汉', "
            "'坚持不懈，终会成功'),"
            "(20000011, '深海孤鲸', 'qrc:/image/avatar/38.jpg', '女', '西安', '历史与现代的交融'),"
            "(20000012, '迷雾森林', 'qrc:/image/avatar/39.jpg', '男', '南京', "
            "'六朝古都，文化底蕴'),"
            "(20000013, '星空守望者', 'qrc:/image/avatar/40.jpg', '女', '苏州', "
            "'园林之城，人间天堂'),"
            "(20000014, '幻夜星辰', 'qrc:/image/avatar/44.jpg', '男', '天津', "
            "'海河之滨，魅力之城'),"
            "(20000015, '键盘侠本侠', 'qrc:/image/avatar/45.jpg', '女', '重庆', "
            "'山城重庆，火锅之都'),"
            "(20000016, 'Bug制造机', 'qrc:/image/avatar/46.jpg', '男', '长沙', "
            "'岳麓山下，橘子洲头'),"
            "(20000017, '阳光总在风雨后', 'qrc:/image/avatar/47.jpg', '女', '郑州', "
            "'中原大地，文化之源'),"
            "(20000018, '月下独酌', 'qrc:/image/avatar/48.jpg', '男', '青岛', "
            "'海滨之城，啤酒之都'),"
            "(20000019, '流年似水', 'qrc:/image/avatar/49.jpg', '女', '大连', '浪漫之都，时尚之城')"
            ";");

        Query("CREATE TABLE OnlineState ("
              "UserID INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
              "IsOnline bool,"
              "LastOffline DATETIME);");

        InsertIntoOnlineState(20);
        // Query("INSERT INTO OnlineState "
        //       "(UserID, IsOnline, LastOffline)VALUES"
        //       "(20000000, false, '1900-01-01 00:00:00'),"
        //       "(20000001, false, '1900-01-01 00:00:00'),"
        //       "(20000002, false, '1900-01-01 00:00:00'),"
        //       "(20000003, false, '1900-01-01 00:00:00');");

        CreateUsersTable(20);

    } catch (mysqlpp::Exception e) {
        std::cout << e.what();
    }
}

mysqlpp::StoreQueryResult RelationalBroker::Query(std::string command)
{
    // std::cerr << "正在查询数据库的信息 " << std::endl;
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
    // std::cerr << "成功查询数据库的信息！" << std::endl;
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
