#include "INetMessage.h"

#include <cassert>

using namespace mnet;

void INetMessage::EncodeBaseMessageData(std::vector<char>& container) const {
	assert(container.size() >= INetMessage::MESSAGE_HEADER_LEN);

    size_t offset{ 0 };
    std::memcpy(&container[0], &m_Type, sizeof(m_Type));
    offset += sizeof(m_Type);

	size_t bodySize = container.size() - INetMessage::MESSAGE_HEADER_LEN;
    std::memcpy(&container[0] + offset, &bodySize, sizeof(bodySize));
    offset += sizeof(bodySize);
}

std::vector<char> INetMessage::CreateEncodingBuffer() const
{
	std::vector<char> buffer;
	buffer.resize(INetMessage::MESSAGE_HEADER_LEN);
    return buffer;
}
