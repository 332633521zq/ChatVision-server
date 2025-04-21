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
    bool SaveTextMsg(unsigned int uid1,
                     unsigned int uid2,
                     std::string msg_data,
                     bool is_forwarded = false);
    bool SaveFileMsg(unsigned int uid1,
                     unsigned int uid2,
                     std::filesystem::path file_name,
                     const std::string file_content, size_t length);
    std::vector<std::string> GetOfflineTextMsg(unsigned int conn_uid, unsigned int uid2);
    std::filesystem::path GetLatestModifiedFile(const std::string& directory);
    std::time_t GetFileLatestModifyTime(std::filesystem::path file_path);
    void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
    void GetFiles(const std::string& directory,
                  std::vector<std::string>& files,
                  const std::string& datetime);

private:
    FileTools();
};
