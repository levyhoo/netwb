#ifndef __SERVER__20150807__H
#define __SERVER__20150807__H
#include "BaseServer.h"
#include "Clog.h"

class Server;
typedef boost::shared_ptr<Server> ServerPtr;

class Server : public BaseServer
{
public:
    Server(io_service& ioservice, string listenIp, unsigned short listenPort);
    virtual ~Server(){};

    virtual void regist();
    //virtual void onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);
    //virtual void onRequest(const string &func, const ByteArray &value, const r_int64 seq, NetConnectionPtr connection);
    ServerPtr getPtr();

    void doSomething(const ByteArray &param, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection);
    void send_doSomething(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection);

};

#endif