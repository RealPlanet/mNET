#pragma once

#ifndef _H_ASYNC_SOCKET_
#define _H_ASYNC_SOCKET_

#include <thread>

#include "mnet.h"
#include "Socket.h"

namespace mnet
{
    struct AsyncPollInfo;
    class AsyncSocket;
    // Event callbacks
    constexpr size_t RECEIVE_BUFFER_LEN = 1024;

    class AsyncSocket
        : public Socket {
    public:
        DLL_API AsyncSocket(SocketOptions options);
        DLL_API virtual ~AsyncSocket();

        // Begin listening for messages
        DLL_API void BeginListening();
        // Begin listening for data, accept only a connect at a time
        DLL_API void BeginListeningSingle();

        DLL_API virtual void OnClientConnected(Socket* sock) override;
        DLL_API virtual void OnClientDisconnected(Socket* sock) override;

    protected:
        DLL_API virtual void OnReceiveData(Socket* from, std::vector<char>& data) = 0;

    private:
        std::thread m_internalThread;
        AsyncPollInfo* m_pPollingInfo;

        void ListenThreadClient();
        void ListenThreadServer();
        void SingleListenThreadServer();
    };
}

#endif // !_H_ASYNC_SOCKET_
