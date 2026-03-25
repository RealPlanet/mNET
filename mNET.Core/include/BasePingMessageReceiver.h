#pragma once

#ifndef _H_PING_MESSAGE_RECEIVER_
#define _H_PING_MESSAGE_RECEIVER_

#include "mnet.h"
#include "IMessageReceiver.h"
#include "PingMessage.h"

namespace mnet
{
    class DLL_API BasePingMessageReceiver
        : public IMessageReceiver
    {
    public:
        virtual void Process(Socket* sender, const INetMessage* message) override;
        virtual INetMessage* AllocateMessageOfType(size_t type) override;

        virtual void OnPingReceived(const PingMessage* message) {}
        virtual void OnPingReplyReceived(const PingReply* message) {}
        virtual void OnUnknownMessageReceived(const INetMessage* message) {}

        Socket* GetCurrentSender() { return m_pCurrentSender; }
    protected:
        Socket* m_pCurrentSender;
    };
}

#endif // !_H_PING_MESSAGE_RECEIVER_
