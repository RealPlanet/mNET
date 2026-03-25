#pragma once

#ifndef _H_MESSAGE_RECEIVER_
#define _H_MESSAGE_RECEIVER_

#include "mnet.h"
#include "INetMessage.h"

namespace mnet
{
    class MessageSocket;

    INTERFACE IMessageReceiver
    {
    public:
        virtual ~IMessageReceiver() = default;

        virtual void Process(Socket* sender, const INetMessage* message) = 0;
        virtual void OnClientConnected(Socket* client) = 0;
        virtual void OnClientDisconnected(Socket* client) = 0;

        virtual void OnUnknownMessageReceived(const mnet::INetMessage* message) {}

        void SetSocket(MessageSocket* owner) { m_pSocket = owner; }
        MessageSocket* GetSocket() { return m_pSocket; }
        virtual INetMessage* AllocateMessageOfType(size_t type) = 0;

    private:
        MessageSocket* m_pSocket{ nullptr };
    };
}

#endif // !_H_MESSAGE_RECEIVER_


