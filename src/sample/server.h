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

    virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);
    ServerPtr getPtr();

    bool stat(vector<DataEventRaw>& UploadEvents, vector<DataEventRaw>& CreditEvents, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection);
    void send_stat(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection);

};

#endif