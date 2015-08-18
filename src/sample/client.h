#ifndef __CLINET_20150807__H_
#define __CLINET_20150807__H_
#include "BaseClient.h"

class Client;
typedef boost::shared_ptr<Client> ClientPtr;
typedef map<r_int64, CallBack> CBTable;

class Client : public BaseClient
{
public:
    Client(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort);
    virtual ~Client(){};

    void setAutoConnect(bool bAuto){m_autoReConnect = bAuto;};

    virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);
    virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);

    ClientPtr getPtr();
    void doSomething();
    void onDoSomething(ByteArray& param, string error);

private:
    CBTable m_CBs;
    r_int64 m_requestSeq;
    boost::mutex m_mutex;
    bool m_autoReConnect;
};
#endif