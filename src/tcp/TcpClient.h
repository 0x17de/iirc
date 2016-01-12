//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPCLIENT_H
#define IIRC_TCPCLIENT_H


#include <list>
#include <memory>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "TcpInterface.h"
#include "server.pb.h"



class TcpInterfaceImpl;
class TcpClient : public std::enable_shared_from_this<TcpClient> {
    TcpInterfaceImpl& tcpInterfaceImpl;

    boost::asio::ip::tcp::socket socket;
    std::vector<uint8_t> data;
    iirc::Header header;

    typedef std::list<std::weak_ptr<TcpClient>> ClientList;
    std::shared_ptr<ClientList> clientList;
    ClientList::iterator it;

    void readHeader();
    void readData();

public:
    TcpClient(TcpInterfaceImpl& tcpInterface, boost::asio::ip::tcp::socket socket);
    ~TcpClient();

    void run(std::shared_ptr<ClientList> clientList, ClientList::iterator it);
};


#endif //IIRC_TCPCLIENT_H