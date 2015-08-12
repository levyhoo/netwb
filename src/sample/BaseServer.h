#ifndef __BASESERVER__20150807__H
#define __BASESERVER__20150807__H
#include <net/NetServer.h>
using namespace net;

typedef boost::function< void (const ByteArray &value, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection) > ProcessFunc;
typedef map<string, ProcessFunc> FuncTable;

class BaseServer;
typedef boost::shared_ptr<BaseServer> BaseServerPtr;

class BaseServer : public NetServer
{
public:
    BaseServer(io_service& ioservice, string listenIp, unsigned short listenPort);
    virtual ~BaseServer(){};

    virtual void regist() = 0;
    void registerFunc(const string& method, const ProcessFunc& func);
    virtual void onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);
    virtual void onRequest(const string &func, const ByteArray &value, const r_int64 seq, NetConnectionPtr connection);
    void sendResp(const ByteArray& resp, r_int16& cmd, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection, r_int8& compress);
    BaseServerPtr getPtr();

protected:
    boost::shared_mutex         m_funcTableMutex;
    FuncTable                   m_funcTable;
};

#endif