#pragma once
#include <iostream>
#include <memory>
#include <mutex>

template<typename T>
class Singleton
{
protected: // 保证子类能调用基类构造函数，且外部不能调用
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;

    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_flags; // s_flags只有在第一次调用该函数时会初始化一次，不会重复初始化
            // 且s_falgs存在于静态区，会随进程结束自动释放
        std::call_once(s_flags, [&]() { _instance = std::shared_ptr<T>(new T); });
        // call_once只会调用一次,原理：加锁->设置s_flags标记的状态->执行匿名函数初始化智能指针->解锁
        return _instance;
    }

    void PrintAddress() { std::cout << _instance.get() << std::endl; }

    ~Singleton() { std::cout << "this is Singleton destruct" << std::endl; }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr; // 模板在编译时不生效，实例化时才会生效
