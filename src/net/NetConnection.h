#ifndef __NET_CONNECTION_H__
#define __NET_CONNECTION_H__

#include "net/NetCommon.h"
#include "net/NetPackageParser.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include "net/ProxyInfo.h"
#include "BaseCipher.h"

using namespace std;

namespace net
{
    using namespace boost;
    using namespace boost::asio;
    using namespace boost::asio::ip;

    #define NET_RECONNECT_INTERVAL            3       /* 连接中断后重新连接的时间间隔 */
    #define NET_REACCEPT_INTERVAL             3       /* 重新接受连接的时间间隔 */
    #define NET_KEEP_ALIVE_INTERVAL           2       /* 心跳信息发送间隔 */
    #define NET_KEEP_ALIVE_TIMEOUT_INTERVAL   10000      /* 心跳信息超时间隔 */

    /* 前向声明 */
    class NetConnection;
    class NetManager;
    class NetServer;
    class NetClient;
    class RPCClient;

    /**
     * 该类的每个实例代表一个连接
     */
    class DLL_EXPORT_NET NetConnection : public enable_shared_from_this<NetConnection>
    {
        friend class NetManager;
        friend class NetClient;
        friend class NetServer;
        friend class RPCConnection;

    public:
        NetConnection(StrandPtr strand, SocketPtr socket);
        virtual ~NetConnection();

    public:
        /**
        * 发送数据
        * @param async 是否异步发送数据
        * @remark 发送的时候会将发送数据复制一份
        */
        virtual bool sendData(ByteArray& data, r_int32 len, bool async = true);
        bool sendData(NetPackage& package, bool async = true);
        size_t recvDataSync(char* buf, size_t len);

        void init();
        /**
        * 本连接是否有效
        */
        bool isConnected();

        /**
        * 设置加密
        */
        void setCipher(const boost::shared_ptr<BaseCipher>& cipher);

        void sendKeepAlive(const system::error_code& error);
        void timeoutCheck(const system::error_code& error = boost::system::error_code());

        void setKeepAliveInterval(r_int32 interval) { m_keepAliveInterval = interval; };
        void setTimeOutInterval(r_int32 interval) { m_timeoutInterval = interval; };

        /**
         *  连接成功后的本端地址
         */
        string localAddress() const;

        /**
         *  对端地址
         */
        string peerAddress() const;

        /** 
        * 连接关闭 
        */
        void close();
        
        /** 
        * 取socket 
        */
        boost::asio::ip::tcp::socket& getSocket() ;
        SocketPtr getSocketPtr() ;

        /** 
        * 异步链接
        */
        void async_connect(const ProxyInfo& proxy, string serverIp, int port, const boost::function<void (const boost::system::error_code& error)>& callback) ;
        void connect(const ProxyInfo& proxy, string serverIp, int port, boost::system::error_code& error) ;

        /** 
        * 会话开始,
        * 应该是先建立连接, 然后再是会话开始
        */
        r_int32 start();
        /** 
        * 会话停止 
        */
        r_int32 stop();


        /** 
        * 返回压缩类型和设置类型
        */
        r_int8 m_compressType;
        inline r_int8 getCompressType() { return m_compressType; }
        void setCompressType( r_int8 compressType) { m_compressType = compressType; }


    protected:
        /** 
        * 连接建立 
        */
        virtual void onConnectionMade();
        /** 
        * 连接中断
        */
        virtual void onConnectionError(const boost::system::error_code& err);
        /** 
        * 数据到达
        */
        virtual void onDataReceived(const ByteArray& data, r_int32 len);
        /** 
        * 数据发送完成 
        */
        virtual void onDataSend(r_int32 len);

        virtual void doClose(bool active);


    protected:
        /** 
        * 设置管理器 
        */
        void setNetManager(boost::shared_ptr<NetManager> pNetManager);
        /**
        * 接收数据
        */
        void recvData();
        /**
        * 同步发送数据
        * @remark 发送的时候会将发送数据复制一份。同一个连接最好只调用同步或者异步发送之一，不要混合调用
        */
        bool sendDataSync(ByteArray& data, r_int32 len);

        /**
        * 异步发送数据
        * @remark 发送的时候会将发送数据复制一份。同一个连接最好只调用同步或者异步发送之一，不要混合调用
        */
        bool sendDataAsync(ByteArray& data, r_int32 len);

        void doSendData();

        /* 接收数据处理 */
        void handleRecvData(const boost::system::error_code& err, r_int32 bytesReceived);
        /* 发送数据处理 */
        void handleSendData(const boost::system::error_code& err, r_int32 bytesSend);
        /* 错误处理 */
        void handleConnectionError(const boost::system::error_code& err);

        void doSendKeepAlive(const system::error_code& error);
        void doTimeoutCheck(const system::error_code& error);

    public:
        boost::weak_ptr<NetManager>     m_pNetManager;          /* 连接管理器 */
        SocketPtr               m_socket;               /* socket */
        string                  m_peerIp;               /* 对端ip */
        unsigned short          m_peerPort;             /* 对端port */
        boost::shared_ptr<BaseCipher>  m_cipher;               /* 加解密模块 */
        StrandPtr               m_strand;

    protected:
        boost::recursive_mutex  m_mutex;
        bool                    m_bConnected;       /* 连接是否有效 */
        NetPackageParser        m_pkgParser;        /* 网络数据包解析器 */
        ByteArray               m_recvBuf;          /* 接收缓冲*/
        r_int32                 m_recvBufLen;       /* 接收缓冲已使用量 */
        unsigned char*          m_waitingBuf;       /* 等待的buffer */
        r_int32                 m_waitingBufLen;    /* 等待buffer长度 */
        unsigned char*          m_sendBuf;          /* 发送缓冲区 */
        r_int32                 m_sendBufLen;       /* 发送缓冲区长度 */
        r_int32                 m_waitingPos;       /* 当前位置 */
        bool                    m_bSending;         /* 是否正在发送 */
        r_int32                 m_sendAveSize;      /* 发送队列的平均有效长度 */

        r_int32                     m_lastSendTime;
        asio::deadline_timer        m_sendKATimer;
        r_int32                     m_lastRecvTime;
        asio::deadline_timer        m_timeoutTimer;

        r_int32                     m_timeoutInterval;   //超时时间，单位秒
        r_int32                     m_keepAliveInterval; //保活间隔，单位秒
    };
}

#endif