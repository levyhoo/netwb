#include "server.h"
#include <iostream>
using namespace std;
#include "Clog.h"
#include <boost/lexical_cast.hpp>


int main(int argc, char** argv)
{
    int threadnum = 1;
    if (argc == 2)
    {
        threadnum = boost::lexical_cast<int>(argv[1]);
    }
    Clog::getInstance()->init("../../config/config.log4cxx", false);
    Clog::getInstance()->enableLog(true);
    boost::asio::io_service io;
    boost::asio::io_service::work wk(io);
    ServerPtr ser = ServerPtr(new Server(io, "127.0.0.1", 12345));
    if (ser != NULL)
    {
        ser->start();
        STDLOG(LLV_INFO, "server start ...");
        ser->regist();
    }
    boost::thread_group ths;
    for (int i=0; i<threadnum; i++)
    {
        ths.create_thread(boost::BOOST_BIND(&boost::asio::io_service::run, &io));
    }
    ths.join_all();
    return 0;
}
