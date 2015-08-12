#include <common/Stdafx.h>
#include <net/NetPackageParser.h>
#include "BaseCipher.h"
#ifdef _WIN32_WINNT
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

using namespace boost;
using namespace net;


NetPackageParser::NetPackageParser()
{
    m_dataBuf.resize(64 * 1024);
    reset();
}

void NetPackageParser::reset()
{
    m_dataLen = 0;
    m_dataIndex = 0;
}

void NetPackageParser::ajust()
{
    m_dataLen = m_dataLen - m_dataIndex;
    if (m_dataIndex > 0)
    {
        if (m_dataLen > 0)
        {
            memcpy(&m_dataBuf[0], &m_dataBuf[m_dataIndex], m_dataLen);
        }

        m_dataIndex = 0;
    }
}

void NetPackageParser::feedData(const ByteArray& data, size_t len)
{
    if (!m_encryptInfo.cipher)
    {
        feedPack(data, len);
    } else {
        m_encryptInfo.dataLen += len;
        if (m_encryptInfo.dataBuf.size() < m_encryptInfo.dataLen)
        {
            m_encryptInfo.dataBuf.resize(m_encryptInfo.dataLen);
        }
        memcpy(&m_encryptInfo.dataBuf[m_encryptInfo.dataIndex], &data[0], len);

        while (true)
        {
            if (m_encryptInfo.packSize == 0)
            {
                if ((m_encryptInfo.dataLen - m_encryptInfo.dataIndex) >= sizeof(r_uint32))
                {
                    memcpy(&m_encryptInfo.packSize, &m_encryptInfo.dataBuf[m_dataIndex], sizeof(r_uint32));
                    m_encryptInfo.packSize = ntohl(m_encryptInfo.packSize);
                } else {
                    break  ;
                }
            } 

            if ((m_encryptInfo.dataLen - m_encryptInfo.dataIndex) >= m_encryptInfo.packSize)
            {
                ByteArray bytes(m_encryptInfo.packSize * 2);
                while(true)
                {
                    size_t len = bytes.size();
                    bool nbreak = true;
                    r_uint32 realLen = ntohl(*(r_uint32*)&m_encryptInfo.dataBuf[m_encryptInfo.dataIndex + sizeof(r_uint32)]);
                    r_int32 ret = m_encryptInfo.cipher->decrypt(&m_encryptInfo.dataBuf[m_encryptInfo.dataIndex + 2 * sizeof(r_uint32)], m_encryptInfo.packSize, &bytes[0], &len);
                    switch (ret)
                    {
                    case CIPHER_STATUS_OK:
                        feedPack(bytes, realLen);
                        break;
                    case CIPHER_STATUS_BUFFER_ERROR:
                        bytes.resize(bytes.size() * 2);
                        nbreak = false;
                        break;
                    default:
                        break;
                    }

                    if (nbreak)
                    {
                        break;
                    }
                }
                m_encryptInfo.dataIndex += m_encryptInfo.packSize;
                m_encryptInfo.packSize = 0;
            }
        }

        if (m_encryptInfo.dataIndex > 0)
        {
            m_encryptInfo.dataLen = m_encryptInfo.dataLen - m_encryptInfo.dataIndex;
            if (m_encryptInfo.dataLen > 0)
            {
                memcpy(&m_encryptInfo.dataBuf[0], &m_encryptInfo.dataBuf[m_encryptInfo.dataIndex], m_encryptInfo.dataLen);
            }

            m_encryptInfo.dataIndex = 0;
        }
    }
}

void NetPackageParser::feedPack(const ByteArray& data, size_t len)
{
    if (len <= 0)
    {
        return;
    }
    if (m_dataBuf.size() < m_dataLen + len)
    {
        m_dataBuf.resize(m_dataBuf.size() + len);
    }
    memcpy(&m_dataBuf[m_dataLen], &data[0], len);
    m_dataLen += len;
}


bool NetPackageParser::nextPackage(NetPackageHeader* header, unsigned char** p, size_t* len, bool* isReset)
{
    if(m_dataLen - m_dataIndex >= NET_PACKAGE_HEADER_LENGTH)
    {
        r_uint32 pkgLength;

        // 获取package 长度
        memcpy(&pkgLength, &m_dataBuf[m_dataIndex], sizeof(r_uint32));
        pkgLength = ntohl(pkgLength);

        // 出现异常情况，即过长包或者负长度包
        if (pkgLength < NET_PACKAGE_HEADER_LENGTH || pkgLength > NET_PACKAGE_INVALID_LENGTH)
        {
            if (NULL != isReset)
            {
                *isReset = true;
            }
            reset();
            return false;
        }

        if (m_dataLen - m_dataIndex >= pkgLength)
        {
            size_t nPos = header->decode(m_dataBuf, m_dataIndex);
            *p = &m_dataBuf[nPos];
            *len = pkgLength - NET_PACKAGE_HEADER_LENGTH;

            m_dataIndex += pkgLength;
            return true;
        }
    }

    if (m_dataIndex > 0)
    {
        ajust();
    }

    return false;
}

void NetPackageParser::setCipher(const boost::shared_ptr<BaseCipher>& cipher)
{
    m_encryptInfo.cipher = cipher;
}

bool NetPackageParser::isInitStatus() const
{
    return m_dataIndex == 0 && m_dataLen == 0;
}

EncrytInfo::EncrytInfo(): dataLen(0), dataIndex(0), packSize(0), cipher()
{

}