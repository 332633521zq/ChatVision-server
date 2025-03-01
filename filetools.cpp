#include "filetools.h"
#include <ctime>
#include <iostream>

FileTools::FileTools() {}

FileTools::~FileTools() {}

bool FileTools::CreateDir(std::filesystem::path dir_path)
{
    // 创建目录
    if (!std::filesystem::exists(dir_path)) {
        if (!std::filesystem::create_directories(dir_path)) {
            std::cerr << "Failed to create directory: " << dir_path << std::endl;
            return false;
        }
    }
    return true;
}

bool FileTools::CreateFile(std::filesystem::path file_path)
{
    // 创建或打开文件
    std::ofstream file(file_path, std::ios::app); // 以追加模式打开文件
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    file.close();
    return true;
}

void FileTools::InitChatMsgFiles(unsigned int uid1, unsigned int uid2)
{
    std::filesystem::path root_path = std::filesystem::current_path() / "chatmsgs";
    std::string dir_name = std::to_string(uid1) + "_" + std::to_string(uid2);
    std::filesystem::path text_path = root_path / dir_name / "textmsg";
    std::filesystem::path picture_path = root_path / dir_name / "picture";
    std::filesystem::path video_path = root_path / dir_name / "video";
    std::filesystem::path audio_path = root_path / dir_name / "audio";

    std::cout << "\ntext_path: " << text_path << std::endl;
    std::cout << "picture_path" << picture_path << std::endl;
    std::cout << "video_path" << video_path << std::endl;
    std::cout << "audio_path" << audio_path << std::endl;

    CreateDir(text_path);
    CreateDir(picture_path);
    CreateDir(video_path);
    CreateDir(audio_path);
}

bool FileTools::SaveTextMsg(unsigned int uid1, unsigned int uid2, const std::string msg_data)
{
    // 判断是否两位用户是否有聊天记录
    std::string msg_dir = std::to_string(uid1) + "_" + std::to_string(uid2);
    std::string op_msg_dir = std::to_string(uid2) + "_" + std::to_string(uid1);

    std::filesystem::path dir_path = std::filesystem::current_path() / "chatmsgs" / msg_dir;
    std::filesystem::path op_dir_path = std::filesystem::current_path() / "chatmsgs" / op_msg_dir;

    if (!std::filesystem::exists(dir_path) && !std::filesystem::exists(op_dir_path)) {
        InitChatMsgFiles(uid1, uid2);
    }

    // 根据日期创建存储当天聊天消息的文件
    std::time_t now = std::time(nullptr);
    auto currentDate = std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(currentDate, "%Y-%m-%d");
    std::string curdate_str = ss.str();

    std::filesystem::path file_path = dir_path / "textmsg" / (curdate_str + ".txt");
    CreateFile(file_path);

    // 打开文件
    std::ofstream file(file_path, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    // 获取当前时间
    std::time_t nowtime = std::time(nullptr);
    std::string timestamp = std::ctime(&nowtime);
    timestamp.pop_back(); // 去掉换行符

    // 写入消息
    file << "[" << timestamp << "]: " << msg_data << std::endl;
    file.close();

    return true;
}

// std::vector<std::string> FileTools::GetTextMsg(unsigned int uid1, unsigned int uid2)
// {
//     return;
// }
