#include "jsonHelper.h"



bool jsonHelper::getSubJson(ByteArray& value, const string& field, const ByteArray& strJson)
{
    try
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        ptree pt;
        std::stringstream ss((char *)&strJson[0]);
        read_json(ss, pt);
        ptree child = pt.get_child(field);
        std::stringstream ret;
        write_json(ret, child);
        size_t len = ret.str().size() + 1;
        value.resize(len);
        memcpy(&value[0], ret.str().c_str(), len);
        return true;
    }
    catch (...)
    {
    }

    return false; 
}

bool jsonHelper::appendSub(const string& field, const ByteArray& value, ByteArray& strJson)
    {
        if (value.size() == 0)
        {
            return false;
        }
        try
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            ptree ptV, ptJ;
            std::stringstream ssV((char*)&value[0]);
            if (0 != strJson.size())
            {
                std::stringstream ssJ((char*)&strJson[0]);
                read_json(ssJ, ptJ);
            }
            read_json(ssV, ptV);
            ptJ.add_child(field, ptV);
            std::stringstream ssRet;
            write_json(ssRet, ptJ);
            size_t len = ssRet.str().size() + 1;
            strJson.clear();
            strJson.resize(len);
            memcpy(&strJson[0], ssRet.str().c_str(), len);
            return true;
        }
        catch(...)
        {
        }
        return false;
    }


