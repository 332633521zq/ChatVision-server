#include "ioservicepool.h"

using boost::asio::make_work_guard;

IOServicePool::IOServicePool(std::size_t size)
    :_io_services(size), _next_ioservices(0),_works(size)
{
    for(std::size_t i = 0;i<size;i++){
        _works[i] = std::make_unique<WorkGuard>
                (make_work_guard(_io_services[i].get_executor()));
    }

    for(std::size_t i = 0;i<_io_services.size();i++){
        _threads.emplace_back([this,i](){
            _io_services[i].run();
        });
    }
}

IOServicePool::~IOServicePool()
{
    Stop();
    std::cout<<"IOServicePool destruct"<<std::endl;
}

boost::asio::io_context &IOServicePool::GetIOService()
{
    return _io_services[_next_ioservices++ % _io_services.size()];
}

void IOServicePool::Stop()
{
    for(auto& work: _works){
        work.reset();
    }

    for(auto& t: _threads){
        t.join();
    }
}
