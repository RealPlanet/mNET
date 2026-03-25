#include "mNetServer.h"

using namespace mnet;

mNetServer::mNetServer(ServerOptions options)
{
    SocketOptions socketOptions;
    socketOptions.Address = options.Address;
    socketOptions.Port = options.Port;
    socketOptions.Family = AddressFamily::IPV4;
    socketOptions.Protocol = SocketProtocol::TCP;
    m_pSocket = new Socket(socketOptions);
}
