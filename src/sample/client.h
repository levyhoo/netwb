#ifndef __CLINET_20150807__H_
#define __CLINET_20150807__H_
#include "BaseClient.h"

class Client;
typedef boost::shared_ptr<Client> ClientPtr;

class Client : public BaseClient
{
public:
    Client(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort);
    virtual ~Client(){};

    ClientPtr getPtr();
    virtual void stat(vector<DataEventRaw>& UpdateEvents, vector<DataEventRaw>& CreditEvents);
    virtual void onStat(string& resp, string error);
};
#endif