#ifndef IO_SERVICE_POOL_HPP
#define IO_SERVICE_POOL_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <deque>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
typedef unsigned char byte;

struct ext_io_service
{
    io_service_ptr ios;
    work_ptr work;
    boost::shared_ptr<boost::thread> trd;
    int use_count;
    int idle_begin_time; 

    ext_io_service() : use_count(0), idle_begin_time((int)time(NULL)) {};
};

typedef boost::shared_ptr<ext_io_service> ext_io_service_ptr;

class io_service_pool
    : private boost::noncopyable
{
public:
    static io_service_pool& instance()
    {
        static io_service_pool s_ios_pool;
        return s_ios_pool;
    }

    explicit io_service_pool(std::size_t pool_size = 0, std::size_t max_pool_size = 600, int timeout_interval = 0);
    void init(std::size_t pool_size = 0, std::size_t max_pool_size = 600, int timeout_interval = 0);

    void run();

    void stop();

    io_service_ptr get_io_service();
    io_service_ptr get_io_service_exclusive(); 

    void complete(long tag);

private:
    ext_io_service_ptr create();

    const ext_io_service_ptr& get_work_ios();
    void recycle(const ext_io_service_ptr& io_service);
    void adjust();

private:

    std::size_t pool_size_;
    std::size_t max_pool_size_;
    std::map<long, ext_io_service_ptr> work_io_services_;
    std::map<long, ext_io_service_ptr> exclusive_io_services_;
    std::deque<ext_io_service_ptr> idle_io_services_;
    int timeout_interval_;
    boost::mutex mutex_;
};

#endif 
