#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp> 
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/any.hpp>
#include <net/NetClient.h>
#include <net/NetServer.h>
#include <boost/lexical_cast.hpp>
#include <vector>
#include "io_service_pool.h"
#include <boost/threadpool.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
using namespace std;
using namespace net;


/*

1. bjam�����������

bjam stage --toolset=msvc-9.0 --without-python --stagedir="E:\SDK\boost\bin\vc9" link=static runtime-link=shared runtime-link=static threading=multi debug release
��1��stage/install��

stage��ʾֻ���ɿ⣨dll��lib����install�������ɰ���ͷ�ļ���includeĿ¼�������Ƽ�ʹ��stage����Ϊinstall���ɵ����includeĿ¼ʵ�ʾ���boost��װ����ѹ�����boostĿ¼��E:\SDK\boost\boost��ֻ��includeĿ¼�༸����hpp�ļ�������С�������Կ���ֱ��ʹ�ã����Ҳ�ͬ��IDE������ʹ��ͬһ��ͷ�ļ��������Ƚ�ʡ����ʱ�䣬Ҳ��ʡӲ�̿ռ䡣

��2��toolset��

ָ������������ѡ����borland��gcc��msvc-9.0��VS2008���ȡ�

��3��without/with��

�鿴boost�������������bjam --show-libraries��

��4��stagedir/prefix��

stageʱʹ��stagedir��installʱʹ��prefix����ʾ���������ļ���·�����Ƽ�����ͬ��IDEָ����ͬ��Ŀ¼����VS2008��Ӧ����E:\SDK\boost\bin\vc9��VC6��Ӧ����E:\SDK\boost\bin\vc6���������ɵ�һ��Ŀ¼���棬���Թ������ʹ����install��������ô��������ͷ�ļ�Ŀ¼��vc9��Ӧ�ľ���E:\SDK\boost\bin\vc9\include\boost-1_46\boost,

��5��build-dir��

�������ɵ��м��ļ���·���������������û�õ���Ĭ�Ͼ��ڸ�Ŀ¼��E:\SDK\boost���£�Ŀ¼��Ϊbin.v2���ȱ�����ɺ�ɽ����Ŀ¼ȫ��ɾ����û���ˣ������Բ���Ҫȥ���á�

��6��link��

���ɶ�̬���ӿ�/��̬���ӿ⡣���ɶ�̬���ӿ���ʹ��shared��ʽ�����ɾ�̬���ӿ���ʹ��static��ʽ��һ��boost����ܶ�����static��ʽ���룬��Ϊ���շ����������boost��dll�о���Ƚ���׸��

��7��runtime-link��

��̬/��̬����C/C++����ʱ�⡣ͬ����shared��static���ַ�ʽ������runtime-link��linkһ�����Բ���4����Ϸ�ʽ�����˿��Ը����Լ�����Ҫѡ����롣

��8��threading��

��/���̱߳��롣һ�㶼д���̳߳��򣬵�ȻҪָ��multi��ʽ�ˣ������Ҫ��д���̳߳�����ô����Ҫ���뵥�߳̿⣬����ʹ��single��ʽ��

��9��debug/release��

����debug/release�汾��һ�㶼�ǳ����debug�汾��Ӧ���debug�汾���������������롣        link= static �� ��̬�⡣ ���ɵĿ��ļ������� ��lib����ͷ
link= shared �� ��̬�⡣���ɵĿ��ļ������ޡ�lib����ͷ

threading= mult : ֧�ֶ��̡߳� ���ɵĿ��ļ������а��� ��-mt��

variant=release  ���ɵĿ��ļ����Ʋ����� ��-gd��
variant= debug  ���ɵĿ��ļ����ư��� ��-gd��

runtime-link= static  ���ɵĿ��ļ����ư��� ��-s��
runtime-link= shared  ���ɵĿ��ļ����Ʋ����� ��-s��

*/

//shared_ptr scoped_ptr intrusive_ptr weak_ptr shared_array scoped_array
void testShared()
{
    cout<<__FUNCTION__ <<endl;
    boost::shared_ptr<string> str(new string("shared_ptr"));
    boost::shared_ptr<string> str1 = boost::make_shared<string>("hello1");
    boost::shared_ptr<string> str2 = boost::shared_ptr<string>(new string("hello2"));
    boost::shared_ptr<string> str3 = str2;
    str2.reset(new string("test"));
}

void testScoped()
{
    cout<<__FUNCTION__ <<endl;
    boost::scoped_ptr<string> str1 (new string("testScoped"));
    //boost::scoped_ptr<string> str2 = str1; error
    str1.reset(new string("test"));

}

void testweak()
{
    boost::shared_ptr<int> sp(new int(1));
    assert(sp.use_count() == 1);
    boost::weak_ptr<int> wp(sp);
    if (!wp.expired())
    {
        boost::shared_ptr<int> sp1 = wp.lock();
        assert(wp.use_count() == 2);
    }
    assert(wp.use_count() == 1);
}
//enable_shared_from_this



namespace {
    class A;
    class B;

    typedef boost::shared_ptr<A> A_ptr;
    typedef boost::shared_ptr<B> B_ptr;

    class A
    {
    public:
        ~A(){cout<<"~A"<<endl;}
        //B_ptr b;
        boost::weak_ptr<B> b;
    };

    class B
    {
    public:
        ~B(){cout<<"~B"<<endl;}
        //A_ptr a;
        boost::weak_ptr<A> a;
    };

    void testWk()
    {
        A_ptr objA(new A);
        B_ptr objB(new B);

        objA->b = objB;
        objB->a = objA;

//         assert(objA.use_count() == 2);
//         assert(objB.use_count() == 2);
    }

}



//function bind
namespace testfunction
{
    void dod(string str)
    {
        cout<<str<<endl;
    }

    void dodo(string str1, string str2)
    {
        cout<<str1<<str2<<endl;
    }

    class A 
    {
    public:
        void test(std::string ss)
        {
            cout<<ss<<endl;
        }
    };

    void test()
    {
        boost::function<void(string)> f;
        f = &dod;
        f("testfunction");

        f = boost::bind(dodo, _1, " for bind");
        f("testfunction");

        A a;
        f = boost::bind(&A::test, &a, _1);
        f("testfunction");

    }
}

//thread

namespace testthread
{
    // mutex recursive_mutex shared_mutex timed_mutex
    // unique_lock shared_lock lock_guard
    typedef boost::shared_lock<boost::shared_mutex> ReadLock;
    typedef boost::unique_lock<boost::shared_mutex> WriteLock;

    boost::mutex mutex;    
    void threadfun1()  
    {  
        cout<<"begin threadfun1"<<endl;
        boost::lock_guard<boost::mutex> lock(mutex);  
        cout<<"end threadfun1"<<endl;
    }    

    void threadfun2()  
    {  
        cout<<"begin threadfun2"<<endl;
        boost::lock_guard<boost::mutex> lock(mutex);  
        threadfun1();  
        cout<<"end threadfun2"<<endl;
    }  

    boost::recursive_mutex rec_mutex;   
    void threadfun3()  
    {  
        cout<<"begin threadfun3"<<endl;
        boost::recursive_mutex::scoped_lock lock(rec_mutex);   
        cout<<"end threadfun3"<<endl;
    }    

    void threadfun4()  
    {  
        cout<<"begin threadfun4"<<endl;
        boost::recursive_mutex::scoped_lock lock(rec_mutex);   
        threadfun3();  
        cout<<"end threadfun4"<<endl;
    }  

    void testThreadLock()
    {
        boost::thread t(&threadfun2);
        boost::thread t1(&threadfun4);
        t1.join();
        t.join();
        //t.timed_join(boost::posix_time::seconds(3));
    }

    void testThreadGroup()
    {
        boost::thread_group ths;
        boost::thread* th = ths.create_thread(&threadfun2);
        ths.add_thread(new boost::thread(threadfun2));
        ths.join_all();
//         delete th;
//         ths.remove_thread(th);
    }

    boost::condition con;
    boost::condition con1;
    int n;
    void get()
    {
        do 
        {
            boost::mutex::scoped_lock lock(mutex);
            while (n==0)
            {
                con.wait(mutex);
            }
            n--;
            cout<<boost::this_thread::get_id()<<"get "<< n <<endl;
            boost::this_thread::sleep(boost::posix_time::microseconds(200));
            con1.notify_one();

        } while (1);

    }

    void put()
    {
        do 
        {
            boost::mutex::scoped_lock lock(mutex);
            while (n==10)
            {
                con1.wait(mutex);
            }
            n++;
            cout<<boost::this_thread::get_id()<<"put "<< n <<endl;
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            con.notify_one();

        } while (1);
    }

    void test()
    {
        boost::thread_group ths;
        ths.create_thread(&get);
        ths.create_thread(&get);
        ths.create_thread(&get);
        ths.create_thread(&put);
        ths.join_all();
    }


}


void testRgx()
{
    {
        std::string str("tencent");
        boost::regex reg( "t\\w*t" );
        if (regex_match(str, reg))
        {
            std::cout << str << " is match" << std::endl;
        }
        else
        {
            std::cout << str << " is not match" << std::endl;
        }
    }
    {
        //smatch wsmatch cmatch wcmatch
        const char* mail = "tencent@qq.com";
        boost::cmatch res;
        boost::regex reg("(\\w+)@(\\w+).(\\w+)");
        if (boost::regex_match(mail,res, reg))
        {
            for (boost::cmatch::iterator pos = res.begin(); pos != res.end(); ++pos)
            {
                std::cout << *pos << std::endl;
            }
            std::cout << "name:" << res[1] << std::endl;
        }

    }
    {
        std::string mail("tencent@qq.com.cn");
        boost::regex reg("(\\w+)@(\\w+).(\\w+)");
        std::cout << boost::regex_replace(mail, reg, "$1@139.$3") << std::endl;
        std::cout << boost::regex_replace(mail, reg, "my$1@$2.$3") << std::endl;
    }
}

void testfilesystem()
{
    {
        boost::filesystem::path cur_path = boost::filesystem::current_path();
        cout<<"current path "<<cur_path<<endl;
    }
    {
        boost::filesystem::path file("d:/test/test.txt");
        boost::filesystem::path dir("d:/test");
        boost::filesystem::recursive_directory_iterator d(dir); //directory_iterator
        while (d != boost::filesystem::recursive_directory_iterator())
        {
            cout<<*d<<endl;
            d++;
        }
        if (boost::filesystem::is_directory(dir))
        {
            cout<<dir<<" is directory"<<endl;
        }
        if (boost::filesystem::is_regular_file(file))
        {
            cout<<file<<" is file and size : "<<boost::filesystem::file_size(file)<<endl;
            boost::filesystem::remove(file);
        }
    }
}

typedef boost::signals2::signal<void(string msg)> sigType;
void doOne(string msg){cout<<"doOne"<<endl;}
void doTwo(string msg, int a){cout<<"doTwo"<<endl;}
void testSingal()
{
    //observer
    sigType sig;
    boost::signals2::connection c1 = sig.connect(&doOne);
    boost::signals2::connection c2 = sig.connect(boost::bind(&doTwo,_1,1));
    sig("hello");

    c2.disconnect();
    sig("hello");

}


void doTestServer()
{
    boost::asio::io_service io;
    boost::asio::io_service::work wk(io);
    NetServerPtr sev = NetServerPtr(new NetServer(io, "127.0.0.1", 12345));
    sev->start();
    boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
    t.join();
}

// void doTestClient()
// {
//     io_service_pool pool(20);
//     boost::asio::io_service io;
//     boost::asio::io_service::work wk(io);
//     std::vector<NetClientPtr> clients;
//     string msg("send messages");
//     for (int i=0; i< 100; i++)
//     {
//         NetClientPtr cli = NetClientPtr(new NetClient(io, "127.0.0.1", 12345));
//         clients.push_back(cli);
//         cli->start();
//         string str = msg + boost::lexical_cast<string>(i);
//         cli->sendData(str.c_str(), str.size());
//         cout<<"senddatas : "<<str<<endl;
//     }
//     boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
//     t.join();
// }

void testNet()
{
	boost::asio::io_service io;
	io.post(&doTestServer);
    
	io.run();//pool.get_io_service()->post(&doTestClient);
}


namespace {
    boost::asio::io_service io;
    boost::asio::strand strand_(io);
    boost::asio::io_service::work wk(io);
    boost::thread_group threads;
    void print(int a)
    {
        static int b = 0;
        cout<<boost::this_thread::get_id()<<" a : "<<a<<" b :"<<b<<endl;
        b++;
    }

#define FUNCST(a) void print_st##a(){strand_.post(boost::bind(print,a));}
#define FUNCIO(a) void print_io##a(){io.post(boost::bind(print,a));}
FUNCIO(1)
FUNCIO(2)
FUNCIO(3)
FUNCST(1)
FUNCST(2)
FUNCST(3)
    void testStrand()
    {
        for (int i=0; i<2; i++)
        {
            threads.create_thread(boost::bind(&boost::asio::io_service::run, &io));
        }
//         threads.create_thread(print_io1);
//         threads.create_thread(print_io2);
//         threads.create_thread(print_io3);
        threads.create_thread(print_st1);
        threads.create_thread(print_st2);
        threads.create_thread(print_st3);

        threads.join_all();
    }
}


void testany()
{
    cout<<"test any"<<endl;
    {
        typedef std::vector<boost::any> anys;
        anys tmp;
        tmp.push_back(2);
        tmp.push_back(string("test"));
        for(unsigned int i=0;i<tmp.size();++i)
        {
            cout<<tmp[i].type().name()<<endl;
            try
            {
                int result = any_cast<int>(tmp[i]);
                cout<<result<<endl;
            }
            catch(boost::bad_any_cast & ex)
            {
                cout<<"cast error:"<<ex.what()<<endl;
            }
        }
    }
}

void test_format()
{
    cout << boost::format("string : %1%,  double: %2%  int :%3%") % "name" % 12.34 % 56 <<endl;
    format f("string : %1%,  double: %2%  int :%3%"); f % "name" % 12.34 % 56;
    cout << f.str() << endl;
}

namespace thdpool
{
    using namespace boost::threadpool;

    boost::mutex mtx;
    template<typename T>
    void print(T value)
    {
        //boost::mutex::scoped_lock lock(mtx);
        cout<<boost::this_thread::get_id()<<" value : "<<value<<endl;
    }
#define FUNC(a) void task##a(){print(a);}

    FUNC(1);
    FUNC(2);
    FUNC(3);
    FUNC(4);
    FUNC(5);

    int task()
    {
        return 10;
    }
    int taskt()
    {
        return 5;
    }


    void FifoPool()
    {
        cout<<__FUNCTION__<<endl;

        pool tp(5);
        tp.schedule(&task1);
        tp.schedule(&task2);
        tp.schedule(&task3);
        tp.schedule(&task4);
        tp.schedule(&task5);
        size_t active_threads   = tp.active();
        size_t pending_threads  = tp.pending();
        size_t total_threads    = tp.size();
        //tp.size_controller().resize(10);
        cout<<"pool size " << tp.size()<<endl;
        tp.wait();
    }

    void LifoPool()
    {
        cout<<__FUNCTION__<<endl;
        lifo_pool tp(5);
        tp.schedule(&task1);
        tp.schedule(&task2);
        tp.schedule(&task3);
        tp.schedule(&task4);
        tp.schedule(&task5);
        cout<<"pool size " << tp.size()<<endl;
        tp.wait();
    }
    void PrioPool()
    {
        cout<<__FUNCTION__<<endl;

        prio_pool tp(5);
        schedule(tp, prio_task_func(1, &task1));
        schedule(tp, prio_task_func(2, &task2));
        schedule(tp, prio_task_func(2, &task3));
        schedule(tp, prio_task_func(6, &task4));
        schedule(tp, prio_task_func(5, &task5));
        tp.wait();
    }

    void Future()
    {
        fifo_pool tp(5);
        future<int> fut = schedule(tp, &task);
        future<int> futt = schedule(tp, &taskt);
        int ret = fut();
        int rett = futt();
        cout<<"ret : "<<ret <<endl;
    }

    void pause()
    {
        cout<<__FUNCTION__<<boost::this_thread::get_id()<<endl;

        while (1)
        {
            ;
        }
    }
    io_service_ptr io1;

//     void postOther()
//     {
//         boost::this_thread::sleep(boost::posix_time::seconds(3));
//         io1->post(&task1);
//         io1->post(&task5);
//         io1->post(&task5);
//         io1->post(&task5);
//         io1->post(&task5);
//     }

    void  ioPool()
    {
        cout<<__FUNCTION__<<boost::this_thread::get_id()<<endl;

        io_service_pool pool(5);
        io1 = pool.get_io_service();
        io_service_ptr io2 = pool.get_io_service();

       // io1->post(&task1);
        io1->post(&task2);
        io2->post(&task3);
        io1->post(&task4);
        io1->post(&pause);
        io2->post(&task5);
        boost::thread_group gp;
        gp.create_thread(boost::bind(&boost::asio::io_service::run, io1));
        gp.create_thread(boost::bind(&boost::asio::io_service::run, io2));
        //gp.create_thread(&postOther);
        gp.join_all();
    }
}



namespace timertest
{
    boost::asio::io_service io;
    boost::asio::deadline_timer timer(io);

    void testTimer()
    {
        using namespace boost::posix_time;
        boost::system::error_code error;
        timer.cancel(error);
        timer.expires_from_now(boost::posix_time::seconds(3));
        timer.async_wait(boost::bind(&testTimer));
        ptime t(second_clock::local_time()); //second_clock::universal_time() UTC
        string now = to_simple_string(t);
        cout<<"time : "<< now <<endl;
    }
}

int main(int argc, char** argv)
{
    //testStrand();
//     testNet();
//     while (true)
//     {
//         boost::this_thread::sleep(boost::posix_time::seconds(3));
//     }

//     {
//         thdpool::FifoPool();
//         thdpool::LifoPool();
//         thdpool::PrioPool();
//         thdpool::Future();
//         thdpool::ioPool();
//     }

//     {
//         timertest::testTimer();
//         timertest::io.run();
//     }
//     {
//         testShared();
//         testScoped();
//     }
//     {
//         testfunction::test();
//     }
    {
		testNet();
    }
    return 0;
}