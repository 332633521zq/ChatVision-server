#include "filelock.h"

FileLock::FileLock(const std::string& filePath)
    : filePath(filePath)
    , fd(-1)
{}

bool FileLock::lock()
{
    fd = open(filePath.c_str(), O_RDWR); // 打开文件
    if (fd == -1) {
        std::cerr << "无法打开文件：" << filePath << std::endl;
        return false;
    }

    // 加独占锁（写锁）
    if (flock(fd, LOCK_EX) == -1) {
        std::cerr << "无法加锁文件：" << filePath << std::endl;
        close(fd);
        return false;
    }

    return true;
}

bool FileLock::unlock()
{
    if (fd != -1) {
        // 解锁
        if (flock(fd, LOCK_UN) == -1) {
            std::cerr << "无法解锁文件：" << filePath << std::endl;
            close(fd);
            return false;
        }
        close(fd);
        fd = -1;
    }
    return true;
}

FileLock::~FileLock()
{
    unlock(); // 确保析构时解锁
}
