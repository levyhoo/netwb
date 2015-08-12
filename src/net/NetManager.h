#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include "net/NetConnection.h"
#include "net/NetPackage.h"
#include <net/NetCommon.h>
#include <set>

using namespace std;

namespace net
{
    /* 前向声明*/
    class NetClient;
    class NetServer;

    /**
     * 网络连接管理器
     */
    class DLL_EXPORT_NET NetManager
    {
        friend class NetClient;
        friend class NetServer;
        friend class NetConnection;

    public:
        NetManager();
        virtual ~NetManager();

    public:
        //////////////////////////////////////////////////////////////////////////
        // 继承类只需重写这些函数
        //////////////////////////////////////////////////////////////////////////
        /**
        * 连接建立
        */
        virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection) = 0;
        /**
        * 连接中断
        */
        virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection) = 0;
        /**
        * 数据到达（原始数据）
        * @remark 如果该函数被重写，则函数onPackageReceived()不会被自动调用。
        */
        virtual void onDataReceived(const ByteArray& data, r_int32 len, boost::shared_ptr<NetConnection> connection);
        /**
        * 数据包到达
        * @remark 如果函数onDataReceived()被重写，则该函数不会被自动调用。
        */
        virtual void onPackageReceived(const NetPackageHeader& header, const unsigned char* content, const size_t& contentLen, boost::shared_ptr<NetConnection> connection);
        /**
        * 数据发送完成
        */
        virtual void onDataSend(r_int32 len, boost::shared_ptr<NetConnection> connection) = 0;
        /**
        * 创建连接
        */
        virtual NetConnectionPtr createConnection() = 0;

    public:
        /**
        * 得到所有连接
        */
        set<boost::shared_ptr<NetConnection> > getAllConnections();
        /**
        * 删除所有连接
        */
        void removeAllConnections();

        /**
        * 删除一个连接
        */
        void removeConnection(boost::shared_ptr<NetConnection> connection);

    protected:
        /**
        * 开始运行.
        * @remark 子类必须重写该函数
        */
        virtual r_int32 start(bool async = true) = 0;
        /**
        * 停止运行.
        * @remark 子类必须重写该函数
        */
        virtual r_int32 stop() = 0;
        /**
        * 添加一个连接
        */
        void addConnection(boost::shared_ptr<NetConnection> connection);
    
    private:
        boost::recursive_mutex m_mutex;                             /* 全局锁 */
        set<boost::shared_ptr<NetConnection> > m_connectionPool;    /* 连接池 */
    };
}

#endif
