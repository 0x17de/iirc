//
// Created by it on 1/12/16.
//

#include "TcpInterfaceImpl.h"
#include "TcpClient.h"
#include <iostream>


using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;



TcpInterfaceImpl::TcpInterfaceImpl() : endpoint(tcp::v4(), 12667), socket(ioService), acceptor(ioService, endpoint) {
    ioServicePtr = &ioService;
}

void TcpInterfaceImpl::accept() {
    acceptor.async_accept(socket, [this] (boost::system::error_code ec) {
        cerr << "ACCEPT" << endl;
        if (!ec) {
            clients.push_front(make_shared<TcpClient>(*this, move(socket)));
            clients.front()->run(clients.begin());
        }
        accept();
    });
}

void TcpInterfaceImpl::run() {
    accept();
    ioService.run();
}