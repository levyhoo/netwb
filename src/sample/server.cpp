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

void Server::doSomething(const ByteArray &param, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    STDLOG(LLV_INFO, "func : %s", __FUNCTION__);
    string status("ok");
    string ret("this is the response of doSomething");
    
    m_ioservice.post(boost::bind(&Server::send_doSomething, getPtr(), ret, status, seq, connection));
}

void Server::send_doSomething(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    STDLOG(LLV_INFO, "func : %s", __FUNCTION__);
    ByteArray response;
    MAKERESPBEGIN(status);
    ADDPARAM("resp", resp);
    MAKERESPEND(response);
    r_int16 cmd = (r_int16)NET_CMD_RPC;
    r_int8 compress = (r_int8)COMPRESS_DOUBLE_ZLIB;
    sendResp(response, cmd, seq, connection, compress);
}

void Server::regist()
{
    boost::shared_lock<shared_mutex> lock(m_funcTableMutex);
    m_funcTable.insert(make_pair<string, ProcessFunc>("doSomething", boost::bind(&Server::doSomething, getPtr(), _1, _2, _3)));
}


ServerPtr Server::getPtr()
{
    return boost::shared_dynamic_cast<Server>(shared_from_this());
}

