#include "BasePingMessageReceiver.h"
#include "PingMessage.h"
using namespace mnet;

void BasePingMessageReceiver::Process(Socket* receiver, const INetMessage* message)
{
    m_pCurrentSender = nullptr;
    if (message == nullptr) {
        return;
    }

    m_pCurrentSender = receiver;
    switch (message->Type())
    {
    case PingMessage::TYPE:
    {
        OnPingReceived((const PingMessage*)message);
        break;
    }
    case PingReply::TYPE:
    {
        OnPingReplyReceived((const PingReply*)message);
        break;
    }
    default:
    {
        OnUnknownMessageReceived(message);
        break;
    }
    }

    m_pCurrentSender = nullptr;
}

INetMessage* BasePingMessageReceiver::AllocateMessageOfType(size_t type)
{
    switch (type)
    {
    case PingMessage::TYPE:
    {
        return new PingMessage();
    }
    case PingReply::TYPE:
    {
        return new PingReply();
        break;
    }
    default:
    {
        // TODO Report!
        return nullptr;
    }
    }
}
