#include "BaseServer.h"
#include "jsonHelper.h"
#include <common/Constants.h>
#include <net/stream.h>
#include <common/NetCommand.h>
#include "Clog.h"

BaseServer::BaseServer(io_service& ioservice, io_service& io, string listenIp, unsigned short listenPort)
:m_io(io), m_wk(io), m_strand(io), NetServer(ioservice, listenIp, listenPort)
{
}

void BaseServer::registerFunc(const string& method, const ProcessFunc& func)
{
    boost::unique_lock<shared_mutex> lock(m_funcTableMutex);
    m_funcTable[method] = func;
}

void BaseServer::onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection)
{

    if (contentLen <= 0 || NULL == connection)
    {
        return ;
    }

    ByteArray tmpData;
    if (header.m_flag.compress == COMPRESS_ZLIB || header.m_flag.compress == COMPRESS_DOUBLE_ZLIB)
    {
        if (header.m_flag.compress == COMPRESS_DOUBLE_ZLIB && connection->m_compressType != COMPRESS_DOUBLE_ZLIB)
        {
            connection->m_compressType = COMPRESS_DOUBLE_ZLIB;
        }
        bool bOK = zlibDecompress(const_cast<unsigned char*>(contentP), contentLen, tmpData, 0);
        if (tmpData.empty())
        {
            return;
        }
        if (bOK)
        {
            if (tmpData.size() <= 0)
            {
                return;
            }
        }
    }
    else if (header.m_flag.compress == COMPRESS_LZMA || header.m_flag.compress == COMPRESS_DOUBLE_LZMA)
    {
        if (header.m_flag.compress == COMPRESS_DOUBLE_LZMA && connection->m_compressType != COMPRESS_DOUBLE_LZMA)
        {
            connection->m_compressType = COMPRESS_DOUBLE_LZMA;
        }
        bool bOK = LzmaDecomp(const_cast<unsigned char*>(contentP), contentLen, tmpData, 0);
        if (tmpData.empty())
        {
            return;
        }
        if (bOK)
        {
            if (tmpData.size() <= 0)
            {
                return;
            }
        }
    }
    else
    {
        tmpData.resize(contentLen+1);
        memcpy(&tmpData[0], contentP, contentLen);
    }

    switch (header.m_cmd)
    {
    case NET_CMD_RPC:
        {
            string func("");
            ByteArray param;
            if (jsonHelper::getInstance()->getField(func, "func", tmpData))
            {
                if (jsonHelper::getInstance()->getSubJson(param, "param", tmpData))
                {
                    m_strand.post(boost::bind(&BaseServer::onRequest, getPtr(), func, param, header.m_seq, connection));
                }
            }
            else
            {
                //deal else
            }
        }
        break;
    case NET_CMD_KEEPALIVE:
        {
            STDLOG(LLV_INFO, "send keepalive packet, peer : %s", connection->peerAddress().c_str());
            //���ͱ����
            NetPackageHeader header(NET_CMD_KEEPALIVE_RESPONSE, 0, 0, 0);
            header.m_length = sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH;

            ByteArray bytes;
            bytes.resize(sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH);
            r_int32 pos = header.encode(bytes);
            connection->sendData(bytes, bytes.size());
        }
        break;
    case NET_CMD_KEEPALIVE_RESPONSE:
        {
            STDLOG(LLV_INFO, "get keepalive packet, peer : %s", connection->peerAddress().c_str());
        }
        break;
    default:
        break;
    }
}

void BaseServer::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{

}

void BaseServer::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{

}

void BaseServer::onRequest(const string &func, const ByteArray &value, const r_int64 seq, NetConnectionPtr connection)
{
    boost::shared_lock<shared_mutex> lock(m_funcTableMutex);
    FuncTable::iterator iter = m_funcTable.find(func);
    if (iter != m_funcTable.end())
    {
        try
        {
            iter->second(value, seq, connection);
        }
        catch(...)
        {

        }
    }
}

BaseServerPtr BaseServer::getPtr()
{
    return boost::shared_dynamic_cast<BaseServer>(shared_from_this());
}

void BaseServer::sendResp(const ByteArray& resp, r_int16& cmd, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection, r_int8& compress)
{
    ByteArray bytes;
    NetPackageHeader header(cmd, seq, compress, 0);
    if (!zlibCompress(resp, bytes, NET_PACKAGE_HEADER_LENGTH))
    {
        header.m_flag.compress = COMPRESS_NONE;
    }
    header.m_length = bytes.size();
    header.encode(bytes);
    connection->sendData(bytes, bytes.size());
}

void BaseServer::_stat(const ByteArray &param, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    string resp = "this is response of stat";
    string status = "ok";
    vector<DataEventRaw> UploadEvents, CreditEvents;
    jsonHelper::getInstance()->getField(UploadEvents, "UploadEvents", param);
    jsonHelper::getInstance()->getField(CreditEvents, "CreditEvents", param);
    if(stat(UploadEvents, CreditEvents, seq, connection))
    {
        m_io.post(boost::bind(&BaseServer::_send_stat, getPtr(), resp, status, seq, connection));
    }
}

void BaseServer::_send_stat(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    ByteArray response;
    MAKERESPBEGIN(status);
    ADDPARAM("resp", resp);
    MAKERESPEND(response);
    r_int16 cmd = (r_int16)NET_CMD_RPC;
    r_int8 compress = (r_int8)COMPRESS_DOUBLE_ZLIB;
    sendResp(response, cmd, seq, connection, compress);
}

void BaseServer::regist()
{
    boost::shared_lock<shared_mutex> lock(m_funcTableMutex);
    m_funcTable.insert(make_pair<string, ProcessFunc>("stat", boost::bind(&BaseServer::_stat, getPtr(), _1, _2, _3)));
    m_funcTable.insert(make_pair<string, ProcessFunc>("test", boost::bind(&BaseServer::_test, getPtr(), _1, _2, _3)));
}

void BaseServer::_test(const ByteArray &param, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    string resp = "this is response of test";
    string status = "ok";
    int id=0;
    jsonHelper::getInstance()->getField(id, "id", param);
    if(test(id, seq, connection))
    {
        m_io.post(boost::bind(&BaseServer::_send_test, getPtr(), resp, status, seq, connection));
    }
}

void BaseServer::_send_test(const string& resp, const string& status, const r_int64& seq, const boost::shared_ptr<NetConnection> &connection)
{
    ByteArray response;
    MAKERESPBEGIN(status);
    ADDPARAM("resp", resp);
    MAKERESPEND(response);
    r_int16 cmd = (r_int16)NET_CMD_RPC;
    r_int8 compress = (r_int8)COMPRESS_DOUBLE_ZLIB;
    sendResp(response, cmd, seq, connection, compress);
}
