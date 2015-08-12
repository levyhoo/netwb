#include "common/Stdafx.h"
#include "net/NetServer.h"
#include "net/NetSocket.h"

using namespace net;


NetServer::NetServer(io_service& ioservice, string listenIp, unsigned short listenPort, bool bReuseAddr, bool autoBind)
:m_ioservice(ioservice),
m_listenIp(listenIp),
m_listenPort(listenPort),
m_acceptorStrand(new boost::asio::strand(ioservice)),
m_acceptor(ioservice), 
m_acceptTimer(ioservice),
m_linstenTimer(ioservice),
m_bStopped(false),
m_bAutoBind(autoBind),
m_bReuseAddress(bReuseAddr)
{
    if (m_bAutoBind)
    {
        bool bNeedBind = true;
        try
        {
            do 
            {
                /* 初始化连接接收器 */
                asio::socket_base::reuse_address reuseAddrOption(bReuseAddr);
                boost::asio::socket_base::keep_alive keepaliveOption(true);
                tcp::endpoint listenEndpoint(ip::address::from_string(m_listenIp.c_str()), m_listenPort);
                m_acceptor.open(listenEndpoint.protocol());
                boost::system::error_code error;
                if (bReuseAddr)
                {
                    m_acceptor.set_option(reuseAddrOption, error);
                }
                m_acceptor.set_option(keepaliveOption, error);
                m_acceptor.bind(listenEndpoint, error);
                if (!error)
                {
                    bNeedBind = false;
                    m_acceptor.listen();                
                }
                else
                {
                    m_acceptor.close();
                    if(m_listenPort < 10000)
                    {
                        m_listenPort = 10000;
                    }
                    if (m_listenPort < 65534)
                    {
                        ++m_listenPort;
                    }
                    else
                    {
                        m_listenPort = 10000;
                    }

                }
            } while (bNeedBind);
        }
        catch (const boost::system::error_code& error)
        {
            boost::system::error_code _error;
            m_acceptor.cancel(_error);
        }
        catch(...)
        {
            boost::system::error_code error;
            m_acceptor.cancel(error);
        }
    }
}

NetServer::~NetServer()
{

}

r_int32 NetServer::start(bool async)
{
    if (!m_bAutoBind)
    {
        /* 初始化连接接收器 */
        asio::socket_base::reuse_address reuseAddrOption(m_bReuseAddress);
        boost::asio::socket_base::keep_alive keepaliveOption(true);
        tcp::endpoint listenEndpoint(ip::address::from_string(m_listenIp.c_str()), m_listenPort);
        m_acceptor.open(listenEndpoint.protocol());
        boost::system::error_code error;
        if (m_bReuseAddress)
        {
            m_acceptor.set_option(reuseAddrOption, error);
        }
        m_acceptor.set_option(keepaliveOption, error);
        m_acceptor.bind(listenEndpoint, error);
        if (error != boost::system::error_code())
        {
            m_acceptor.close();
        }
        else
        {
            m_acceptor.listen();
        }
    }    

    if (m_acceptor.is_open())
    {
        m_acceptorStrand->post(boost::bind(&NetServer::startAccept, shared_from_this()));
    } else {

        //接口单同步启动一个服务，如果监听失败，需要抛出一个异常
        if(!async)
        {
            throw "端口监听失败";
        }

        m_linstenTimer.expires_from_now(boost::posix_time::seconds(NET_REACCEPT_INTERVAL));
        m_linstenTimer.async_wait(m_acceptorStrand->wrap(boost::bind(&NetServer::onListenTimer, shared_from_this(), _1)));
    }
    return 0;
}

r_int32 NetServer::stop()
{
    boost::system::error_code error;
    m_acceptTimer.cancel(error);

    try
    {
        m_acceptor.cancel(error);
        m_acceptor.close(error);
    }
    catch (...)
    {
    }

    try
    {
        m_linstenTimer.cancel(error);
    }
    catch (...)
    {
    }

    m_bStopped = true;
    removeAllConnections();
    return 0;
}

r_int32 NetServer::sendDataToClients(ByteArray& data, r_int32 len, bool async)
{
    m_acceptorStrand->post(boost::bind(&NetServer::doSendDataToClients, shared_from_this(), data, len, async));

    return 0;
}

r_int32 NetServer::doSendDataToClients(ByteArray& data, r_int32 len, bool async)
{
    boost::recursive_mutex::scoped_lock rlock(m_mutex);
    try
    {
        set<boost::shared_ptr<NetConnection> >::iterator iter = m_connectionPool.begin();
        for ( ; iter != m_connectionPool.end(); iter++)
        {
            if ((*iter).get() != NULL)
                (*iter)->sendData(data, len, async);
        }
    }
    catch(...) 
    {

    }

    return 0;
}


/*
 * 监听新连接
 */
void NetServer::startAccept()
{
    boost::shared_ptr<NetConnection> connection = createConnection();
    connection->setNetManager(shared_from_this());

    m_acceptor.async_accept(connection->getSocket(), m_acceptorStrand->wrap(boost::bind(&NetServer::handleAccept, shared_from_this(), boost::asio::placeholders::error, connection)));
}

/*
 * 处理新连接
 */
void NetServer::handleAccept(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    if (!err)
    {
        try
        {
            tcp::endpoint endpoint = connection->getSocket().remote_endpoint();
            connection->m_peerIp = endpoint.address().to_string();
            connection->m_peerPort = endpoint.port();
        }
        catch (...)
        {
        }

        onAccept(connection);


        /* 监听新连接 */
        startAccept();
    }
    else
    {
        if (!m_bStopped && m_acceptor.is_open())
        {
            m_acceptTimer.expires_from_now(boost::posix_time::seconds(NET_REACCEPT_INTERVAL));
            m_acceptTimer.async_wait(m_acceptorStrand->wrap(boost::bind(&NetServer::onAcceptTimer, shared_from_this(), _1)));
        }
    }
}

void NetServer::onAccept(boost::shared_ptr<NetConnection> connection)
{
    addConnection(connection);
    connection->start();
}

void NetServer::onAcceptTimer(const boost::system::error_code& error)
{
    if (!error)
    {
        startAccept();
    }
}

void NetServer::onListenTimer(const boost::system::error_code& error)
{
    if (!error && !m_bStopped)
    {
        if (!m_acceptor.is_open())
        {
            tcp::endpoint listenEndpoint(ip::address::from_string(m_listenIp.c_str()), m_listenPort);
            m_acceptor.open(listenEndpoint.protocol());   
            boost::system::error_code error;         
            m_acceptor.bind(listenEndpoint, error);
            if (!error)
            {
                m_acceptor.listen(boost::asio::socket_base::max_connections, error);
            }
            if (!error)
            {
                startAccept();
            } else {
                m_acceptor.close(error);
                m_linstenTimer.expires_from_now(boost::posix_time::seconds(NET_REACCEPT_INTERVAL));
                m_linstenTimer.async_wait(m_acceptorStrand->wrap(boost::bind(&NetServer::onListenTimer, shared_from_this(), _1)));
            }
        }
    }
}


void NetServer::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{

}


void NetServer::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    
}

void NetServer::onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection)
{

}

void NetServer::onDataSend(r_int32 len, boost::shared_ptr<NetConnection> connection)
{

}

boost::shared_ptr<NetConnection> NetServer::createConnection()
{
    net::StrandPtr strand(new boost::asio::strand(m_ioservice));
    SocketPtr socket = NetSocket::makeComonSocket(strand);
    return boost::shared_ptr<NetConnection>(new NetConnection(strand, socket));
}

