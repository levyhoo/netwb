#include "common/Stdafx.h"
#include "net/ProxyHelper.h"
#include "stream.h"
#include "strings.h"
#include <boost/lexical_cast.hpp>

namespace net
{
    using boost::asio::ip::tcp;

    static PROXY_CONNECT_STATUS httpProxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error);
    static PROXY_CONNECT_STATUS sock4Proxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error);
    static PROXY_CONNECT_STATUS sock5Proxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error);
    static bool connectSocket(const string& ip, unsigned short port, tcp::socket& socket, boost::system::error_code& error);

    PROXY_CONNECT_STATUS ProxyHelper::passProxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error)
    {
        switch (proxyInfo.type)
        {
        case PROXY_TYPE_NON :
            {
                string ip;
                unsigned short port;
                getIpPort(address, ip, port);
                if (connectSocket(ip, port, socket, error))
                {
                    return PROXY_CONNECT_STATUS_OK;
                } else {
                    return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
                }
            }
            break;
        case PROXY_TYPE_HTTP:
            return httpProxy(proxyInfo, address, socket, error);
            break;
        case PROXY_TYPE_SOCKS4:
            return sock4Proxy(proxyInfo, address, socket, error);
            break;
        case PROXY_TYPE_SOCKS5:
            return sock5Proxy(proxyInfo, address, socket, error);
            break;
        }
        return PROXY_CONNECT_STATUS_UNKNOWN_ERROR;
    }

    bool isTimeout( boost::asio::ip::tcp::socket& socket )
    {
        //判断是否超时
        fd_set fdSet;
        FD_ZERO(&fdSet);

        struct timeval timeStruct;
        timeStruct.tv_sec = 10;
        timeStruct.tv_usec = 0;

        int nativeSocket = socket.native();
        FD_SET(nativeSocket, &fdSet);

        select(nativeSocket+1, &fdSet, NULL, NULL, &timeStruct);
        if(!FD_ISSET(nativeSocket, &fdSet))
        { 
            // timeout
            return true;
        }

        return false;
    }

    bool connectSocket(const string& ip, unsigned short port, tcp::socket& socket, boost::system::error_code& error)
    {
        // 连接代理
        tcp::resolver resolver(socket.get_io_service());
        tcp::resolver::query query(tcp::v4(), ip, boost::lexical_cast<string>(port));

        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
        tcp::resolver::iterator end_iterator;
        while (endpoint_iterator != end_iterator)
        {
            boost::asio::connect(socket, endpoint_iterator, error);
            if (error)
            {
                ++endpoint_iterator;
            }
            else
            {
                break;
            }
        }

        if (!error)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    PROXY_CONNECT_STATUS httpProxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error)
    {
        if (!connectSocket(proxyInfo.ip, proxyInfo.port, socket, error))
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }        

        //构造HTTP请求头
        string auth_raw, auth;
        auth_raw = proxyInfo.username + ":" + proxyInfo.password;
        encodeBase64((BYTE *)&auth_raw[0], auth_raw.size(), auth);
        ostringstream command;
        command<<"CONNECT " << address <<" HTTP/1.1\r\n"
            <<"Proxy-Authorization: Basic "<<auth<<"\r\n"
            <<"Authorization: Basic "<<auth<<"\r\n\r\n";
        string writebuffer = command.str();
        socket.write_some(boost::asio::buffer(writebuffer.data(), writebuffer.size()), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }
        
        if(isTimeout(socket))
        { 
            return PROXY_CONNECT_STATUS_TIMTOUT_ERROR;
        }

        //读取响应，并分析
        boost::asio::streambuf response;
        size_t n = boost::asio::read_until(socket, response, "\r\n\r\n", error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        boost::asio::streambuf::const_buffers_type bufs = response.data();
        string resbuf(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + n);

        vector<string> items;
        boost::algorithm::split(items, resbuf, boost::is_any_of("\n"));
        if( items.size() < 0 || items[0].find("HTTP/1.0 200") == -1 )
        {
            error = boost::system::error_code(11130, boost::asio::error::get_system_category());
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        return PROXY_CONNECT_STATUS_OK;
    }

    PROXY_CONNECT_STATUS sock4Proxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error)
    {
        if (!connectSocket(proxyInfo.ip, proxyInfo.port, socket, error))
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        string ip;
        unsigned short port;
        getIpPort(address, ip, port);

        //构造socks4请求头
        SOCKS4_REQ req = {0};
        req.vn      = 4;
        req.cd      = 1;
        req.port    = htons(port);
        req.address = inet_addr(ip.c_str());
        string writebuffer;
        writebuffer.resize(sizeof(req));
        memcpy(&writebuffer[0], &req, sizeof(req));
        socket.write_some(boost::asio::buffer(writebuffer.data(), writebuffer.size()), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }
        
        if(isTimeout(socket))
        { 
            return PROXY_CONNECT_STATUS_TIMTOUT_ERROR;
        }

        //分析返回值
        boost::array<char, 256> buf;
        socket.read_some(boost::asio::buffer(buf), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        SOCKS4_RES* res = (SOCKS4_RES*)&buf[0];
        if( res->vn != 0 || res->cd != 90 )
        {
            boost::system::error_code error(17026, boost::asio::error::get_system_category());
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        return PROXY_CONNECT_STATUS_OK;
    }

    PROXY_CONNECT_STATUS sock5Proxy(const ProxyInfo& proxyInfo, const string& address, tcp::socket& socket, boost::system::error_code& error)
    {
        if (!connectSocket(proxyInfo.ip, proxyInfo.port, socket, error))
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        //验证req1
        SOCKS5_REQ1 req = {0};
        req.ver         = 5;
        req.nmethods    = 1;
        req.method1     = proxyInfo.need_check ? 2:0;
        string writebuffer;
        writebuffer.resize(sizeof(req));
        memcpy(&writebuffer[0], &req, sizeof(req));
        socket.write_some(boost::asio::buffer(writebuffer.data(), writebuffer.size()), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        if(isTimeout(socket))
        { 
            return PROXY_CONNECT_STATUS_TIMTOUT_ERROR;
        }

        boost::array<char, 256> buf;
        socket.read_some(boost::asio::buffer(buf), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        SOCKS5_RES1* res = (SOCKS5_RES1*)&buf[0];
        if( res->ver != 5 || (res->method != 0 && res->method != 2) )
        {
            boost::system::error_code error(21726, boost::asio::error::get_system_category());
            return PROXY_CONNECT_STATUS_PROXY_ERROR;
        }

        //验证auth
        if(proxyInfo.need_check)
        {
            SOCKS5_AUTH_REQ req = {0};
            req.ver     =    1;
            req.ulen    = min<size_t>(255, proxyInfo.username.size());
            req.plen    = min<size_t>(255, proxyInfo.password.size());
            memcpy(&req.username, &proxyInfo.username[0], req.ulen);
            memcpy(&req.password, &proxyInfo.password[0], req.plen);

            string writebuffer;
            writebuffer.resize(3 + req.ulen + req.plen);
            memcpy(&writebuffer[0], &req.ver, 2);
            memcpy(&writebuffer[2], &req.username, req.ulen);
            memcpy(&writebuffer[2+req.ulen], &req.plen, 1);
            memcpy(&writebuffer[3+req.ulen], &req.password, req.plen);
            socket.write_some(boost::asio::buffer(writebuffer.data(), writebuffer.size()), error);
            if(error)
            {
                return PROXY_CONNECT_STATUS_PROXY_ERROR;
            }

            boost::array<char, 256> buf;
            socket.read_some(boost::asio::buffer(buf), error);
            if(error)
            {
                return PROXY_CONNECT_STATUS_PROXY_ERROR;
            }
            SOCKS5_AUTH_RES* res = (SOCKS5_AUTH_RES*)&buf[0];
            if( res->ver != 1 || res->status != 0 )
            {
                error = boost::system::error_code(27526, boost::asio::error::get_system_category());
                return PROXY_CONNECT_STATUS_PROXY_ERROR;
            }
        }

        //验证req2
        string ip;
        unsigned short port;
        getIpPort(address, ip, port);
        SOCKS5_REQ2 req2 = {0};
        req2.ver         = 5;
        req2.command     = 1;
        req2.reserved    = 0;
        req2.atype       = 1;
        req2.port        = htons(port);
        req2.address     = inet_addr(ip.c_str());

        string writebuffer2;
        writebuffer2.resize(sizeof(req2));
        memcpy(&writebuffer2[0], &req2, sizeof(req2));
        socket.write_some(boost::asio::buffer(writebuffer2.data(), writebuffer2.size()), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        boost::array<char, 256> buf2;
        socket.read_some(boost::asio::buffer(buf2), error);
        if(error)
        {
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        SOCKS5_RES2* res2 = (SOCKS5_RES2*)&buf2[0];
        if( res2->ver != 5 || res2->rep != 0 )
        {
            error = boost::system::error_code(31826, boost::asio::error::get_system_category());
            return PROXY_CONNECT_STATUS_ENDPOINT_ERROR;
        }

        return PROXY_CONNECT_STATUS_OK;
    }
}