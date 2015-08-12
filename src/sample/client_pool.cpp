#include "client_pool.h"
#include <iostream>
using namespace std;

client_pool::client_pool():next_(0)
{

}

client_pool::~client_pool()
{
    destroy();
}

void client_pool::init(int pool_size, boost::function<NetClientPtr (boost::asio::io_service& )> create_func)
{
    for (int i = 0; i < pool_size; ++i)
    {
        try 
        {
            add(create_func(io));
        }
        catch (const std::pair<int, std::string>& err)
        {
            std::cout << (boost::format("failed to create client ,%s") %err.second).str() << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << (boost::format("failed to create client  %s") %e.what()).str() << std::endl;
        }
    }
}

void client_pool::destroy()
{
    stop();
    clients_.clear();
}

void client_pool::start()
{
    const size_t cnt = clients_.size();
    boost::thread_group ths;
    for (size_t i = 0; i < cnt; ++i)
    {
        NetClientPtr& client = clients_.at(i);
        client->start();
        ths.create_thread(boost::bind(&boost::asio::io_service::run, &io));
    }
    ths.join_all();
}

void client_pool::stop()
{
    const size_t cnt = clients_.size();
    for (size_t i = 0; i < cnt; ++i)
    {
        NetClientPtr& client = clients_.at(i);
        client->stop();
    }
}

void client_pool::add(const NetClientPtr& client)
{
    if (NULL != client)
    { 
        clients_.push_back(client);
    }
   
}

NetClientPtr& client_pool::get()
{
    const size_t cnt = clients_.size();
    if (cnt > 0)
    {
        return clients_.at(next() % cnt);
    }
    throw std::logic_error("empty client pool");
}

long long client_pool::next()
{
    if (next_ >= (std::numeric_limits<long long>::max)())
    {
        next_ = 0;
    }
    return next_++;
}

NetClientPtr creat_test_client(asio::io_service& io, string ip, unsigned short port)
{
    NetClientPtr ret = NetClientPtr(new NetClient(io, ip, port));
    return ret;
}
