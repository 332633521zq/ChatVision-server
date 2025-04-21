#include "filetools.h"
#include "filelock.h"
#include "usermanager.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

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
    std::cout << "InitChatMsgFiles ----------------------"<<std::endl;
    std::filesystem::path root_path = std::filesystem::current_path() / "chatmsgs";
    std::string dir_name;
    if (uid1 < uid2) {
        dir_name = std::to_string(uid1) + "_" + std::to_string(uid2);
    } else {
        dir_name = std::to_string(uid2) + "_" + std::to_string(uid1);
    }
    std::filesystem::path text_path = root_path / dir_name / "textmsg";
    std::filesystem::path picture_path = root_path / dir_name / "picture";
    std::filesystem::path video_path = root_path / dir_name / "video";
    std::filesystem::path audio_path = root_path / dir_name / "audio";
    std::filesystem::path file_path = root_path / dir_name / "file";

    std::cout << "\ntext_path: " << text_path << std::endl;
    std::cout << "picture_path" << picture_path << std::endl;
    std::cout << "video_path" << video_path << std::endl;
    std::cout << "audio_path" << audio_path << std::endl;
    std::cout << "file_path" << file_path << std::endl;

    CreateDir(text_path);
    CreateDir(picture_path);
    CreateDir(video_path);
    CreateDir(audio_path);
    CreateDir(file_path);
}

bool FileTools::SaveTextMsg(unsigned int uid1,
                            unsigned int uid2,
                            const std::string msg_data,
                            bool is_forwarded)
{
    // 判断是否两位用户是否有聊天记录
    std::string msg_dir;
    if (uid1 < uid2) {
        msg_dir = std::to_string(uid1) + "_" + std::to_string(uid2);
    } else {
        msg_dir = std::to_string(uid2) + "_" + std::to_string(uid1);
    }
    std::filesystem::path dir_path = std::filesystem::current_path() / "chatmsgs" / msg_dir;

    if (!std::filesystem::exists(dir_path)) {
        InitChatMsgFiles(uid1, uid2);
    }
    // 根据日期创建存储当天聊天消息的文件
    std::time_t now = std::time(nullptr);
    auto currentDate = std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(currentDate, "%Y-%m-%d");
    std::string curdate_str = ss.str();

    std::filesystem::path file_path = dir_path / "textmsg" / (curdate_str + ".txt");
    // CreateFile(file_path);

    FileLock file_lock(file_path);
    file_lock.lock();
    // 打开文件
    std::ofstream file(file_path, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    // 获取当前时间
    auto nowtime = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(nowtime);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = oss.str();

    std::string forward_state = is_forwarded == true ? "**Forwarded**" : "**Unforward**";

    json json_msg = json::parse(msg_data);
    json_msg["datetime"] = timestamp;
    // 写入消息
    file << forward_state << json_msg.dump() << std::endl;
    file.close();

    file_lock.unlock();
    return true;
}


// Base64编码工具函数
std::string Base64Encode(const unsigned char* input, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // 不换行
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

std::vector<unsigned char> Base64Decode(const std::string& input) {
    // 创建Base64解码的BIO链
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // 不处理换行符

    // 将输入字符串放入内存BIO
    BIO* mem = BIO_new_mem_buf(input.data(), static_cast<int>(input.length()));
    mem = BIO_push(b64, mem);

    // 准备输出缓冲区
    std::vector<unsigned char> output(input.length()); // 解码后数据不会比输入长
    int decoded_length = BIO_read(mem, output.data(), static_cast<int>(input.length()));

    // 清理资源
    BIO_free_all(mem);

    if(decoded_length < 0) {
        throw std::runtime_error("Base64解码失败");
    }

    output.resize(decoded_length);
    return output;
}

bool FileTools::SaveFileMsg(unsigned int uid1,
                            unsigned int uid2,
                            std::filesystem::path file_name,
                            const std::string file_content,
                            size_t length)
{
    // 判断是否两位用户是否有聊天记录
    std::string msg_dir;
    if (uid1 < uid2) {
        msg_dir = std::to_string(uid1) + "_" + std::to_string(uid2);
    } else {
        msg_dir = std::to_string(uid2) + "_" + std::to_string(uid1);
    }
    std::filesystem::path dir_path = std::filesystem::current_path() / "chatmsgs" / msg_dir;
    file_name = dir_path / "file" /file_name;

    if (!std::filesystem::exists(dir_path)) {
        InitChatMsgFiles(uid1, uid2);
    }

    std::ofstream file(file_name,std::ios::app | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << file_name << std::endl;
        return false;
    }

    auto decode = Base64Decode(file_content);
    file.write(std::string(decode.begin(),decode.end()).c_str(), length);

    return true;
}

std::time_t FileTools::GetFileLatestModifyTime(std::filesystem::path file_path)
{
    FileLock file_lock(file_path);
    file_lock.lock();

    auto writeTime = std::filesystem::last_write_time(file_path);
    auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        writeTime - std::filesystem::file_time_type::clock::now()
        + std::chrono::system_clock::now());
    auto writeTimeT = std::chrono::system_clock::to_time_t(systemTime);

    file_lock.unlock();
    return writeTimeT;
}

// 获取目录中最后更改的文件路径
std::filesystem::path FileTools::GetLatestModifiedFile(const std::string& directory)
{
    std::filesystem::path latest_file;
    std::time_t latestTime = 0;

    // 遍历目录中的所有文件
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (std::filesystem::is_regular_file(entry)) {
            // 获取文件的最后修改时间
            auto writeTimeT = GetFileLatestModifyTime(entry);
            // 比较并更新最新文件
            if (writeTimeT > latestTime) {
                latestTime = writeTimeT;
                latest_file = entry.path();
            }
        }
    }

    return latest_file;
}

// 查找目录中所有更改时间在datetime之后的文件
void FileTools::GetFiles(const std::string& directory,
                         std::vector<std::string>& files,
                         const std::string& datetime)
{
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        // std::cout << entry.path() << std::endl;
        auto last_modify = std::format("{:%Y-%m-%d %H:%M:%S}",
                                       std::chrono::system_clock::from_time_t(
                                           GetFileLatestModifyTime(entry.path())));
        if (last_modify.compare(datetime) > 0) { // 检查是否为子目录
            files.push_back(entry.path().c_str());
        }
    }
}

void FileTools::ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // 避免无限循环
    }
}

bool compare(std::string datetime1, std::string datetime2)
{
    return datetime1.compare(datetime2) < 0;
}

// conn_uid:正在尝试连接服务端的用户id,该函数返回自用户上次下线直到现在收到的所有离线消息
std::vector<std::string> FileTools::GetOfflineTextMsg(unsigned int conn_uid, unsigned int uid2)
{
    std::string msg_dir;
    if (conn_uid < uid2) {
        msg_dir = std::to_string(conn_uid) + "_" + std::to_string(uid2);
    } else {
        msg_dir = std::to_string(uid2) + "_" + std::to_string(conn_uid);
    }
    std::filesystem::path dir_path = std::filesystem::current_path() / "chatmsgs" / msg_dir;

    if (!std::filesystem::exists(dir_path)) {
        std::cout << "uid " << conn_uid << uid2 << " have not communicated\n";
        return std::vector<std::string>();
    }

    std::filesystem::path file_dir = dir_path / "textmsg";

    // 1.从数据库读取conn_uid的上次下线时间并存入临时变量
    // 2.提取出日期-找到最早的离线消息所在的文件
    // 3.记录该文件的最后修改时间last_change_time
    // 4.读取文件内标记为**Unforward**的消息，存入vector容器，同时修改标记为**Forwarded**
    // 5.遍历所有last_change_time后修改的文件并读出其内容

    std::string lastoffline = UserManager::GetInstance()->LastOfflineTime(conn_uid);
    // std::string filename = lastoffline.substr(0, 10) + ".txt";
    if (strcmp(lastoffline.substr(0, 10).c_str(), "1900-01-01") == 0)
        return std::vector<std::string>();

    std::string prefix_forwarded = "**Forwarded**";
    std::string prefix_unforward = "**Unforward**";
    std::vector<std::string> offline_msgs;

    std::vector<std::string> files;
    // files.push_back(latest_file);
    GetFiles(file_dir, files, lastoffline.substr(0, 10));
    std::sort(files.begin(), files.end(), compare);

    for (auto readfile : files) {
        std::cout << "最后更改的文件: " << readfile << std::endl;

        FileLock file_lock(readfile);
        file_lock.lock();

        std::ifstream file(readfile);
        std::string filedata;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (filedata == "")
                    filedata = line;
                filedata = filedata + "\n" + line;
                if (line.rfind(prefix_unforward) == 0) {
                    std::cout << line << std::endl;
                    std::string msg_data = line.substr(13, line.length() - 13);
                    if (json::parse(msg_data)["uid"]
                        == conn_uid) // 离线消息的发送者是conn_uid,直接结束查找
                        return std::vector<std::string>();
                    offline_msgs.push_back(msg_data);
                }
            }
            filedata = filedata + "\n";
            // 修改标记为**Forwarded**
            ReplaceAll(filedata, prefix_unforward, prefix_forwarded);
            std::ofstream outFile(readfile, std::ios::out | std::ios::trunc);
            if (!outFile.is_open()) {
                std::cerr << "无法打开文件！" << std::endl;
            }
            // 写入修改后的内容
            outFile << filedata;
            outFile.close();
            file.close();
        } else {
            std::cerr << "open file failed: " << readfile << std::endl;
        }

        file_lock.unlock();
    }

    std::cout << "offline_msgs.data():" << offline_msgs.data() << std::endl;
    return offline_msgs;
}
