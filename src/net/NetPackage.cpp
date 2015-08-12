#include <common/Stdafx.h>
#include <net/NetPackage.h>
#ifdef _WIN32_WINNT
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

using namespace net;


NetPackageHeader::NetPackageHeader(r_uint16 cmd, r_uint64 seq, r_uint8 compress, r_uint8 encrypt)
{
    m_length = NET_PACKAGE_HEADER_LENGTH;
    m_cmd = cmd;
    m_seq = seq;
    m_flag.compress = compress;
    m_flag.encrypt = encrypt;
    m_flag.reserved = 0;
    m_flag.seq = m_seq >> 32;
    m_flag.version = 0;
}

r_int32 NetPackageHeader::encode(ByteArray& data , r_int32 nPos)
{
    if (data.size() < nPos + NET_PACKAGE_HEADER_LENGTH)
    {
        data.resize(nPos + NET_PACKAGE_HEADER_LENGTH);
    }

    r_uint32 netLen = htonl(m_length);
    r_uint32 netSeq = htonl((r_uint32)m_seq);
    r_uint16 netCmd = htons(m_cmd);

    memcpy(&data[nPos], &netLen, sizeof(r_uint32));
    nPos += sizeof(r_uint32);
    memcpy(&data[nPos], &netSeq, sizeof(r_uint32));
    nPos += sizeof(r_uint32);
    memcpy(&data[nPos], &netCmd, sizeof(r_uint16));
    nPos += sizeof(r_uint16);

    m_flag.seq = m_seq >> 32;
    r_uint16 flag = (((m_flag.reserved & 0x0f) << 12) | ((m_flag.seq & 0x0f) << 8) | ((m_flag.version & 0x03) << 6) | ((m_flag.encrypt & 0x07) << 3) | (m_flag.compress & 0x07));
    flag = htons(flag);
    memcpy(&data[nPos], &flag, sizeof(r_uint16));
    nPos += sizeof(r_uint16);

    return nPos;
}

ByteArray NetPackageHeader::encode()
{
    ByteArray bArr;
    encode(bArr, 0);

    return  bArr;
}

r_int32 NetPackageHeader::decode(const ByteArray& data , r_int32 nPos)
{
    return decode(&data[0], nPos);
}

r_int32 NetPackageHeader::decode(const r_uint8* data , r_int32 nPos)
{
    m_length = *(r_uint32*)&data[nPos];
    m_length = ntohl(m_length);
    nPos += sizeof(r_uint32);

    m_seq = *(r_uint32*)&data[nPos];
    m_seq = ntohl((r_uint32)m_seq);
    nPos += sizeof(r_uint32);

    m_cmd = *(r_uint16*)&data[nPos];
    m_cmd = ntohs(m_cmd);
    nPos += sizeof(r_uint16);

    r_uint16 flag;
    memcpy(&flag, &data[nPos], sizeof(r_uint16));
    flag = ntohs(flag);
    m_flag.reserved = (flag >> 12) & 0x0f;
    m_flag.seq = (flag >> 8) & 0x0f;
    m_flag.version = (flag >> 6) & 0x03;
    m_flag.encrypt = ( flag >> 3) & 0x07;
    m_flag.compress = flag & 0x07;

    //序号的完整值
    m_seq = ((r_uint64)m_flag.seq << 32) | m_seq;

    nPos += sizeof(r_uint16);

    return nPos;
}

NetPackage::NetPackage()
{
    
}

r_int32 NetPackage::encode(ByteArray& data, r_int32 nPos)
{
    m_pkgHeader.m_length = NET_PACKAGE_HEADER_LENGTH + m_pkgContent.size();
    
    if (data.size() < m_pkgHeader.m_length + nPos)
    {
        data.resize(m_pkgHeader.m_length + nPos);
    }
    nPos = m_pkgHeader.encode(data, nPos);
    if (m_pkgContent.size() > 0)
    {
        memcpy(&data[nPos], &m_pkgContent[0], m_pkgContent.size());
    }
    nPos += m_pkgContent.size();

    return nPos;
}


ByteArray NetPackage::encode()
{
    ByteArray bArr;

    encode(bArr, 0);

    return bArr;
}


r_int32 NetPackage::decode(const ByteArray& data, r_int32 pkgLength, r_int32 nPos)
{
    return decode(&data[0], pkgLength, nPos);
}

r_int32 NetPackage::decode(const r_uint8* data, r_int32 pkgLength, r_int32 nPos)
{
    nPos = m_pkgHeader.decode(data, nPos);

    m_pkgContent.resize(pkgLength - NET_PACKAGE_HEADER_LENGTH);
    if (m_pkgContent.size() > 0)
    {
        memcpy(&m_pkgContent[0], &data[nPos], m_pkgContent.size());
    }
    nPos += m_pkgContent.size();

    return nPos;
}
