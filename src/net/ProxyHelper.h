#ifndef PROXY_HELPER_2012_06_13_H_
#define PROXY_HELPER_2012_06_13_H_

#include <net/NetCommon.h>
#include <boost/asio.hpp>
#include "net/ProxyInfo.h"

namespace net
{
    enum PROXY_CONNECT_STATUS{
        PROXY_CONNECT_STATUS_OK,
        PROXY_CONNECT_STATUS_PROXY_ERROR,
        PROXY_CONNECT_STATUS_ENDPOINT_ERROR,
        PROXY_CONNECT_STATUS_TIMTOUT_ERROR,
        PROXY_CONNECT_STATUS_UNKNOWN_ERROR,
    };

    class DLL_EXPORT_NET ProxyHelper
    {
    public:
        static PROXY_CONNECT_STATUS passProxy(const ProxyInfo& proxyInfo, const string& address, boost::asio::ip::tcp::socket& socket, boost::system::error_code& error);
        bool isTimeout(boost::asio::ip::tcp::socket& socket);
    };
}

#endif