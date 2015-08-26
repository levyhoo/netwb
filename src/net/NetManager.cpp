#include "common/Stdafx.h"
#include "net/NetManager.h"
//---
#include <iostream>
using namespace std;


using namespace net;


NetManager::NetManager()
{
}

NetManager::~NetManager()
{
    removeAllConnections();
}

void NetManager::addConnection(boost::shared_ptr<NetConnection> connection)
{
    boost::recursive_mutex::scoped_lock rlock(m_mutex);
    m_connectionPool.insert(connection);
}

void NetManager::removeConnection(boost::shared_ptr<NetConnection> connection)
{
    boost::recursive_mutex::scoped_lock rlock(m_mutex);
    connection->close();
    if (m_connectionPool.find(connection) != m_connectionPool.end())
        m_connectionPool.erase(connection);
}

set<boost::shared_ptr<NetConnection> > NetManager::getAllConnections()
{
    boost::recursive_mutex::scoped_lock rlock(m_mutex);
    return m_connectionPool;
}

void NetManager::removeAllConnections()
{
    boost::recursive_mutex::scoped_lock rlock(m_mutex);
    set<boost::shared_ptr<NetConnection> >::iterator iter = m_connectionPool.begin();
    for ( ; iter != m_connectionPool.end(); iter++)
        (*iter)->close();
    m_connectionPool.clear();
}

void NetManager::onDataReceived(const ByteArray& data, r_int32 len, boost::shared_ptr<NetConnection> connection)
{
    connection->m_pkgParser.feedData(data, len);
    do
    {
        NetPackageHeader header;
        unsigned char* p = NULL;
        size_t contentLen = 0;
        bool genNewPack = connection->m_pkgParser.nextPackage(&header, &p, &contentLen);
        if (genNewPack)
        {
            onPackageReceived(header, p, contentLen, connection);
        }
        else
        {
            break;
        }
    }while(true);
}

void NetManager::onPackageReceived(const NetPackageHeader& header, const unsigned char* contentP, const size_t& contentLen, boost::shared_ptr<NetConnection> connection)
{

}