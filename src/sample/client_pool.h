#ifndef __ROUND_CLIENT_POOL_H__
#define __ROUND_CLIENT_POOL_H__
#include <net/NetClient.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
using namespace net;

class client_pool
        : public boost::noncopyable, public boost::enable_shared_from_this<client_pool>
    {
    public:

        client_pool();
        virtual ~client_pool();

        void init(int pool_size, boost::function<NetClientPtr (boost::asio::io_service&)> create_func);
        void destroy();

        void start();
        void stop();

        void add(const NetClientPtr& client);
        NetClientPtr& get();

    protected:
        std::vector<NetClientPtr> clients_;

    private:
        long long next();

    private:
        long long next_;
        boost::asio::io_service io;

    };
    typedef boost::shared_ptr<client_pool> client_pool_ptr;
    NetClientPtr creat_test_client(asio::io_service& io, string ip, unsigned short port);
#endif