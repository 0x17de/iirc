#include "Application.h"
#include "common.pb.h"

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Application app;
    return app.run();
}