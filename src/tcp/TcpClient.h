//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPCLIENT_H
#define IIRC_TCPCLIENT_H


#include <list>
#include <memory>
#include <vector>
#include <streambuf>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "TcpInterface.h"
#include "common.pb.h"



class TcpInterfaceImpl;
class TcpClient : public std::enable_shared_from_this<TcpClient>, public std::stringbuf {
    TcpInterfaceImpl& tcpInterfaceImpl;
    UserHandler* userHandler = 0;

    boost::asio::ip::tcp::socket socket;
    std::vector<uint8_t> data;
    iircCommon::DataType dataType;
    uint64_t dataSize;

    typedef std::list<std::weak_ptr<TcpClient>> ClientList;
    std::shared_ptr<ClientList> clientList;
    ClientList::iterator it;

    void readDataType();
    void readDataSize();
    void readData();

public:
    TcpClient(TcpInterfaceImpl& tcpInterface, boost::asio::ip::tcp::socket socket);
    ~TcpClient();

    void run(std::shared_ptr<ClientList> clientList, ClientList::iterator it);
    void write(const char* data, size_t length);
    virtual int sync(); // ostream flush
};


#endif //IIRC_TCPCLIENT_H
