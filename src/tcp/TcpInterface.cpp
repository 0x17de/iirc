//
// Created by it on 1/12/16.
//

#include "TcpInterface.h"
#include "TcpInterfaceImpl.h"


using namespace std;



TcpInterface::TcpInterface() : impl(make_shared<TcpInterfaceImpl>(*this)) {

}

void TcpInterface::run() {
    impl->run();
}

void TcpInterface::stop() {
    if (impl->ioServicePtr != 0)
        impl->ioServicePtr->stop();
}

void TcpInterface::onHeader(function<bool(const Header& header, void* t)> callback) {
    impl->headerCallback = callback;
}

void TcpInterface::onData(function<bool(const vector<uint8_t>& data, void* t)> callback) {
    impl->dataCallback = callback;
}

void TcpInterface::setCallbackParameter(void *t) {
    impl->t = t;
}
