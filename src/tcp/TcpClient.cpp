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
    if (clientList != 0)
        clientList->erase(it);
    tcpInterfaceImpl.closeCallback(this, &userHandler);
}

void TcpClient::run(std::shared_ptr<ClientList> clientList, ClientList::iterator it) {
    this->clientList = clientList;
    this->it = move(it);
    readDataType();
}

void TcpClient::readDataType() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer((char*)&dataType, sizeof(uint16_t)),
                            [this,self] (boost::system::error_code ec, std::size_t s) {
                                if (!ec) {
                                    cerr << "READ(Type): " << (uint32_t)dataType << " " << s << endl;
                                    readDataSize(); // continue if header could be parsed
                                }
                            });
}

void TcpClient::readDataSize() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer((char*)&dataSize, sizeof(uint64_t)),
                            [this,self] (boost::system::error_code ec, std::size_t s) {
                                if (!ec && tcpInterfaceImpl.headerCallback(dataType, dataSize, &userHandler)) {
                                    try {
                                        cerr << "READ(Size): " << dataSize << " " << s << endl;
                                        data.resize(dataSize);
                                    } catch(bad_alloc& e) {
                                        cerr << "ClientHeaderException: " << e.what() << endl;
                                        return;
                                    }
                                    readData(); // continue if header could be parsed
                                }
                            });
}

void TcpClient::readData() {
    auto self = shared_from_this();
    try {
        cerr << "RESIZE: " << dataSize << endl;
        if (dataSize > 0xffffff || dataSize <= 0) {
            cerr << "RESIZE: " << dataSize << " REJECTED" << endl;
            return;
        }
        data.resize(dataSize);
    } catch(bad_alloc& e) {
        cerr << "ClientHeaderException: " << e.what() << endl;
        return;
    }
    boost::asio::async_read(socket,
                            boost::asio::buffer(data.data(), dataSize),
                            [this,self] (boost::system::error_code ec, std::size_t s) {
                                cerr << "LEN: " << s << endl;
                                if (!ec && tcpInterfaceImpl.dataCallback(dataType, data, self, &userHandler)) {
                                    readDataType();
                                }
                            });
}

void TcpClient::write(const char *data, size_t length) {
    boost::asio::async_write(socket, boost::asio::buffer(data, length), [](const boost::system::error_code& ec, std::size_t bytes_transferred){});
}

int TcpClient::sync() {
    string s(str());
    str(std::basic_string<char>());
    write(s.c_str(), s.size());
    return 0;
}
