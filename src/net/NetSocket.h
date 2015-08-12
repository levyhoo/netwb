#ifndef _ISocket_2013_5_28_h__
#define _ISocket_2013_5_28_h__

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "net/ProxyInfo.h"
#include <net/NetCommon.h>

namespace net
{
    class DLL_EXPORT_NET NetSocket : public boost::enable_shared_from_this<NetSocket>
    {
    public:
        static SocketPtr makeComonSocket(net::StrandPtr strand);
        static SocketPtr makeSslSocket(net::StrandPtr strand, boost::asio::ssl::context& context);

        virtual void cancel(boost::system::error_code& error) = 0;
        virtual void close(boost::system::error_code& error) = 0;
        virtual boost::system::error_code shutdown(const boost::asio::socket_base::shutdown_type what, boost::system::error_code& ec) = 0;
        virtual void async_read_some(void* p, size_t len, const boost::function<void (const boost::system::error_code& error, size_t)>& callback) = 0;
        virtual void async_write(void* p, size_t len, const boost::function<void (const boost::system::error_code& error, size_t)>& callback) = 0;
        virtual void async_connect(const ProxyInfo& proxy, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) = 0;
        virtual void connect(const ProxyInfo& proxy, string serverIp, int port, boost::system::error_code& error) = 0;
        virtual void write(void* p, size_t len) = 0;
        virtual boost::asio::ip::tcp::endpoint local_endpoint() = 0;
        virtual boost::asio::ip::tcp::endpoint remote_endpoint() = 0;
        virtual boost::asio::ip::tcp::socket& getSocket() = 0;

        virtual size_t read_some(void* p, size_t len, boost::system::error_code& error) = 0;

    protected:

    private:

    };

    template <typename SocketType>
    class TSocket : public NetSocket
    {
        typedef boost::shared_ptr<SocketType> SocketTypePtr;
    public:
        TSocket(){};
        virtual ~TSocket(){};

        virtual void cancel(boost::system::error_code& error) 
        {
            getSocket().cancel(error);
        };

        virtual void close(boost::system::error_code& error) 
        {
            getSocket().close(error);
        };

        virtual boost::system::error_code shutdown(const boost::asio::socket_base::shutdown_type what, boost::system::error_code& ec) 
        {
            return getSocket().shutdown(what, ec);
        };

        virtual void async_read_some(void* p, size_t len, const boost::function<void (const boost::system::error_code& error, size_t)>& callback) 
        {
            m_socket->async_read_some(boost::asio::buffer(p, len), callback);
        };

        virtual void async_write(void* p, size_t len, const boost::function<void (const boost::system::error_code& error, size_t)>& callback) 
        {
            boost::asio::async_write(*m_socket, boost::asio::buffer(p, len), callback);
        };

        virtual void write(void* p, size_t len) 
        {
            boost::asio::write(*m_socket, boost::asio::buffer(p, len));
        };

        virtual boost::asio::ip::tcp::endpoint local_endpoint() 
        {
            return getSocket().local_endpoint();
        };

        virtual boost::asio::ip::tcp::endpoint remote_endpoint() 
        {
            return getSocket().remote_endpoint();
        };

        virtual size_t read_some(void* p, size_t len, boost::system::error_code& error)
        {
            return m_socket->read_some(boost::asio::buffer(p, len), error);
        };

    protected:
        SocketTypePtr m_socket;
    };

    class DLL_EXPORT_NET CommonSocket :
        public TSocket<boost::asio::ip::tcp::socket>
    {
    public:
        CommonSocket(net::StrandPtr strand)
        {
            m_socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(strand->get_io_service()));
            m_resolver = boost::shared_ptr<boost::asio::ip::tcp::resolver>(new boost::asio::ip::tcp::resolver(strand->get_io_service()));
        };

        ~CommonSocket(){
            if (NULL != m_socket)
            {
                boost::system::error_code error;
                m_socket->close(error);
            }
        };

        virtual boost::asio::ip::tcp::socket& getSocket() 
        {
            return *m_socket;
        };

        virtual void async_connect(const ProxyInfo& proxy, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) ;
        virtual void connect(const ProxyInfo& proxy, string serverIp, int port, boost::system::error_code& error) ;

    protected:
        void on_resolve_handler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iter, const boost::function<void (const boost::system::error_code& error)>& callback);

    private:
        boost::shared_ptr<boost::asio::ip::tcp::resolver> m_resolver;
    };

    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
    class DLL_EXPORT_NET SSLSocket:
        public TSocket<ssl_socket>
    {
    public:
        SSLSocket(net::StrandPtr strand, boost::asio::ssl::context& context)
        {
            m_socket = boost::shared_ptr<ssl_socket>(new ssl_socket(strand->get_io_service(), context));
        };

        ~SSLSocket(){
            if (NULL != m_socket)
            {
                boost::system::error_code error;
                close(error);
            }
        };

        virtual boost::asio::ip::tcp::socket& getSocket() 
        {
            return m_socket->next_layer();
        };

        virtual void async_connect(const ProxyInfo& proxy, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) ;
        virtual void connect(const ProxyInfo& proxy, string serverIp, int port, boost::system::error_code& error) ;

        boost::shared_ptr<ssl_socket> getSSL() {
            return m_socket;
        };

    protected:
        void doAfterConnect(const boost::function<void (const boost::system::error_code& error)>& callback);
        void handleConnect(const boost::system::error_code& error, const boost::function<void (const boost::system::error_code& error)>& callback);
        void handleShake(const boost::system::error_code& error, const boost::function<void (const boost::system::error_code& error)>& callback);

    private:
    };
    void onResolveConnect(boost::asio::ip::tcp::socket& socket, boost::asio::ip::tcp::resolver::iterator& endpoint_iterator, const boost::system::error_code& error, const boost::function<void (const boost::system::error_code& error)>& callback);
}

#endif // ISocket_h__