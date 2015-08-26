#include "client.h"
#include <iostream>
using namespace std;
#include "Clog.h"
#include <boost/lexical_cast.hpp>
#include "StructsHelper.h"

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
vector<DataEventRaw> up, crd;
boost::mutex mtx;
void send(ClientPtr client, int num )
{

    for (int i=0; i<num; i++)
    {
        ioservice.post(boost::bind(&Client::test, client, i));
    }
}


int main(int argc, char** argv)
{
    
    int threadsnum = 1;
    int num = 1;
    if (argc == 3)
    {
        threadsnum = boost::lexical_cast<int>(argv[1]);
        num = boost::lexical_cast<int>(argv[2]);
    }
    Clog::getInstance()->init("../../config/config_client.log4cxx", false);
    Clog::getInstance()->enableLog(true);
    boost::thread_group ths;
    std::vector<ClientPtr> clients;
//     DataEventRaw obj, obj1, obj2, obj3;
//     obj.id = 1;
//     obj.activeid = 2;
//     obj.walkdate = 3;
//     obj.userid = 4;
//     strcpy(obj.sourcetable, "hello");
//     memcpy(&obj1, &obj, sizeof(obj));
//     memcpy(&obj2, &obj, sizeof(obj));
//     memcpy(&obj3, &obj, sizeof(obj));
//     obj1.id = 12;
//     obj2.id = 13;
//     obj3.id = 14;
//     up.push_back(obj);
//     up.push_back(obj1);
//     crd.push_back(obj2);
//     crd.push_back(obj3);

    for (int i=0; i<threadsnum; i++)
    {
        ClientPtr client = ClientPtr(new Client(ioservice, "127.0.0.1", 12345));
        if (NULL != client)
        {
            clients.push_back(client);
            client->start(false);
            ths.create_thread(boost::BOOST_BIND(send, client, num));
        }
    }
    

    ths.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));
    ths.join_all();










    


}