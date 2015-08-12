#ifndef __NET_CLIENT_H__
#define __NET_CLIENT_H__


#include "net/NetManager.h"
#include "net/ProxyInfo.h"
#include <net/NetCommon.h>


namespace net
{
    /**
     * 客户端
     */
    class DLL_EXPORT_NET NetClient : public NetManager, public boost::enable_shared_from_this<NetClient>
    {
    public:
        /**
        * 构造客户端
        * @param ioservice 异步网络服务组件
        * @param ip 服务器地址
        * @param port 服务器端口
        */
        NetClient(io_service& ioservice, string serverIp, unsigned short serverPort);
        virtual ~NetClient();

    public:
        //////////////////////////////////////////////////////////////////////////
        // 继承类只需重写这些函数
        //////////////////////////////////////////////////////////////////////////
        /** 
        * 连接建立 
        */
        virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);
        /** 
        * 连接中断
        */
        virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);
        /** 
        * 数据包到达
        * @remark 如果要获取原始数据，则重写函数onDataReceived()，本函数不会被调用。
        */
        virtual void onPackageReceived(const NetPackageHeader& header, const unsigned char* content, const size_t& contentLen, boost::shared_ptr<NetConnection> connection);
        /** 
        * 数据发送完成 
        */
        virtual void onDataSend(r_int32 len, boost::shared_ptr<NetConnection> connection);
        /**
        * 创建连接
        */
        virtual NetConnectionPtr createConnection() ;

    public:        
        /**
        * 设置代理
        */
        void setProxy(const ProxyInfo proxyInfo);

        /**
        * 运行客户端 
        */
        r_int32 start(bool async = true);
        /** 
        * 停止客户端 
        */
        r_int32 stop();
        /**
        * 发送数据
        * @param async 是否异步发送数据
        * @return 0-正在发送;1-网络连接无效
        */
        r_int32 sendData(ByteArray& data, r_int32 len, bool async = true);
        r_int32 sendData(const char* data, r_int32 len, bool async = true);
        size_t recvDataSync(char* buf, size_t len);

        /**
        * 连接是否有效
        */
        bool isConnected();

    protected:
        void reconnect(int seconds);

    private:
        /**
        * 同步建立连接
        */
        void startConnectSync();
        /** 
        * 异步建立连接
        */
        void startConnectAsync();

        /** 
        * 连接建立 
        */
        void handleConnect(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);

        /** 
        * 代理服务器的处理
        */
        void handleHttpProxy(boost::shared_ptr<NetConnection> connection);
        void handleSock4Proxy(boost::shared_ptr<NetConnection> connection);
        void handleSock5Proxy(boost::shared_ptr<NetConnection> connection);
        void onReconnectTimer(const boost::system::error_code& error);


    public:
        io_service&                     m_ioservice;
        boost::shared_ptr<NetConnection>       m_connection;
        string                          m_serverIp;
        unsigned short                  m_serverPort;

    protected:
        deadline_timer m_reconnectTimer; //重新连接的定时器
        ProxyInfo      m_proxyInfo;      //代理信息
    };
}

#endif