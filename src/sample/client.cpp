#include "client.h"
#include "jsonHelper.h"
#include "Clog.h"

Client::Client(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort)
:BaseClient(ioservice, serverIp, serverPort)
{

}

void Client::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{

}

void Client::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{

}

ClientPtr Client::getPtr()
{
    return boost::shared_dynamic_cast<Client>(shared_from_this());
}

void Client::doSomething()
{
    STDLOG(LLV_INFO, "req func : %s, ", __FUNCTION__);
    std::string strFuncName("doSomething");
    ByteArray req;
    int id = 3;

    MAKEREQBEGIN("doSomething");
    ADDPARAM("id", id);
    MAKEREQEND(req);

    request(req, boost::bind(&Client::onDoSomething, getPtr(), _1, _2), NET_CMD_RPC, COMPRESS_DOUBLE_ZLIB);
}

void Client::onDoSomething(ByteArray& param, string error)
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



