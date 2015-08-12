#include "common/Stdafx.h"
#include "net/NetClient.h"
#include "stream.h"
#include "net/ProxyHelper.h"
#include "net/NetSocket.h"

using namespace net;


NetClient::NetClient(io_service& ioservice, string serverIp, unsigned short serverPort) : m_ioservice(ioservice), m_reconnectTimer(ioservice)
{
    m_serverIp = serverIp;
    m_serverPort = serverPort;
}

NetClient::~NetClient()
{

}

r_int32 NetClient::start(bool async)
{
    if (async)
    {
        m_ioservice.post(boost::bind(&NetClient::startConnectAsync, shared_from_this()));
    }
    else
    {
        startConnectSync();
    }

    return 1;
}

r_int32 NetClient::stop()
{
    boost::system::error_code error;
    m_reconnectTimer.cancel(error);

    m_ioservice.post(boost::bind(&NetClient::removeAllConnections, shared_from_this()));
    return 1;
}

r_int32 NetClient::sendData(ByteArray& data, r_int32 len, bool async)
{
    r_int32 ret = 1;

    if (m_connection.get() != NULL)
    {
        ret = m_connection->sendData(data, len, async) ? 1 : 0;
    }

    return ret;
}

r_int32 NetClient::sendData(const char* data, r_int32 len, bool async)
{
    ByteArray vdata(len);
    memcpy(&vdata[0], data, len);
    return sendData(vdata, len, async);
}

size_t NetClient::recvDataSync(char* buf, size_t len)
{
    return m_connection->recvDataSync(buf, len);
}

bool NetClient::isConnected()
{
    bool ret = false;

    if (m_connection.get() != NULL)
        ret = m_connection->isConnected();

    return ret;
}

void NetClient::startConnectSync()
{
    if (isConnected())
    {
        return;
    }

    try
    {
        m_connection = createConnection();
        if (NULL == m_connection)
        {
            boost::system::error_code error = boost::asio::error::service_not_found;
            m_ioservice.post(boost::bind(&NetClient::onConnectionError, shared_from_this(), error, m_connection));
        } else {
            m_connection->setNetManager(shared_from_this());
            boost::system::error_code error;
            m_connection->connect(m_proxyInfo, m_serverIp, m_serverPort, error);
            if (!error)
            {
                addConnection(m_connection);
                m_connection->start();
            } else {
                onConnectionError(error, m_connection);
            }
        }
    }
    catch (const boost::system::error_code& error)
    {
        onConnectionError(error, m_connection);
    }
    catch (...)
    {
        boost::system::error_code error = boost::asio::error::ssl_errors();
        onConnectionError(error, m_connection);
    }
}

void NetClient::startConnectAsync()
{
    if (isConnected())
    {
        return;
    }
    
    //判断是否启用了代理
    try
    {
        m_connection = createConnection();
        if (NULL == m_connection)
        {
            boost::system::error_code error = boost::asio::error::service_not_found;
            m_ioservice.post(boost::bind(&NetClient::onConnectionError, shared_from_this(), error, m_connection));
        } else {
            m_connection->setNetManager(shared_from_this());
            m_connection->async_connect(m_proxyInfo, m_serverIp, m_serverPort, boost::bind(&NetClient::handleConnect, shared_from_this(), boost::asio::placeholders::error, m_connection));    
        }
    }
    catch (const boost::system::error_code& error)
    {
        m_ioservice.post(boost::bind(&NetClient::onConnectionError, shared_from_this(), error, m_connection));
    }
    catch (...)
    {
        boost::system::error_code error = boost::asio::error::ssl_errors();
        m_ioservice.post(boost::bind(&NetClient::onConnectionError, shared_from_this(), error, m_connection));
    }
}

void NetClient::handleConnect(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    if (!err)
    {
        try
        {
            tcp::endpoint endpoint = connection->getSocket().remote_endpoint();
            connection->m_peerIp = endpoint.address().to_string();
            connection->m_peerPort = endpoint.port();
            string str = endpoint.address().to_string();

            // 检测自成交, 如果自成交, 网络断开            
            tcp::endpoint localEndpoint = connection->getSocket().local_endpoint();
            string localstr = endpoint.address().to_string();
            if (endpoint == localEndpoint)
            {
                connection->close();
                onConnectionError(err, connection);
            }
        }
        catch (...)
        {
        }
        addConnection(connection);
        connection->start();
    } else {
        onConnectionError(err, connection);
    }
}

void NetClient::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{

}

void NetClient::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    reconnect(NET_RECONNECT_INTERVAL);
}

void NetClient::onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection)
{

}

void NetClient::onDataSend(r_int32 len, boost::shared_ptr<NetConnection> connection)
{

}

NetConnectionPtr NetClient::createConnection() 
{
    StrandPtr strand(new boost::asio::strand(m_ioservice));
    SocketPtr socket = NetSocket::makeComonSocket(strand);
    NetConnectionPtr connection = boost::shared_ptr<NetConnection>(new NetConnection(strand, socket));
    return connection;
}


void NetClient::onReconnectTimer(const boost::system::error_code& error)
{
    if (!error)
    {
        startConnectAsync();
    }
}

void NetClient::setProxy( const ProxyInfo proxyInfo )
{
    m_proxyInfo = proxyInfo;
}

void NetClient::reconnect(int seconds)
{
    m_reconnectTimer.expires_from_now(boost::posix_time::seconds(seconds));
    m_reconnectTimer.async_wait(boost::bind(&NetClient::onReconnectTimer, shared_from_this(), _1));
}