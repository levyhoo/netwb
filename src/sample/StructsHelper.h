#ifndef _STRUCTSHELPER_H
#define _STRUCTSHELPER_H
#include "jsonHelper.h"




class DataEventRaw
{
public:
    int		id;
    int		userid;
    int		activeid;
    int		walkdate;
    char	sourcetable[50];
} ;

template <>
class Codec<DataEventRaw>
{
    CODEC_DECLEAR();
public :
    void getField(const string& field, DataEventRaw& value, const ByteArray& strJson)
    {
        ptree pt, pts;
        std::stringstream ss;
        ss << (char *)&strJson[0];
        read_json(ss, pts);
        pt = pts.get_child(field);
        value.id = pt.get<int>("id");
        value.userid = pt.get<int>("userid");
        value.activeid = pt.get<int>("activeid");
        value.walkdate = pt.get<int>("walkdate");
        string sourcetable = pt.get<string>("sourcetable");
        memcpy(value.sourcetable, sourcetable.c_str(), sourcetable.length() + 1);
    }
    void makeJsonPt(const string& field, const DataEventRaw& value, ptree& pts)
    {
        ptree pt;
        pt.put("id", value.id);
        pt.put("userid", value.userid);
        pt.put("activeid", value.activeid);
        pt.put("walkdate", value.walkdate);
        pt.put("sourcetable", value.sourcetable);
        pts.put_child(field, pt);
    }
    void write(DataEventRaw& value, ptree::value_type& pts)
    {
        ptree pt = pts.second;
        value.id = pt.get<int>("id");
        value.userid = pt.get<int>("userid");
        value.activeid = pt.get<int>("activeid");
        value.walkdate = pt.get<int>("walkdate");
        string sourcetable = pt.get<string>("sourcetable");
        memcpy(value.sourcetable, sourcetable.c_str(), sourcetable.length() + 1);
    }
    void makeArrayElement(const DataEventRaw& value, ptree& pts)
    {
        ptree pt;
        pt.put("id", value.id);
        pt.put("userid", value.userid);
        pt.put("activeid", value.activeid);
        pt.put("walkdate", value.walkdate);
        pt.put("sourcetable", value.sourcetable);
        pts.push_back(std::make_pair("", pt));
    }
};

#endif