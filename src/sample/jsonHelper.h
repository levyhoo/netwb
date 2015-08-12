#ifndef __JSONHELPER_20150807_H__
#define __JSONHELPER_20150807_H__
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <common/DataType.h>
#include <boost/thread.hpp>
#include "Singleton.h"
#include <boost/foreach.hpp>

using namespace std;
using namespace boost::property_tree;

#define CODEC_DECLEAR() \
public :\
    static Codec& instance()\
{\
    static Codec acodec;\
    return acodec;\
}\
private:\
    Codec(){};\
    Codec(const Codec& acodec){};\
    Codec& operator=(const Codec& rhs){}; 

template <typename Type>
class Codec
{
    CODEC_DECLEAR();
public :

    void read(const string& field, ptree& pt, const ByteArray& strJson)
    {
        std::stringstream ss;
        ss << (char *)&strJson[0];
        read_json(ss, pt);
    }
    void write(Type& value, ptree::value_type& pt)
    {
        value = pt.second.data();
    }
    void getField(const string& field, Type& value, const ByteArray& strJson)
    {

        ptree pt;
        read(field, pt, strJson);
        value = pt.get<Type>(field);
    }

    void makeJson(const string& field, const Type& value, ByteArray& strJson)
    {
        ptree pt;
        std::stringsream ss;
        pt.put(field, value);
        write_json(ss, pt, false);
        size_t len = ss.str().size() + 1;
        strJson.resize(len);
        memcpy(&strJson[0], ss.str().c_str(), len);

    }
    void makeJsonPt(const string& field, const Type& value, ptree& pt)
    {
        pt.put(field, value);
    }
    //
    void makeArrayElement(const Type& value, ptree& pt)
    {
        pt.push_back(std::make_pair<string, Type>("", value));
    }
};

//-------------------------------vector------------------------------
template <typename Type>
class Codec<std::vector<Type> >
{
    CODEC_DECLEAR();
public:
    void getField(const string& field, std::vector<Type>& values, const ByteArray& strJson)
    {
        ptree pt, pts;
        std::stringstream ss;
        ss << (char *)&strJson[0];
        read_json(ss, pt);
        pts = pt.get_child(field);
        BOOST_FOREACH(ptree::value_type& v, pts)
        {
            Type e;
            Codec<Type>::instance().write(e, v);
            values.push_back(e);
        }
    }
    void makeJson(const string& field, const std::vector<Type>& values, ByteArray& strJson)
    {
        ptree pts, pt;
        makeJsonPt(field, values, pts);
        pt.put_child(field, pts);
        std::stringstream ss;
        write_json(ss, pt, false);
        size_t len = ss.str().size() + 1;
        cout<<ss.str();
        strJson.resize(len);
        memcpy(&strJson[0], ss.str().c_str(), len);
    }
    void makeJsonPt(const string& field, const std::vector<Type>& values, ptree& pts)
    {
        size_t len = values.size();
        for(int i=0; i<len; i++)
        {
            Codec<Type>::instance().makeArrayElement(values[i], pts);
        }
    }

};

//--------------------------shared_pt------------------------------------
template <typename Type>
class Codec<boost::shared_ptr<Type> >
{
    CODEC_DECLEAR();
public:
    void read(const string& field, ptree& pt, const ByteArray& strJson)
    {
        std::stringstream ss;
        ss << (char *)&strJson[0];
        read_json(ss, pt);
    }

    void write(boost::shared_ptr<Type>& value, ptree::value_type& pt)
    {
        value = boost::shared_ptr<Type>(new Type(pt.second.data()));
    }

    void getField(const string& field, boost::shared_ptr<Type>& value, const ByteArray& strJson)
    {
        ptree pt;
        read(field, pt, strJson);
        value = boost::shared_ptr<Type>(new Type(pt.get<Type>(field)));
    }
    void makeJson(const string& field, const boost::shared_ptr<Type>& value, ByteArray& strJson)
    {
        ptree pt;
        std::stringstream ss;
        pt.put(field, *value);
        write_json(ss, pt, false);
        size_t len = ss.str().size() + 1;
        strJson.resize(len);
        memcpy(&strJson[0], ss.str().c_str(), len);
    }
    void makeJsonPt(const string& field, const boost::shared_ptr<Type>& value, ptree& pts)
    {
        pts.put(field, *value);
    }
    void makeArrayElement(const boost::shared_ptr<Type>& value, ptree& pt)
    {
        pt.push_back(std::make_pair("", *value));
    }

};

class jsonHelper : public Singleton<jsonHelper>
{
public:
    jsonHelper(){};
    ~jsonHelper(){};

private:
    boost::mutex m_mutex;
public:

    template<typename T>
    bool getField(T& value, const string& field, const ByteArray& strJson)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            Codec<T>::instance().getField(field, value, strJson);
            return true;
        }
        catch (...)
        {
        }

        return false; 
    };

    bool getSubJson(ByteArray& value, const string& field, const ByteArray& strJson);

    template<typename T>
    bool append(const string& field, const T& value, ByteArray& strJson)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            ptree pt;
            if (strJson.size() != 0)
            {
                std::stringstream ss((char *)&strJson[0]);
                read_json(ss, pt);
            }
            pt.add(field, value);
            std::stringstream ssRet;
            write_json(ssRet, pt, false);
            strJson.clear();
            size_t len = ssRet.str().size() + 1;
            strJson.resize(len);
            memcpy(&strJson[0], ssRet.str().c_str(), len);
            return true;
        }
        catch (...)
        {
        }
        return false;
    }
    
    bool appendSub(const string& field, const ByteArray& value, ByteArray& strJson);


    template<typename T>
    bool makeJson(const string& field, const T& value, ByteArray& strJson)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            Codec<T>::instance().makeJson(field, value, strJson);
            return true;
        }
        catch(...)
        {
        
        }
        return false;
    }



    template <typename T>
    bool makeJsonPt(const string& field, const T& value, ptree& pt)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            Codec<T>::instance().makeJsonPt(field, value, pt);
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    bool getJson(ByteArray& resp, ptree& pt, ptree& pts)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            pt.put_child("param", pts);
            std::stringstream ss;
            write_json(ss, pt, false);
            size_t len = ss.str().size() + 1;
            resp.resize(len);
            memcpy(&resp[0], ss.str().c_str(), len);
            return true;
        }
        catch(...)
        {

        }
        
        return false;
    }

    std::string str(ByteArray& ret)
    {
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            std::stringstream ss;
            ss<<(char*)&ret[0];
            return ss.str();
        }
        catch (...)
        {
        }
        return string("");
    }
    
};


#define MAKERESPBEGIN(status) \
    {\
        ptree pt, pts;\
        jsonHelper::getInstance()->makeJsonPt("status", status, pt);

#define ADDPARAM(field, value) \
        jsonHelper::getInstance()->makeJsonPt(field, value, pts);

#define MAKERESPEND(resp) \
        jsonHelper::getInstance()->getJson(resp, pt, pts);\
    }

#define MAKEREQBEGIN(func) \
    {\
        ptree pt, pts;\
        jsonHelper::getInstance()->makeJsonPt("func", func, pt);\

#define MAKEREQEND(req) \
        jsonHelper::getInstance()->getJson(req, pt, pts);\
    }
#endif