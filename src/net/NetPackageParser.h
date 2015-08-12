/**
* 网络包解析器
* 
* @author   huangjian
* @date     2011-12-18
*/


#ifndef __NET_PACKAGE_PARSER__
#define __NET_PACKAGE_PARSER__

#include "net/NetPackage.h"
#include <net/NetCommon.h>
#include "BaseCipher.h"

namespace net
{

    struct DLL_EXPORT_NET EncrytInfo{
        ByteArray  dataBuf;      //缓冲区
        size_t     dataLen;      //缓冲区数据长度
        size_t     dataIndex;    //上次解析缓冲区的位置
        size_t     packSize;    //上次解析缓冲区的位置
        boost::shared_ptr<BaseCipher> cipher;  //解密器
        
        EncrytInfo();
    };

    class DLL_EXPORT_NET NetPackageParser
    {
    public:
        NetPackageParser();

    public:
        /* 重置状态 */
        void reset();

        /* 调整各索引位置 */
        void ajust();

        /* 设置数据 */
        void feedData(const ByteArray& data, size_t len);

        /* 设置pack */
        void feedPack(const ByteArray& data, size_t len);

        /* 生成网络数据包，循环调用，直到返回无效数据 */
        bool nextPackage(NetPackageHeader* header, unsigned char** p, size_t* len, bool* isReset = NULL);

        void setCipher(const boost::shared_ptr<BaseCipher>& cipher);

        bool isInitStatus() const;

    private:
        ByteArray  m_dataBuf;      //缓冲区
        size_t     m_dataLen;      //缓冲区数据长度
        size_t     m_dataIndex;    //上次解析缓冲区的位置
        EncrytInfo m_encryptInfo;
    }; 
}


#endif