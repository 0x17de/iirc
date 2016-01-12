//
// Created by it on 1/12/16.
//

#include "TcpInterface.h"
#include "TcpInterfaceImpl.h"


using namespace std;



TcpInterface::TcpInterface() : impl(make_shared<TcpInterfaceImpl>()) {

}

void TcpInterface::run() {
    impl->run();
}

void TcpInterface::stop() {
    if (impl->ioServicePtr != 0)
        impl->ioServicePtr->stop();
}





