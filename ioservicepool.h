#pragma once

#include "singleton.h"
#include <boost/asio.hpp>

class IOServicePool:public Singleton<IOServicePool>
{
    friend class Singleton<IOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IOService::executor_type>;
    using WorkPtr = std::unique_ptr<WorkGuard>;
    IOServicePool(const IOServicePool&) = delete;
    IOServicePool& operator = (const IOServicePool&) = delete;
    ~IOServicePool();

    boost::asio::io_context& GetIOService();    // 用轮询的方式返回一个io_context
    void Stop();
private:
    IOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOService> _io_services;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _next_ioservices;
};

