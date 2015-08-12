﻿#include "common/Stdafx.h"
#include "common/MarketType.h"
#include "strings.h"
#include <iconv/iconv.h>
#include "common/type.h"

#ifdef WIN32
list<string> operator<<( const string & left, const string & right )
{
    list<string> l;
    l.push_back(left);
    l.push_back(right);
    return l;
}

list<string> & operator<<( list<string> & left, const string & right )
{
    left.push_back(right);
    return left;
}
#endif // WIN32

namespace net
{
    typedef int (*TRANFORM_STRING_FUNC)(int a);

    string transformString(const string & originalString,TRANFORM_STRING_FUNC func1)
    {
        string ret = originalString;
        transform(ret.begin(),ret.end(),ret.begin(),func1);
        return ret;
    }

    string toUpper(const string & originalString)
    {
        return transformString(originalString,toupper);
    }

    string toLower(const string & originalString)
    {
        return transformString(originalString,tolower);
    }

    void string_replace( std::string &strBig, const std::string &strsrc, const std::string &strdst )
    {
        std::string::size_type last_pos = 0;
        std::string::size_type pos = 0;
        std::string::size_type srclen = strsrc.size();
        std::stringstream ss;

        while( (pos=strBig.find(strsrc, pos)) != std::string::npos )
        {
            ss << strBig.substr(last_pos, pos - last_pos) << strdst;
            pos += srclen;
            last_pos = pos;
        }
        ss << strBig.substr(last_pos, strBig.size() - last_pos);
        strBig = ss.str();
    }

    void string_replace( std::string &strBig, char c, const::std::string & strdst )
    {
        std::string::size_type last_pos = 0;
        std::string::size_type pos = 0;
        std::stringstream ss;

        while( (pos=strBig.find(c, pos)) != std::string::npos )
        {
            ss << strBig.substr(last_pos, pos - last_pos) << strdst;
            pos += 1;
            last_pos = pos;
        }
        ss << strBig.substr(last_pos, strBig.size() - last_pos);
        strBig = ss.str();
    }

    void bsonAddQuota( string & str )
    {
        std::string::size_type posSpace = 0;

        while ( (posSpace = str.find(" ", posSpace)) != std::string::npos )
        {
            std::string::size_type posNextSpace = str.find(" ", posSpace + 1);
            std::string::size_type posNextColon = str.find(":", posSpace + 1);
            if (
                std::string::npos != posNextColon
                && (posNextColon - posSpace > 1)
                && (posSpace == str.size() - 1 || str[posSpace + 1] != '"')
                && (str[posNextColon - 1] != '"')
                && (std::string::npos == posNextSpace || posNextSpace > posNextColon)
                )
            {
                size_t i = posSpace + 1;
                size_t n = posNextColon - posSpace - 1;
                str.replace(i, n, "\"" + str.substr(i, n) + "\"");
                posSpace += (n + 2);
            }
            else
            {
                ++posSpace;
            }
        }
    }
    void getIpPort(const string& data, string& ip, unsigned short& port)
    {
        vector<string> items;
        boost::split(items, data, boost::is_any_of(":"));
        if (items.size() == 2)
        {
            ip = items[0];
            port = atoi(items[1].c_str());

        } 
        else 
        {
            ip = "0.0.0.0";
            port = 0;
        }
    }

    void getIpPort(char* data, string& ip, unsigned short& port)
    {
        return getIpPort(string(data), ip, port);
    }


    string gbk2utf8(const string & strIn)
    {
        const char * inbuf = strIn.c_str();
        size_t inlen       = strIn.length();

        size_t bufSize = 2 * inlen;
        char *buf      = new char[bufSize];
        memset(buf, 0, bufSize);
        char *pout = buf;

        iconv_t cd;
        cd = iconv_open("UTF-8", "GBK");
        if ( cd == 0 )
        {
            delete buf;
            return strIn;
        }

        const char **pin = &inbuf;
        size_t rstSize = bufSize;
        if ( iconv(cd, pin, &inlen, &pout, &bufSize) == -1 )
        {
            iconv_close(cd);
            delete buf;
            return strIn;
        }

        iconv_close(cd);
        string strOut = string(buf, rstSize - bufSize);
        delete buf;
        
        return strOut;
    }

    string fixSqlString( const string & content )
    {
        string strOut;
        for (size_t i = 0; i < content.size(); i++)
        {
            if (content[i] == '"')
            {
                strOut += '\\';
            }
            strOut += content[i];
        }
        return strOut;
    }

    string trimQuotation( string strIn )
    {
        string strOut = strIn;
        if (!strOut.empty())
        {
            if (strOut[0] == '\"')
            {
                strOut = strOut.substr(1, strOut.size() - 1);
            }
            if (strOut[strOut.size() - 1] == '\"')
            {
                strOut = strOut.substr(0, strOut.size() - 1);
            }
        }
        return strOut;
    }

    string getMarketName(string market)
    {
        if (market == "SH")
        {
            return "上证所";
        }
        else if (market == "SZ")
        {
            return "深交所";
        }
        else if (market == "CFFEX" || market == "IF")
        {
            return "中金所";
        }
        else if (market == "SHFE" || market == "SF")
        {
            return "上期所";
        }
        else if (market == "DCE" || market == "DF")
        {
            return "大商所";
        }
        else if (market == "CZCE" || market == "ZF")
        {
            return "郑商所";
        }
        else if (market.compare(MARKET_SHANGHAI_STOCK_OPTION) == 0)
        {
            return "上证股票期权";
        }
        return "";
    }

    //规则，如果是非数字字段，保留原样，如果是数字，则根据10进制-1，退位无视被字母分隔
    string getStringDefOne(string inStr)
    {
        size_t sz = inStr.size();
        r_int8 fix = 0;
        for (int i = sz - 1; i >= 0; i--)
        {

            r_int8 tmpK = inStr[i];
            if (tmpK < 48 || tmpK > 57)//非数字
            {
                continue;
            }
            if (tmpK == 48)
            {
                inStr[i] = 57;
                fix = 1;
            }
            else
            {
                if (fix)
                {
                    if (tmpK == 48)
                    {
                        inStr[i] = 57;
                        fix = 1;
                        continue;
                    }
                }
                inStr[i] = tmpK - 1;
                fix = 0;
                break;
            }
        }
        if (fix != 0)
        {
            return "";
        }
        return inStr;
    }
}


