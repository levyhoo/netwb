
#ifndef __NET_PACKAGE__
#define __NET_PACKAGE__


#include <net/NetCommon.h>
#include <common/DataType.h>

using namespace std;

namespace net
{
    const r_uint32 NET_PACKAGE_HEADER_LENGTH = 12;                      //网络包头长度
    const r_uint32 NET_PACKAGE_INVALID_LENGTH = 512 * 1024 * 1024;      //非法网络包长度阈值

    /**
    * 压缩方法
    */
    enum NetCompressMethod
    {
        NET_COMPRESS_NONE = 0,  //无压缩
        NET_COMPRESS_ZLIB,      //zlib压缩
        NET_COMPRESS_LZMA,  //lzma压缩
    };

    /**
    * 加密方法 
    */
    enum NetEncrytMethod
    {
        NET_ENCRYPT_NONE = 0,  //无加密
        NET_ENCRYPT_DES,       //DES加密
        NET_ENCRYPT_RSA,       //RSA加密
    };

    /**
    * 保留字段含义
    */
    enum NetReservedMethod
    {
        NET_RESERVED_NONE = 0,
    };

    typedef struct tagNetPackageHeaderFlag
    {
            r_uint8 reserved : 4;
            r_uint8 seq      : 4;
            r_uint8 version  : 2;
            r_uint8 compress : 3;
            r_uint8 encrypt  : 3;
    }NetPackageHeaderFlag;

    /**
    * 网络数据包头
    */
    class DLL_EXPORT_NET NetPackageHeader
    {
    public:
        NetPackageHeader(r_uint16 cmd = 0, r_uint64 seq = 0, r_uint8 compress = 0, r_uint8 encrypt = 0);

        r_int32 encode(ByteArray& data , r_int32 nPos = 0);
        ByteArray encode();

        r_int32 decode(const ByteArray& data , r_int32 nPos = 0);
        r_int32 decode(const r_uint8* data , r_int32 nPos = 0);

    public:
        r_uint32 m_length;
        r_uint64 m_seq;
        r_uint16 m_cmd; 
        NetPackageHeaderFlag    m_flag;  
    };

    /**
    * 网络数据包
    */
    class DLL_EXPORT_NET NetPackage
    {
    public:
        NetPackage();

        /**
        * <p>将网络包转化为二进制流, 已包含包长度</p>
        * @param data 输出buf
        * @param nPos 输出buf的起始输出位置
        * @return 输出buf的输出结束位置
        */
        r_int32 encode(ByteArray& data, r_int32 nPos = 0);

        /**
        * <p>将网络包转化为二进制流, 已包含包长度</p>
        * @return 二进制流
        */
        ByteArray encode();

        /**
        * <p>从二进制流中解析网络包</p>
        * @param data 二进制流
        * @param pkgLength 包长度
        * @param nPos 二进制流的解析起始位置
        * @return 解析完后二进制流的结束位置
        */
        r_int32 decode(const ByteArray& data, r_int32 pkgLength, r_int32 nPos = 0);

        /**
        * <p>从二进制流中解析网络包</p>
        * @param data 二进制流
        * @param pkgLength 包长度
        * @param nPos 二进制流的解析起始位置
        * @return 解析完后二进制流的结束位置
        */
        r_int32 decode(const r_uint8* data, r_int32 pkgLength, r_int32 nPos = 0);

    public:
        NetPackageHeader  m_pkgHeader;
        ByteArray         m_pkgContent;
    };

}

#endif
