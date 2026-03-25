#include "UnknownMessage.h"

using namespace mnet;

UnknownMessage::UnknownMessage()
    : INetMessage(TYPE)
{
}

bool UnknownMessage::Decode(const std::vector<char>& data)
{
    return true;
}

std::vector<char> UnknownMessage::Encode()
{
    std::vector<char> buffer = CreateEncodingBuffer();
    EncodeBaseMessageData(buffer);
    return buffer;
}
