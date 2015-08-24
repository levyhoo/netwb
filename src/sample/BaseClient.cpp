#include "BaseClient.h"
#include "jsonHelper.h"
#include <net/NetPackage.h>

BaseClient::BaseClient(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort)
:NetClient(ioservice, serverIp, serverPort), m_requestSeq(0), m_autoReConnect(true)
{

}

void BaseClient::onConnectionMade(boost::shared_ptr<NetConnection> connection)
{

}

void BaseClient::onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection)
{
    if (!isConnected() && m_autoReConnect)
    {
        start(false);
    }
}

void BaseClient::onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection)
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
            string status;
            if (jsonHelper::getInstance()->getField(status, "status",tmpData))
            {
                r_int64 seq = header.m_seq;
                CBTable::iterator ret = m_CBs.find(seq);
                if (ret != m_CBs.end())
                {
                    ByteArray param;
                    string error;
                    jsonHelper::getInstance()->getField(error, "error",tmpData);
                    if (jsonHelper::getInstance()->getSubJson(param, "param", tmpData))
                    {
                        ret->second(param, error);
                        {
                            boost::mutex::scoped_lock lock(m_mutex);
                            m_CBs.erase(seq);
                        }
                    }
                }

            }
        }
        break;
    case NET_CMD_KEEPALIVE:
        {
            NetPackageHeader header(NET_CMD_KEEPALIVE_RESPONSE, 0, 0, 0);
            header.m_length = sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH;

            ByteArray bytes;
            bytes.resize(sizeof(r_int32) + NET_PACKAGE_HEADER_LENGTH);
            r_int32 pos = header.encode(bytes);
            connection->sendData(bytes, bytes.size());
        }
        break;
    case NET_CMD_KEEPALIVE_RESPONSE: //接收到心跳包反馈
        break;
    default:
        break;
    }
}

BaseClientPtr BaseClient::getPtr()
{
    return boost::shared_dynamic_cast<BaseClient>(shared_from_this());
}

r_int64 BaseClient::genSeq()
{
    boost::mutex::scoped_lock lock(m_mutex);
    r_int64 seq = ++m_requestSeq;
    return seq;
}

void BaseClient::request(const ByteArray& param, CallBack cb, r_uint16 cmd, r_uint8 compress /*= COMPRESS_NONE*/)
{
    r_int64 seq = genSeq();

    ByteArray bytes;
    NetPackageHeader header(cmd, seq, compress, 0);
    if (!zlibCompress(param, bytes, NET_PACKAGE_HEADER_LENGTH))
    {
        header.m_flag.compress = COMPRESS_NONE;
    }
    header.m_length = bytes.size();
    header.encode(bytes);
    if(!m_connection->sendData(bytes, bytes.size()))
    {
        ByteArray ret;
        cb(ret, "send error");
        return;
    }
    if(NULL != cb)
    {
        boost::mutex::scoped_lock lock(m_mutex);
        m_CBs[seq] = cb;
    }
}

void BaseClient::_stat(vector<DataEventRaw>& UpdateEvents, vector<DataEventRaw>& CreditEvents)
{
    ByteArray req;
    MAKEREQBEGIN("stat");
    ADDPARAM("UploadEvents", UpdateEvents);
    ADDPARAM("CreditEvents", CreditEvents);
    MAKEREQEND(req);
    request(req, boost::bind(&BaseClient::_onStat, getPtr(), _1, _2), NET_CMD_RPC, COMPRESS_DOUBLE_ZLIB); 
}

void BaseClient::_onStat(ByteArray& resp, string error)
{
     string strResp = "";
     jsonHelper::getInstance()->getField(strResp, "resp", resp);
     onStat(strResp, error);
}




