#ifndef __PROXY_INFO_H__
#define __PROXY_INFO_H__
#include "net/NetCommon.h"

namespace net{

    enum PROXY_TYPE
    {
        PROXY_TYPE_NON = 0,
        PROXY_TYPE_HTTP,
        PROXY_TYPE_SOCKS4,
        PROXY_TYPE_SOCKS5
    };

    struct ProxyInfo
    {
        PROXY_TYPE    type;        //代理类型
        string        ip;          //代理ip
        int           port;        //端口
        bool          need_check;  //是否需要验证
        string        username;     //用户名
        string        password;    //密码

        ProxyInfo()
        {
            type        = PROXY_TYPE_NON;
            port        = 0;
            need_check  = false;
        }

        bool isEnabled() const
        {
            return type != PROXY_TYPE_NON;
        }
    };

#pragma pack(push, 1)
    typedef struct
    {
        unsigned char    vn;
        unsigned char    cd;
        unsigned short   port;
        unsigned int     address;
        unsigned char    other;
    }SOCKS4_REQ;

    typedef struct 
    {
        unsigned char    vn;
        unsigned char    cd;
    }SOCKS4_RES;
#pragma pack(pop)


#pragma pack(push, 1)
    typedef struct
    {
        unsigned char    ver;
        unsigned char    nmethods;
        unsigned char    method1;
    }SOCKS5_REQ1;

    typedef struct
    {
        unsigned char    ver;
        unsigned char    method;
    }SOCKS5_RES1;

    typedef struct
    {
        unsigned char    ver;
        unsigned char    command;
        unsigned char    reserved;
        unsigned char    atype;
        unsigned int     address;
        unsigned short   port;
    }SOCKS5_REQ2;

    typedef struct
    {
        unsigned char    ver;
        unsigned char    rep;
        unsigned char    rsv;
        unsigned char    atype;
        unsigned char    other[6];
    }SOCKS5_RES2;

    typedef struct
    {
        unsigned char    ver;
        unsigned char    ulen;
        char             username[255];
        unsigned char    plen;
        char             password[255];
    }SOCKS5_AUTH_REQ;

    typedef struct
    {
        unsigned char    ver;
        unsigned char    status;
    }SOCKS5_AUTH_RES;
#pragma pack(pop)
    
}

#endif
