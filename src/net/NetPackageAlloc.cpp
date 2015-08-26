#include "common/Stdafx.h"
#include "common/NetCommand.h"
#include "net/NetPackageAlloc.h"
#include "net/NetPackage.h"
#include "stream.h"
#include "net/NetCommon.h"
#include "common/Constants.h"

namespace net
{
    NetPackageAlloc::NetPackageAlloc(ByteArray& bytes)
        :m_bytes(bytes)
    {
    }

    NetPackageAlloc::~NetPackageAlloc()
    {
    }

    NetPackageAlloc& NetPackageAlloc::defaultInstance()
    {
        static ByteArray bytes;
        static NetPackageAlloc instance(bytes);
        return instance;
    }

    void* NetPackageAlloc::Malloc(size_t sz)
    {
        m_bytes.resize(NET_PACKAGE_HEADER_LENGTH + sz);
        return &m_bytes[NET_PACKAGE_HEADER_LENGTH];
    }

    void* NetPackageAlloc::Realloc(void *p, size_t sz)
    {
        ByteArray raw = m_bytes;
        m_bytes.resize(NET_PACKAGE_HEADER_LENGTH + sz);
        memcpy(&m_bytes[0], &raw[0], raw.size() * sizeof(BYTE));
        return &m_bytes[NET_PACKAGE_HEADER_LENGTH];
    }

    void NetPackageAlloc::Free(void *p)
    {
        m_bytes.clear();
    }

}