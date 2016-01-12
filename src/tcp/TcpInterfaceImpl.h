//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACEIMPL_H
#define IIRC_TCPINTERFACEIMPL_H


#include <list>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>



class TcpClient;
class TcpInterfaceImpl {
public:
    TcpInterfaceImpl();

    boost::asio::io_service ioService;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::acceptor acceptor;

    std::list<std::shared_ptr<TcpClient>> clients;
    boost::asio::io_service* ioServicePtr = 0;

    void run();
    void accept();
};


#endif //IIRC_TCPINTERFACEIMPL_H
