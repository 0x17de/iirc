#ifndef IIRC_APPLICATION_H
#define IIRC_APPLICATION_H


#include "UserHandler.h"
#include "IniReader.h"
#include "DatabaseHandler.h"
#include <tcp/TcpInterface.h>
#include <map>

class Application {
    IniReader iniReader;
    DatabaseHandler databaseHandler;
    std::map<size_t, UserHandler> userHandlers;
    TcpInterface tcpInterface;

public:
    int run();
};


#endif //IIRC_APPLICATION_H
