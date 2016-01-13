//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACEIMPL_H
#define IIRC_TCPINTERFACEIMPL_H


#include <list>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "common.pb.h"



class UserHandler;
class TcpInterface;
class TcpClient;
class TcpInterfaceImpl {
public:
    TcpInterfaceImpl(TcpInterface& tcpInterface);
    ~TcpInterfaceImpl();

    TcpInterface& tcpInterface;
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::acceptor acceptor;

    std::function<bool(iircCommon::DataType dataType, uint64_t dataSize, UserHandler** t)> headerCallback;
    std::function<bool(iircCommon::DataType dataType, const std::vector<uint8_t>& data, std::shared_ptr<TcpClient> client, UserHandler** t)> dataCallback;
    std::function<void(TcpClient* tcpClient, UserHandler** t)> closeCallback;

    typedef std::list<std::weak_ptr<TcpClient>> ClientList;
    std::shared_ptr<ClientList> clients;
    boost::asio::io_service* ioServicePtr = 0;

    void run();
    void accept();
};


#endif //IIRC_TCPINTERFACEIMPL_H
