#include "BaseServer.h"
#include "jsonHelper.h"
#include <common/Constants.h>
#include <net/stream.h>
#include <common/NetCommand.h>

BaseServer::BaseServer(io_service& ioservice, string listenIp, unsigned short listenPort)
:NetServer(ioservice, listenIp, listenPort)
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
                    onRequest(func, param, header.m_seq, connection);
                }
            }
            else
            {
                //deal exception
            }
        }
        break;
    case NET_CMD_KEEPALIVE:
        {
            //发送保活反馈
            NetPackageHeader header(NET_CMD_KEEPALIVE_RESPONSE, 0, 0, 0);
            header.m_length = sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH;

            ByteArray bytes;
            bytes.resize(sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH);
            r_int32 pos = header.encode(bytes);
            connection->sendData(bytes, bytes.size());
        }
        break;
    case NET_CMD_KEEPALIVE_RESPONSE:
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

