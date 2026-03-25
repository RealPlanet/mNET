#include "PingMessage.h"

using namespace mnet;

PingMessage::PingMessage()
	: INetMessage(TYPE)
{
}

bool PingMessage::Decode(const std::vector<char>& data)
{
	return true;
}

std::vector<char> PingMessage::Encode()
{
	std::vector<char> buffer = CreateEncodingBuffer();
	EncodeBaseMessageData(buffer);
	return buffer;
}

PingReply::PingReply()
	: INetMessage(TYPE)
{
	m_Code = 0;
}

bool PingReply::Decode(const std::vector<char>& data)
{
	if (data.size() != sizeof(m_Code))
	{
		return false;
	}

	std::memcpy(&m_Code, data.data(), sizeof(m_Code));
	return true;
}

std::vector<char> PingReply::Encode()
{
	std::vector<char> buffer = CreateEncodingBuffer();
	size_t offset{ buffer.size()};

	buffer.resize(buffer.size() + sizeof(m_Code));
	std::memcpy(&buffer[0] + offset, &m_Code, sizeof(m_Code));
	offset += sizeof(m_Code);

	EncodeBaseMessageData(buffer);
	return buffer;
}
