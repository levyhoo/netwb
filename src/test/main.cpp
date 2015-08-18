#include <sample/jsonHelper.h>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
using namespace std;

typedef boost::shared_ptr<string> strPtr;
boost::mutex mtx;

namespace data
{
    struct FieldDesc
    {
        string name_;
        string type_;
        FieldDesc(string n, string t):name_(n), type_(t){};
    };

    struct structDesc
    {
        int structId_;
        vector<FieldDesc> fields_;
    };

   

    class testData
    {
    public:
        string name;
        int id;
        double score;

        static structDesc desc;
    };

    void registTestData()
    {
        testData::desc.structId_ = 1;
        testData::desc.push(FieldDesc("name", "std::string"));
        testData::desc.push(FieldDesc("id", "int"));
        testData::desc.push(FieldDesc("score", "double"));
    }


}


void test(int id)
{
    strPtr resp = strPtr(new string("hello"));
    string status("status");
    testData obj;
    obj.id = 12;
    obj.name = "huliwei";
    obj.score = 100;

    ByteArray response;
    MAKERESPBEGIN(status);
    ADDPARAM("resp", resp);
    ADDPARAM("id", id);
    ADDPARAM("obj", obj);
    MAKERESPEND(response);
    {
        //boost::mutex::scoped_lock lock(mtx);
        cout<<"response : "<< jsonHelper::getInstance()->str(response)<<endl;
    }
}

void main()
{
    registTestData();
    boost::thread_group ths;
    for(int i=0; i<1; i++)
    {
        ths.create_thread(boost::bind(&test, i));
    }
    ths.join_all();
}