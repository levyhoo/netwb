#include "client.h"
#include <iostream>
using namespace std;
#include "Clog.h"
#include <boost/lexical_cast.hpp>
#include "jsonHelper.h"

boost::asio::io_service ioservice;
boost::asio::io_service::work wk(ioservice);
boost::asio::io_service::strand sd(ioservice);


// void sendData(NetClientPtr client)
// {
//     NetPackage pkg;
//     pkg.m_pkgHeader.m_seq = (r_int64)1024 * 1024 * 1024 + 123455;
//     pkg.m_pkgHeader.m_flag.reserved = 15;
//     pkg.m_pkgHeader.m_flag.compress = 6;
//     pkg.m_pkgContent = ByteArray(6, '*');
//     ByteArray arrData = pkg.encode();
//     client->sendData(arrData, arrData.size());
// }
// 
// void testClient(client_pool_ptr& pool)
// {
//     
//     NetClientPtr client = pool->get();
//     if (NULL != client)
//     {
//         client->start(false);
// 
//         asio::deadline_timer sendTimer(ioservice);
//         //sendTimer.expires_from_now(boost::posix_time::seconds(3));
//         //sendTimer.async_wait(boost::bind(&sendData, client));
// 
//         //ioservice.run();
//     }
// 
// }

void send(ClientPtr client)
{
    boost::asio::deadline_timer sendTimer(ioservice);
    sendTimer.expires_from_now(boost::posix_time::seconds(15));
    sendTimer.async_wait(boost::BOOST_BIND(&Client::doSomething, client));
}


int main(int argc, char** argv)
{
    /*
    int threadsnum = 1;
    if (argc == 2)
    {
        threadsnum = boost::lexical_cast<int>(argv[1]);
    }
    Clog::getInstance()->init("../../config/config_client.log4cxx", false);
    Clog::getInstance()->enableLog(true);
    boost::thread_group ths;
    std::vector<ClientPtr> clients;

    for (int i=0; i<threadsnum; i++)
    {
        ClientPtr client = ClientPtr(new Client(ioservice, "127.0.0.1", 12345));
        if (NULL != client)
        {
            clients.push_back(client);
            client->start(false);
            ths.create_thread(boost::BOOST_BIND(send, client));
        }
    }
    
// 
//     ths.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));
//     ths.join_all();
// 
// 
//     boost::asio::io_service iowk;
    client_pool_ptr pool = client_pool_ptr(new client_pool());
    pool->init(1000, boost::bind(&creat_test_client, _1, "127.0.0.1", 12345));
    boost::thread_group ths;
    for (int i=0; i<1000; i++)
    {
        ths.create_thread(boost::bind(testClient, pool));
    }
    ths.join_all(    
    */
    vector<string> ids, rids;
    ids.push_back("1");
    ids.push_back("1");
    ids.push_back("1");
    ids.push_back("1");
    ids.push_back("1");
    ids.push_back("1");
    
    typedef boost::shared_ptr<string> stringPtr;
    ByteArray resp;
    ByteArray req;
    stringPtr name = stringPtr(new string("huliwei"));
    MAKERESPBEGIN("status");
    ADDPARAM("ids", ids);
    ADDPARAM("name", name);
    MAKERESPEND(resp);
    MAKEREQBEGIN("func");
    ADDPARAM("ids", ids);
    ADDPARAM("name", name);
    MAKEREQEND(req);

    cout<<"req: "<<jsonHelper::getInstance()->str(req)<<endl;
    cout<<"resp: "<<jsonHelper::getInstance()->str(resp)<<endl;
    string v;
    jsonHelper::getInstance()->append("hello", 1, req);
    jsonHelper::getInstance()->append("hello", 2, req);
    jsonHelper::getInstance()->append("hello", 3, req);
    jsonHelper::getInstance()->append("hello", 4, req);
    jsonHelper::getInstance()->getField(v, "hello", req);

    
    cout<<"req: "<<jsonHelper::getInstance()->str(req)<<endl;









    


}