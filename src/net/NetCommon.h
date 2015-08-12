/**
 *  
 *
 *  @author xujun
 *  @since  2012-12-12
 */
#ifndef _NetCommon_2012_12_12_h__
#define _NetCommon_2012_12_12_h__

#include "common/Stdafx.h"

//  definitions used when using static library

#include <boost/asio/strand.hpp>

#ifdef STATIC_NET
    #define DLL_EXPORT_NET
#else
    #ifdef EXPORT_NET
        #define DLL_EXPORT_NET __declspec(dllexport)
    #else
        #define DLL_EXPORT_NET __declspec(dllimport)
    #endif
#endif

namespace net
{
#ifndef KEEP_ALIVE_INTERVAL
#define KEEP_ALIVE_INTERVAL     5
#endif

#ifndef TIME_OUT_INTERVAL
#define TIME_OUT_INTERVAL       600
#endif


    typedef boost::shared_ptr<boost::asio::strand> StrandPtr;
    
    class NetConnection;
    typedef boost::shared_ptr<NetConnection> NetConnectionPtr;

    class NetClient;
    typedef boost::shared_ptr<NetClient> NetClientPtr;

    class NetServer;
    typedef boost::shared_ptr<NetServer> NetServerPtr;

    class NetSocket;
    typedef boost::shared_ptr<NetSocket> SocketPtr;

    class NetPackage;
    typedef boost::shared_ptr<NetPackage> NetPackagePtr;
}


#endif // NetCommon_h__