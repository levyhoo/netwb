#include "common/Stdafx.h"
#include "net/NetConnection.h"
#include "net/NetManager.h"
#include "net/BaseCipher.h"
#include "common/NetCommand.h"
#include <boost/lexical_cast.hpp>
//#include <utils/Threading.h>
#include "net/NetSocket.h"

namespace net
{
        //定义发送缓冲区最大值, 超过该值则会断掉连接
    #ifndef MAX_SEND_BUF_SIZE
    #define MAX_SEND_BUF_SIZE (1024 * 1024 * 64)
    #endif

    //定义发送缓冲区最小值, 超过该值则会断掉连接
    #ifndef MIN_SEND_BUF_SIZE
    #define MIN_SEND_BUF_SIZE (1024 * 64)
    #endif

    NetConnection::NetConnection(StrandPtr strand, SocketPtr socket)
        :m_strand(strand), 
        m_socket(socket),
        m_sendKATimer(strand->get_io_service()), 
        m_timeoutTimer(strand->get_io_service())
    {
        m_compressType = 0;//COMPRESS_NONE
        init();
    }

    NetConnection::~NetConnection()
    {
        system::error_code error;
        m_socket->cancel(error);
        m_socket->close(error);

        if (m_waitingBufLen > 0)
        {
            free(m_waitingBuf);
        }

        if (m_sendBufLen > 0)
        {
            free(m_sendBuf);
        }
    }

    void NetConnection::init()
    {
        m_bConnected = false;
        m_recvBuf.resize(MIN_SEND_BUF_SIZE);
        m_recvBufLen = 0;

        m_sendBufLen = 0;
        m_sendBuf = NULL;
        m_waitingBufLen = 0;
        m_waitingBuf = NULL;
        m_waitingPos = 0;
        m_sendAveSize = 0;

        m_bSending = false;

        m_timeoutInterval = TIME_OUT_INTERVAL;
        m_keepAliveInterval = KEEP_ALIVE_INTERVAL;

        m_lastSendTime= INT32_MAX;
        m_lastRecvTime = INT32_MAX;
    }


    //开始连接的会话
    r_int32 NetConnection::start()
    {
        m_bConnected = true;
        m_lastSendTime = (int)time(NULL);
        m_lastRecvTime = (int)time(NULL);

        onConnectionMade();

        /* 读取包 */
        recvData();

        return 0;
    }

    r_int32 NetConnection::stop()
    {

        return 0;
    }

    void NetConnection::close()
    {
        m_strand->post(boost::bind(&NetConnection::doClose, shared_from_this(), true));
    }

    void NetConnection::doClose(bool active)
    {
        m_bConnected = false;
        try
        {
            boost::system::error_code ec;
            m_socket->cancel(ec);
        }
        catch(...)
        {
        }

        try
        {
            boost::system::error_code ec;
            m_socket->close(ec);

        }
        catch(...)
        {
        }

        try
        {

            boost::system::error_code ec;
            m_socket->shutdown(boost::asio::socket_base::shutdown_both, ec);
        }
        catch (...)
        {
        }

        system::error_code error;
        m_sendKATimer.cancel(error);
        m_timeoutTimer.cancel(error);

        //清缓冲区
        boost::recursive_mutex::scoped_lock rlock(m_mutex);
        m_waitingPos = 0;
        m_bSending = false;
    }

    void NetConnection::setNetManager(boost::shared_ptr<NetManager> pNetManager)
    {
        m_pNetManager = pNetManager;
    }

    //异步接收数据
    void NetConnection::recvData()
    {
        try
        {
            m_socket->async_read_some(&m_recvBuf[0], m_recvBuf.size(),  m_strand->wrap(boost::bind(&NetConnection::handleRecvData, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred )));
        }
        catch(...)
        {
            boost::system::error_code err(boost::asio::error::network_down);
            m_strand->post(boost::bind(&NetConnection::handleConnectionError, shared_from_this(), err));
            //handleConnectionError(err);
        }
    }

    size_t NetConnection::recvDataSync(char* buf, size_t len)
    {
        size_t ret_len(0);
        boost::system::error_code error;
        ret_len = m_socket->read_some(buf, len, error);
        if(error)
        {
            // std::cerr << "error in read, " << ec2.message() << "ret_len: " << ret_len << endl;
            onConnectionError(error);
            // ret_len = 0;
        }
        return ret_len;
    }

    bool NetConnection::sendData(ByteArray& data, r_int32 len, bool async)
    {
        if (async)
            return sendDataAsync(data, len);
        else
            return sendDataSync(data, len);
    }

    bool NetConnection::sendData(NetPackage& package, bool async)
    {
        ByteArray data;
        package.encode(data);
        return sendData(data, data.size(), async);
    }

    bool NetConnection::sendDataSync(ByteArray& data, r_int32 len)
    {
        try
        {
            if (m_bConnected)
            {
                boost::recursive_mutex::scoped_lock rlock(m_mutex);
                void *pSendBuf = &data[0];
                m_socket->write(pSendBuf, len);
                onDataSend(len);
            } else {
                return false;
            }
        }
        catch(...)
        {
            boost::system::error_code err(boost::asio::error::network_down);
            m_strand->post(boost::bind(&NetConnection::handleConnectionError, shared_from_this(), err));
            //handleConnectionError(err);
        }
        return true;
    }

    bool NetConnection::sendDataAsync(ByteArray& data, r_int32 len)
    {
        try
        {
            if (m_bConnected)
            {
                if (data.size() > 0 && len > 0)
                {
                    boost::recursive_mutex::scoped_lock rlock(m_mutex);

                    // 发送缓冲过大， 则断掉连接
                    r_int32 needSize = len + m_waitingPos;
                    if (needSize > MAX_SEND_BUF_SIZE)
                    {
                        m_waitingPos = 0;
                        boost::system::error_code err(boost::asio::error::operation_aborted);
                        m_strand->post(boost::bind(&NetConnection::handleConnectionError, shared_from_this(), err));
                        return false;
                    }

                    // 如果缓冲内存不够， 则增加
                    if (needSize > (r_int32)m_waitingBufLen)
                    {
                        r_int32 resizeLen = needSize <= MIN_SEND_BUF_SIZE ? MIN_SEND_BUF_SIZE : (r_int32)pow( 2, ceil(log(needSize * 1.0 / MIN_SEND_BUF_SIZE) / log(2.0)) ) * MIN_SEND_BUF_SIZE ;
                        m_waitingBuf = (m_waitingBufLen == 0) ? (unsigned char*)malloc(resizeLen) : (unsigned char*)realloc(m_waitingBuf, resizeLen);
                        if (NULL == m_waitingBuf)
                        {
                            assert(false);
                        }
                        m_waitingBufLen = resizeLen;
                    }

                    // 拷贝数据到缓冲区
                    memcpy(&m_waitingBuf[m_waitingPos], &data[0], len);
                    m_waitingPos += len;

                    if (!m_bSending)
                    {
                        doSendData();
                    }
                }
            } else {
                return false;
            }
        }
        catch(...)
        {
            boost::system::error_code err(boost::asio::error::network_down);
            m_strand->post(boost::bind(&NetConnection::handleConnectionError, shared_from_this(), err));
            //handleConnectionError(err);
            return false;
        }
        return true;
    }

    bool NetConnection::isConnected()
    {
        return m_bConnected;
    }


    void NetConnection::handleRecvData(const boost::system::error_code& err, r_int32 bytesReceived)
    {
        if (!err)
        {
            m_lastRecvTime = (r_int32)time(NULL);

            m_recvBufLen = bytesReceived;
            if (m_recvBufLen > 0)
            {
                try{
                    onDataReceived(m_recvBuf, (r_int32)m_recvBufLen);
                }
                catch (...)
                {

                }
            }
            m_recvBufLen = 0;
            recvData();
        }
        else
        {
            handleConnectionError(err);
        }
    }

    void NetConnection::handleSendData(const boost::system::error_code& err, r_int32 bytesSend)
    {
        try{
            if (err)
            {
                handleConnectionError(err);
            } else {         
                m_lastSendTime = (r_int32)time(NULL);   
                onDataSend(bytesSend);
                doSendData();
            }
        }
        catch (...)
        {

        }
    }

    void NetConnection::doSendData()
    {
        boost::recursive_mutex::scoped_lock rlock(m_mutex);
        if (m_waitingPos == 0 )
        {
            m_bSending = false;
            return ;
        } 

        size_t bytesSend = 0;
        if (!m_cipher)
        {
            swap(m_sendBufLen, m_waitingBufLen);
            swap(m_sendBuf, m_waitingBuf);
            bytesSend = m_waitingPos;
            m_waitingPos = 0;
            m_bSending = true;
            m_socket->async_write(m_sendBuf , bytesSend, \
                m_strand->wrap(boost::bind(&NetConnection::handleSendData, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred)));
        } else { 
            while (true)
            {
                if (m_sendBufLen == 0)
                {
                    m_sendBufLen = MIN_SEND_BUF_SIZE;
                    m_sendBuf = (unsigned char *)malloc(m_sendBufLen);
                }

                bytesSend = m_sendBufLen - 2 * sizeof(r_uint32);
                bool nbreak = true;
                r_int32 ret = m_cipher->encrypt(m_waitingBuf, m_waitingPos, m_sendBuf + 2 * sizeof(r_uint32), &bytesSend);
                switch (ret)
                {
                case CIPHER_STATUS_OK:
                    {
                        bytesSend += 2 * sizeof(r_uint32);
                        r_uint32 netLen = htonl(bytesSend);
                        memcpy(m_sendBuf, &netLen, sizeof(r_uint32));
                        r_uint32 realLen = htonl(m_waitingPos);
                        memcpy(m_sendBuf + sizeof(r_uint32), &realLen, sizeof(r_uint32));
                        m_socket->async_write(m_sendBuf , bytesSend, \
                             m_strand->wrap(boost::bind(&NetConnection::handleSendData, shared_from_this(), \
                            asio::placeholders::error, asio::placeholders::bytes_transferred)));
                        m_waitingPos = 0;
                        m_bSending = true;
                    }
                    break;
                case CIPHER_STATUS_BUFFER_ERROR:
                    m_sendBufLen = size_t(m_waitingBufLen * 1.5);
                    m_sendBuf = (unsigned char*)realloc(m_sendBuf, m_waitingBufLen);
                    nbreak = false;
                    break;
                default:
                    break;
                }

                if (nbreak)
                {
                    break;
                }
            }
        }        

        // 内存控制, 采用加权平均计算平均发送长度, 若发送长度是waitingBufLen的1/4则内存减半
        m_sendAveSize =  (m_sendAveSize > 0) ? (r_int32)(m_sendAveSize * 0.98 + bytesSend * 0.02) : bytesSend;
        if ( m_sendAveSize > (MIN_SEND_BUF_SIZE) && m_waitingBufLen > (2 * MIN_SEND_BUF_SIZE) && m_sendAveSize < (r_int32)(m_waitingBufLen / 4) )
        {
            m_waitingBufLen = r_int32(m_waitingBufLen / 2);
            m_waitingBuf = (unsigned char*)realloc(m_waitingBuf, m_waitingBufLen);
            if (NULL == m_waitingBuf)
            {
                assert(false);
            }
        }
    }

    void NetConnection::handleConnectionError(const boost::system::error_code& err)
    {
        if (m_bConnected)
        {
            doClose(false);
            onConnectionError(err);

            boost::shared_ptr<NetManager> manager = m_pNetManager.lock();
            if (NULL != manager)
            {
                manager->removeConnection(shared_from_this());
            }
        }
    }

    void NetConnection::onConnectionMade()
    {
        boost::system::error_code error;
        getSocket().set_option(boost::asio::ip::tcp::no_delay(true), error);
        boost::shared_ptr<NetManager> manager = m_pNetManager.lock();
        if (NULL != manager)
        {
            manager->onConnectionMade(shared_from_this());
        }
    }

    void NetConnection::onConnectionError(const boost::system::error_code& err)
    {
        boost::shared_ptr<NetManager> manager = m_pNetManager.lock();
        if (NULL != manager)
        {
            manager->onConnectionError(err, shared_from_this());
        }
    }

    //数据到达，解析出完整的网络包
    void NetConnection::onDataReceived(const ByteArray& data, r_int32 len)
    {
        boost::shared_ptr<NetManager> manager = m_pNetManager.lock();
        if (NULL != manager)
        {
            manager->onDataReceived(data, len, shared_from_this());
        }
    }

    void NetConnection::onDataSend(r_int32 len)
    {
        boost::shared_ptr<NetManager> manager = m_pNetManager.lock();
        if (NULL != manager)
        {
            manager->onDataSend(len, shared_from_this());
        }
    }

    string NetConnection::localAddress() const
    {
        tcp::endpoint endPoint = m_socket->local_endpoint();
        return endPoint.address().to_string() + ":" + boost::lexical_cast<string>(endPoint.port());
    }

    string NetConnection::peerAddress() const
    {
        return m_peerIp + ":" + boost::lexical_cast<string>(m_peerPort);
    }


    boost::asio::ip::tcp::socket& NetConnection::getSocket() 
    {
        return m_socket->getSocket();
    }

    SocketPtr NetConnection::getSocketPtr()
    {
        return m_socket;
    }

    void NetConnection::setCipher(const boost::shared_ptr<BaseCipher>& cipher)
    {
        m_cipher = cipher;
        m_pkgParser.setCipher(cipher);
    }

    void NetConnection::sendKeepAlive(const system::error_code& error)
    {
        m_strand->post(boost::bind(&NetConnection::doSendKeepAlive, shared_from_this(), error));
    }

    void NetConnection::doSendKeepAlive(const system::error_code& error)
    {
        if (!error)
        {
            r_int32 now = (r_int32)time(NULL);
            if ( (now - m_lastSendTime) > m_keepAliveInterval)
            {
                //发送保活信息
                NetPackageHeader header(NET_CMD_KEEPALIVE, 0, 0, 0);
                header.m_length = sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH;
                
                ByteArray bytes;
                bytes.resize(sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH);
                r_int32 pos = header.encode(bytes);
                sendData(bytes, bytes.size());
            } 

            boost::system::error_code error;
            m_sendKATimer.cancel(error);
            m_sendKATimer.expires_from_now(boost::posix_time::seconds(m_keepAliveInterval));
            m_sendKATimer.async_wait( m_strand->wrap(boost::bind(&NetConnection::doSendKeepAlive, shared_from_this(), boost::asio::placeholders::error)));
        }   
    }

    void NetConnection::timeoutCheck(const system::error_code& error)
    {
        m_strand->post(boost::bind(&NetConnection::doTimeoutCheck, shared_from_this(), error));
    }

    void NetConnection::doTimeoutCheck(const system::error_code& error)
    {
        if (!error)
        {
            r_int32 now = (r_int32)time(NULL);
            if ((now - m_lastRecvTime) >= m_timeoutInterval)
            {
                boost::system::error_code timeoutError(boost::asio::error::timed_out, boost::system::get_system_category() );
                handleConnectionError(timeoutError);
            } else {
                boost::system::error_code error;
                m_timeoutTimer.cancel(error);
                m_timeoutTimer.expires_from_now(boost::posix_time::seconds(m_timeoutInterval));
                m_timeoutTimer.async_wait( m_strand->wrap(boost::bind(&NetConnection::doTimeoutCheck, shared_from_this(), boost::asio::placeholders::error)));
            }
        }
    }

    void NetConnection::async_connect(const ProxyInfo& proxy, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) 
    {
        if (NULL != m_socket)
        {
            m_socket->async_connect(proxy, serverIp, port, callback);
        } else {
            boost::system::error_code error = boost::asio::error::operation_aborted;
            onConnectionError(error);
        }
    }

    void NetConnection::connect(const ProxyInfo& proxy, string serverIp, int port, boost::system::error_code& error) 
    {
        if (NULL != m_socket)
        {
            m_socket->connect(proxy, serverIp, port, error);
        } else {
            error = boost::asio::error::operation_aborted;
        }
    }

}
