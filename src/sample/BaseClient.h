#ifndef __BASECLINET_20150807__H_
#define __BASECLINET_20150807__H_
#include <net/NetClient.h>
#include <common/Constants.h>
#include <net/stream.h>
#include <common/NetCommand.h>
using namespace net;

class BaseClient;
typedef boost::shared_ptr<BaseClient> BaseClientPtr;
typedef boost::function<void(ByteArray&, string)> CallBack;
typedef map<r_int64, CallBack> CBTable;

class BaseClient : public NetClient
{
public:
    BaseClient(boost::asio::io_service& ioservice, string serverIp, unsigned short serverPort);
    virtual ~BaseClient(){};

    virtual void onConnectionMade(boost::shared_ptr<NetConnection> connection);

    virtual void onConnectionError(const boost::system::error_code& err, boost::shared_ptr<NetConnection> connection);

    virtual void onPackageReceived(const NetPackageHeader& header, const unsigned char* content, const size_t& contentLen, boost::shared_ptr<NetConnection> connection);

    BaseClientPtr getPtr();
    r_int64 genSeq();
    void request(const ByteArray& param, CallBack cb, r_uint16 cmd, r_uint8 compress = COMPRESS_NONE);

private:
    CBTable m_CBs;
    r_int64 m_requestSeq;
    boost::mutex m_mutex;
};
#endif