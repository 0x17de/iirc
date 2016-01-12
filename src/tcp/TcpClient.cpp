//
// Created by it on 1/12/16.
//

#include "TcpClient.h"
#include "TcpInterfaceImpl.h"
#include <iostream>


using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;



class HeaderPreparation {
public:
    int headerLength;
    HeaderPreparation() {
        iirc::Header header;
        header.set_type(0);
        header.set_length(0);
        headerLength = header.ByteSize();
    }
};
static const HeaderPreparation headerPreparation;


TcpClient::TcpClient(TcpInterfaceImpl& tcpInterfaceImpl, tcp::socket socket) : tcpInterfaceImpl(tcpInterfaceImpl), clientList(0), socket(move(socket)) {

}

TcpClient::~TcpClient() {
    if (clientList != 0)
        clientList->erase(it);
    tcpInterfaceImpl.closeCallback(tcpInterfaceImpl.t);
}

void TcpClient::run(std::shared_ptr<ClientList> clientList, ClientList::iterator it) {
    this->clientList = clientList;
    this->it = move(it);
    readHeader();
}

void TcpClient::readHeader() {
    auto self = shared_from_this();
    try {
        data.resize(headerPreparation.headerLength);
    } catch(bad_alloc& e) {
        cerr << "ClientHeaderException: " << e.what() << endl;
        return;
    }
    boost::asio::async_read(socket,
                            boost::asio::buffer(data.data(), headerPreparation.headerLength),
                            [this,self] (boost::system::error_code ec, std::size_t s) {
                                if (!ec && tcpInterfaceImpl.headerCallback(header, tcpInterfaceImpl.t)) {
                                    cerr << "LEN: " << s << endl;
                                    header.ParseFromString(string((char*)data.data(), headerPreparation.headerLength));
                                    readData();
                                }
                            });
}

void TcpClient::readData() {
    auto self = shared_from_this();
    try {
        data.resize(header.length());
    } catch(bad_alloc& e) {
        cerr << "ClientHeaderException: " << e.what() << endl;
        return;
    }
    boost::asio::async_read(socket,
                            boost::asio::buffer(data.data(), header.length()),
                            [this,self] (boost::system::error_code ec, std::size_t s) {
                                cerr << "LEN: " << s << endl;
                                if (!ec && tcpInterfaceImpl.dataCallback(data, tcpInterfaceImpl.t)) {
                                    readHeader();
                                }
                            });
}
