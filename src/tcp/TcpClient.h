//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPCLIENT_H
#define IIRC_TCPCLIENT_H


#include <list>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "TcpInterface.h"


class TcpInterfaceImpl;
class TcpClient : public std::enable_shared_from_this<TcpClient> {
    TcpInterfaceImpl& tcpInterfaceImpl;
    boost::asio::ip::tcp::socket socket;
    Header header;
    std::list<std::shared_ptr<TcpClient>>::iterator it;

    void readHeader();
    void readData();

public:
    TcpClient(TcpInterfaceImpl& tcpInterfaceImpl, boost::asio::ip::tcp::socket socket);
    ~TcpClient();

    void run(std::list<std::shared_ptr<TcpClient>>::iterator it);
};


#endif //IIRC_TCPCLIENT_H
