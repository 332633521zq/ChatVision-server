#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/file.h> // 用于 flock
#include <unistd.h>   // 用于 flock

class FileLock
{
public:
    FileLock(const std::string& filePath);
    bool lock();
    bool unlock();
    ~FileLock();

private:
    std::string filePath;
    int fd; // 文件描述符
};
