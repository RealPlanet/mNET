#pragma once

#ifndef _H_MESSAGE_SOCKET_
#define _H_MESSAGE_SOCKET_

#include <map>

#include "mnet.h"
#include "AsyncSocket.h"
#include "IMessageReceiver.h"
#include "INETMessage.h"

namespace mnet
{
    struct ReceiveBuffer;

    class MessageSocket
        : public AsyncSocket
    {
    public:
        DLL_API MessageSocket(SocketOptions options, IMessageReceiver* receiver);
        DLL_API ~MessageSocket();

        DLL_API bool SendMessage(INetMessage* message);
        DLL_API bool SendMessageToClient(INetMessage* message, Socket* client = nullptr);

    protected:
        struct ClientInfo {
            Socket* Client{ nullptr };
            INetMessage* CurrentMessage{ nullptr };
            size_t DataToSkip{ 0 };
			size_t MessageLength{ 0 };
            std::vector<char> Buffer{};
        };

        DLL_API void ProcessMessage(ClientInfo clientInfo, std::vector<char>& data);
        DLL_API void ProcessMessageFromServer(std::vector<char>& data);
        DLL_API void ProcessMessageFromClient(Socket* senderSocket, std::vector<char>& data);
        DLL_API virtual void OnReceiveData(Socket* senderSocket, std::vector<char>& data) override;
        DLL_API virtual void OnClientDisconnected(Socket* socket) override;
        DLL_API virtual void OnClientConnected(Socket* socket) override;
    private:
        std::map<const Socket*, ClientInfo> m_mInfos;
        IMessageReceiver* m_pMessageReceiver;
        INetMessage* AllocateNewMessage(size_t type);
    };
}

#endif
