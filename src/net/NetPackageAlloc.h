#ifndef NetPackegeAlloc_2014_3_10_H
#define NetPackegeAlloc_2014_3_10_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "net/NetCommon.h"
#include "common/Constants.h"
//#include <bson/src/stringdata.h>
//#include <bson/src/util/builder.h>
#include "common/DataType.h"
#include "stream.h"

namespace net
{
    class DLL_EXPORT_NET NetPackageAlloc //: public bson::TrivialAllocator
    {
    public:
        NetPackageAlloc(ByteArray& bytes);
        ~NetPackageAlloc();

        static NetPackageAlloc& defaultInstance();

        void* Malloc(size_t sz);
        void* Realloc(void *p, size_t sz);
        void Free(void *p);


    private:
        ByteArray& m_bytes;
    };
}
#endif
