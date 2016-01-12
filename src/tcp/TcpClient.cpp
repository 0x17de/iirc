//
// Created by it on 1/12/16.
//

#include "TcpClient.h"
#include "TcpInterfaceImpl.h"


using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;



TcpClient::TcpClient(TcpInterfaceImpl& tcpInterfaceImpl, tcp::socket socket) : tcpInterfaceImpl(tcpInterfaceImpl), socket(move(socket)) {

}

TcpClient::~TcpClient() {

}

void TcpClient::run(list<shared_ptr<TcpClient>>::iterator it) {
    this->it = move(it);
    readHeader();
}

void TcpClient::readHeader() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer(&header, sizeof(header)),
                            [this] (boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    readData();
                                } else {
                                    tcpInterfaceImpl.clients.erase(it);
                                }
                            });
}

void TcpClient::readData() {
    auto self = shared_from_this();
    boost::asio::async_read(socket,
                            boost::asio::buffer(&header, sizeof(header)),
                            [this] (boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    readHeader();
                                } else {
                                    tcpInterfaceImpl.clients.erase(it);
                                }
                            });
}
