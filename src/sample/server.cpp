#include "server.h"
#include "jsonHelper.h"
#include <common/Constants.h>
#include <net/stream.h>
#include <common/NetCommand.h>

Server::Server(io_service& ioservice, io_service& io, string listenIp, unsigned short listenPort)
:BaseServer(ioservice, io, listenIp, listenPort)
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

    m_ioservice.post(boost::bind(&Server::_send_stat, getPtr(), ret, status, seq, connection));
    return false;
}


ServerPtr Server::getPtr()
{
    return boost::shared_dynamic_cast<Server>(shared_from_this());
}

bool Server::test(int id, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    STDLOG(LLV_INFO, "func : %s", __FUNCTION__);
    string status("ok");
    string ret("this is the response of test");

    //m_ioservice.post(boost::bind(&Server::_send_test, getPtr(), ret, status, seq, connection));
    return true;
}


