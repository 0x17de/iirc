//
// Created by it on 1/12/16.
//

#include "TcpClient.h"
#include "TcpInterfaceImpl.h"
#include <iostream>


using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;



TcpClient::TcpClient(TcpInterfaceImpl& tcpInterfaceImpl, tcp::socket socket) : tcpInterfaceImpl(tcpInterfaceImpl), clientList(0), socket(move(socket)) {

}

TcpClient::~TcpClient() {
    if (clientList)
        clientList->erase(it);
}

void TcpClient::run(ClientList* clientList, ClientList::iterator it) {
    this->clientList = clientList;
    this->it = move(it);
    readHeader();
}

void TcpClient::readHeader() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer(&header, sizeof(header)),
                            [this,self] (boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    if (tcpInterfaceImpl.headerCallback(header, tcpInterfaceImpl.t)) {
                                        try {
                                            data.resize(header.length);
                                        } catch(bad_alloc& e) {
                                            cerr << "ClientHeaderException: " << e.what() << endl;
                                            return;
                                        }
                                        readData();
                                    }
                                }
                            });
}

void TcpClient::readData() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer(data.data(), data.size()),
                            [this,self] (boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    if (tcpInterfaceImpl.dataCallback(data, tcpInterfaceImpl.t)) {
                                        try {
                                            data.resize(0);
                                        } catch(bad_alloc& e) {
                                            cerr << "ClientHeaderException: " << e.what() << endl;
                                            return;
                                        }
                                        readHeader();
                                    }
                                } else {
                                    try {
                                        data.resize(0);
                                    } catch(bad_alloc& e) {
                                        cerr << "ClientHeaderException: " << e.what() << endl;
                                        return;
                                    }
                                }
                            });
}
