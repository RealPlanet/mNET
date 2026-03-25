#pragma once

#ifndef _H_MNET_SERVER_
#define _H_MNET_SERVER_

#include "Socket.h"

namespace mnet
{
    struct ServerOptions {
        std::string Address;
        int Port;
    };

    class mNetServer
    {
    public:
        mNetServer(ServerOptions options);
    private:
        Socket* m_pSocket;
    };
}

#endif // !_H_MNET_SERVER_


