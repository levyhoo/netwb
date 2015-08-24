#include "server.h"
#include "jsonHelper.h"
#include <common/Constants.h>
#include <net/stream.h>
#include <common/NetCommand.h>

Server::Server(io_service& ioservice, string listenIp, unsigned short listenPort)
:BaseServer(ioservice, listenIp, listenPort)
{
    
}

void Server::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    STDLOG(LLV_INFO, "func : %s, peer : %s", __FUNCTION__, connection->peerAddress().c_str());
}

void Server::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{
    STDLOG(LLV_INFO, "func : %s, peer : %s", __FUNCTION__, connection->peerAddress().c_str());
}

bool Server::stat(vector<DataEventRaw>& UploadEvents, vector<DataEventRaw>& CreditEvents, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    STDLOG(LLV_INFO, "func : %s", __FUNCTION__);
    string status("ok");
    string ret("this is the response of doSomething");

    m_ioservice.post(boost::bind(&Server::send_stat, getPtr(), ret, status, seq, connection));
    return false;
}

void Server::send_stat(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    STDLOG(LLV_INFO, "func : %s", __FUNCTION__);
    _send_stat(resp, status, seq, connection);
}


ServerPtr Server::getPtr()
{
    return boost::shared_dynamic_cast<Server>(shared_from_this());
}

