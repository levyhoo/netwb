#include "common/Stdafx.h"
#include "net/NetSocket.h"
#include <boost/asio/ssl.hpp>
#include "net/ProxyHelper.h"
#include <boost/lexical_cast.hpp>
#include <boost/asio/connect.hpp>

namespace net
{
    SocketPtr NetSocket::makeComonSocket(net::StrandPtr strand)
    {
        return SocketPtr(new CommonSocket(strand));
    }

    SocketPtr NetSocket::makeSslSocket(net::StrandPtr strand, boost::asio::ssl::context& context)
    {
        return SocketPtr(new SSLSocket(strand, context));        
    }

    void CommonSocket::async_connect(const ProxyInfo& proxyInfo, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) 
    {
        if( proxyInfo.isEnabled() )
        {
            boost::system::error_code error;
            stringstream os;
            os << serverIp << ":" << port;
            string address = os.str();
            PROXY_CONNECT_STATUS pstat = net::ProxyHelper::passProxy(proxyInfo, address, getSocket(), error);
            if (pstat == PROXY_CONNECT_STATUS_OK)
            {
                if (NULL != callback)
                {
                    callback(error);
                }
            } else if (pstat == PROXY_CONNECT_STATUS_TIMTOUT_ERROR){
                boost::system::error_code err(boost::asio::error::timed_out);
                if (NULL != callback)
                {
                    callback(err);
                }
            }else {
                if (NULL != callback)
                {
                    callback(error);
                }
            }
        }
        else
        {
            try
            {
                using namespace boost::asio::ip;
                tcp::resolver::query query(tcp::v4(), serverIp, boost::lexical_cast<string>(port));
                m_resolver->async_resolve(query, boost::bind(&CommonSocket::on_resolve_handler, boost::shared_dynamic_cast<CommonSocket>(shared_from_this()), _1, _2, callback));
            }
            catch(const boost::system::error_code& error)
            {
                if (NULL != callback)
                {
                    callback(error);
                }
            } 
            catch (...)
            {
                boost::system::error_code err(boost::asio::error::operation_aborted);
                if (NULL != callback)
                {
                    callback(err);
                }
            }
        }
    }

    void CommonSocket::connect(const ProxyInfo& proxyInfo, string serverIp, int port, boost::system::error_code& error) 
    {
        boost::asio::ip::tcp::endpoint serverEndpoint(boost::asio::ip::address::from_string(serverIp.c_str()), port);
        stringstream os;
        os << serverIp << ":" << port;
        string address = os.str();
        PROXY_CONNECT_STATUS pstat = net::ProxyHelper::passProxy(proxyInfo, address, getSocket(), error);
        if (pstat == PROXY_CONNECT_STATUS_TIMTOUT_ERROR)
        {
            error = boost::asio::error::timed_out;
        }
    }

    void CommonSocket::on_resolve_handler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iter, const boost::function<void (const boost::system::error_code& error)>& callback)
    {
        if (!error)
        {
            boost::asio::ip::tcp::resolver::iterator end;
            boost::asio::async_connect(getSocket(), iter, end, boost::bind(callback, _1));
        }
        else
        {
            if (NULL != callback)
            {
                callback(error);
            }
        }
    }

    void SSLSocket::async_connect(const ProxyInfo& proxyInfo, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) 
    {
        if( proxyInfo.isEnabled() )
        {
            boost::system::error_code error;
            stringstream os;
            os << serverIp << ":" << port;
            string address = os.str();
            PROXY_CONNECT_STATUS pstat = net::ProxyHelper::passProxy(proxyInfo, address, getSocket(), error);
            if (pstat == PROXY_CONNECT_STATUS_OK)
            {
                doAfterConnect(callback);
            } else if (pstat == PROXY_CONNECT_STATUS_TIMTOUT_ERROR){
                boost::system::error_code err(boost::asio::error::timed_out);
                if (NULL != callback)
                {
                    callback(err);
                }
            }else {
                if (NULL != callback)
                {
                    callback(error);
                }
            }
        }
        else
        {
            try
            {
                boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(serverIp.c_str()), port);
                getSocket().async_connect(ep, boost::BOOST_BIND(&SSLSocket::handleConnect, boost::shared_dynamic_cast<SSLSocket>(shared_from_this()), _1, callback));
            }
            catch(const boost::system::error_code& error)
            {
                if (NULL != callback)
                {
                    callback(error);
                }
            } 
            catch (...)
            {
                boost::system::error_code err(boost::asio::error::operation_aborted);
                if (NULL != callback)
                {
                    callback(err);
                }
            }
        }
    }

    void SSLSocket::connect(const ProxyInfo& proxyInfo, string serverIp, int port, boost::system::error_code& error) 
    {
        boost::asio::ip::tcp::endpoint serverEndpoint(boost::asio::ip::address::from_string(serverIp.c_str()), port);
        stringstream os;
        os << serverIp << ":" << port;
        string address = os.str();
        PROXY_CONNECT_STATUS pstat = net::ProxyHelper::passProxy(proxyInfo, address, getSocket(), error);
        if (pstat == PROXY_CONNECT_STATUS_OK)
        {
            // 连接成功， 握手
            m_socket->handshake(boost::asio::ssl::stream_base::client, error);
        } else {
            // 连接失败
            if (pstat == PROXY_CONNECT_STATUS_TIMTOUT_ERROR)
            {
                error = boost::asio::error::timed_out;
            }
        }
    }

    void SSLSocket::doAfterConnect(const boost::function<void (const boost::system::error_code& error)>& callback)
    {
        m_socket->async_handshake(boost::asio::ssl::stream_base::client,
            boost::bind(&SSLSocket::handleShake, boost::shared_dynamic_cast<SSLSocket>(shared_from_this()),
            boost::asio::placeholders::error, callback));
    }

    void SSLSocket::handleConnect(const boost::system::error_code& error, const boost::function<void (const boost::system::error_code& error)>& callback)
    {
        if (!error)
        {
            doAfterConnect(callback);
        } else {
            if (NULL != callback)
            {
                callback(error);
            }
        }
    }

    void SSLSocket::handleShake(const boost::system::error_code& error, const boost::function<void (const boost::system::error_code& error)>& callback)
    {
        string strError = error.message();
        if (error)
        {
            boost::system::error_code err;
            getSocket().close(err);
        }

        if (NULL != callback)
        {
            callback(error);
        }
    }
}