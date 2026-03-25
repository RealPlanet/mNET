#include "mnet.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

static int InitOSSpecific() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

static int DisposeOSSpecific() {
    return WSACleanup();
}

#elif UNIX

static int InitOSSpecific() {
}

static int DisposeOSSpecific() {
}

#else
#error NO_ARCHITECTURE_SET
#endif // _WIN32


int mnet::InitializeLibrary()
{
    return InitOSSpecific();
}

int mnet::DisposeLibrary()
{
    return DisposeOSSpecific();
}
