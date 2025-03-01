#include "userbroker.h"

UserBroker::UserBroker() {}

// nlohmann::json UserBroker::GainAllUid()
// {
//     return;
// }

json UserBroker::FindUser(unsigned int user_id)
{
    std::string command;
    command = "select * from Users where UserID = " + std::to_string(user_id) + ";";
    mysqlpp::StoreQueryResult user = RelationalBroker::Query(command);
    // qDebug() << user.data();
    return storeQueryResultToJson_Users(user, "user_info");
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

json UserBroker::storeQueryResultToJson_Users(const mysqlpp::StoreQueryResult &user,
                                              const std::string msgType)
{
    json jsonArray;

    // 遍历每一行并将字段值存储到 JSON 对象中
    for (size_t i = 0; i < user.size(); ++i) {
        json userData;
        userData["msg_type"] = msgType;
        userData["UserID"] = findValueOfField(user, "UserID", i);
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
