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



namespace iircCommon {
    class Header;
}

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

    UserHandler* t;
    std::function<bool(const iircCommon::Header& header, UserHandler* t)> headerCallback;
    std::function<bool(const iircCommon::Header& header, const std::vector<uint8_t>& data, UserHandler* t)> dataCallback;
    std::function<void(UserHandler* t)> closeCallback;

    typedef std::list<std::weak_ptr<TcpClient>> ClientList;
    std::shared_ptr<ClientList> clients;
    boost::asio::io_service* ioServicePtr = 0;

    void run();
    void accept();
};


#endif //IIRC_TCPINTERFACEIMPL_H
