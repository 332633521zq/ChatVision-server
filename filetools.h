#pragma once

#include "nlohmann/json.hpp"
#include "singleton.h"
#include <filesystem>
#include <fstream>
#include <vector>

class FileTools : public Singleton<FileTools>
{
    friend class Singleton<FileTools>;

public:
    ~FileTools();

    bool CreateDir(std::filesystem::path dir_path);
    bool CreateFile(std::filesystem::path file_path);
    void InitChatMsgFiles(unsigned int uid1, unsigned int uid2);
    bool SaveTextMsg(unsigned int uid1, unsigned int uid2, std::string msg_data);
    std::vector<std::string> GetTextMsg(unsigned int uid1, unsigned int uid2);

private:
    FileTools();
};
