#include <sample/jsonHelper.h>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
using namespace std;

typedef boost::shared_ptr<string> strPtr;
boost::mutex mtx;


void test(int id)
{
    strPtr resp = strPtr(new string("hello"));
    string status("status");

    vector<string> obj, re;
    obj.push_back("1");
    obj.push_back("2");
    obj.push_back("3");

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
    jsonHelper::getInstance()->getField(re, "obj", response);
}

void main()
{
    boost::thread_group ths;
    for(int i=0; i<1; i++)
    {
        ths.create_thread(boost::bind(&test, i));
    }
    ths.join_all();
}