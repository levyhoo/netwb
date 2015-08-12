#ifndef __NET_SERVER_H__
#define __NET_SERVER_H__


#include <net/NetCommon.h>
#include "net/NetManager.h"

namespace net
{
    /**
     * 服务器
     */
    class DLL_EXPORT_NET NetServer : public NetManager, public boost::enable_shared_from_this<NetServer>
    {
    public:
        /**
        * 构造服务器
        * @param ioservice 异步网络服务组件
        * @param ip 监听地址
        * @param port 监听端口
        */
        NetServer(io_service& ioservice, string listenIp, unsigned short listenPort, bool bReuseAddr =  true, bool bAutoBind = false);
        virtual ~NetServer();

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
        * 创建Connection
        */
        virtual boost::shared_ptr<NetConnection> createConnection();

        virtual void onAccept(boost::shared_ptr<NetConnection> connection);

    public:
        /**
        * 运行服务器 
        */
        virtual r_int32 start(bool async = true);
        /** 
        * 停止服务器
        */
        virtual r_int32 stop();
        /**
        * 向所有client广播数据
        */
        r_int32 sendDataToClients(ByteArray& data, r_int32 len, bool async = true);

    protected:
        /** 
        * 异步接收连接 
        */
        void startAccept();
        /**
        * 新连接建立 
        */
        void handleAccept(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);
        /**
        * 向所有client广播数据
        */
        r_int32 doSendDataToClients(ByteArray& data, r_int32 len, bool async = true);

        void onAcceptTimer(const boost::system::error_code& error);
        void onListenTimer(const boost::system::error_code& error);

    public:
        io_service& m_ioservice;
        string m_listenIp;
        unsigned short m_listenPort;

    private:
        StrandPtr       m_acceptorStrand;
        tcp::acceptor   m_acceptor;
        deadline_timer  m_acceptTimer; //接受连接的定时器
        deadline_timer  m_linstenTimer; //接受连接的定时器
        bool            m_bStopped;
        bool            m_bAutoBind; //监听正常
        bool            m_bReuseAddress;
    };
}

#endif