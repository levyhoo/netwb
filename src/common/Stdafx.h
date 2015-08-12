// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "TargetVer.h"

#ifdef _MSC_VER
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
    #pragma warning(disable:4819 4996 4101 4251 4275 4800)
    #pragma warning (error: 4715)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>

#include <boost/shared_ptr.hpp>
#ifndef WIN32
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#endif

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;

// treat some warnings as errors
#ifdef WIN32
#pragma warning (error: 4002)
#pragma warning (error: 4667)
#pragma warning (error: 4715)
#pragma warning (error: 4390)       // if (xxx) ;
#pragma warning (error: 4172)       // return address of local temp memory
#endif // WIN32
