#include "client.h"
#include "Clog.h"

Client::Client(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort)
:BaseClient(ioservice, serverIp, serverPort)
{

}

ClientPtr Client::getPtr()
{
    return boost::shared_dynamic_cast<Client>(shared_from_this());
}

void Client::stat(vector<DataEventRaw>& UpdateEvents, vector<DataEventRaw>& CreditEvents)
{
    _stat(UpdateEvents, CreditEvents);
}

void Client::onStat(string& resp, string error)
{
    STDLOG(LLV_INFO, "resp func : %s", __FUNCTION__);
    if (error.compare("error") == 0)
    {
        return;
    }
    else
    {

    }
}



