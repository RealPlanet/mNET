#include "MessageSocket.h"
#include <map>

#include "INETMessage.h"
#include "UnknownMessage.h"

using namespace mnet;

MessageSocket::MessageSocket(SocketOptions options, IMessageReceiver* receiver)
	: AsyncSocket(options)
{
	m_pMessageReceiver = receiver;
	m_pMessageReceiver->SetSocket(this);
}

MessageSocket::~MessageSocket()
{
	delete m_pMessageReceiver;
	for (auto& it : m_mInfos) {
		delete it.second.CurrentMessage;
	}
}

bool MessageSocket::SendMessage(INetMessage* message)
{
	if (!IsClient()) {
		return false;
	}

	if (message == nullptr) {
		return false;
	}

	std::vector<char> data = message->Encode();
	Send(data);
	return true;
}

bool MessageSocket::SendMessageToClient(INetMessage* message, Socket* client)
{
	if (!IsServer()) {
		return false;
	}

	std::vector<char> data = message->Encode();
	SendToClient(data, client);
	return true;
}

void MessageSocket::ProcessMessage(ClientInfo clientInfo, std::vector<char>& data)
{
	if (data.size() == 0) {
		return;
	}

	if (clientInfo.CurrentMessage == nullptr &&
		clientInfo.DataToSkip > 0)
	{
		if (data.size() > clientInfo.DataToSkip)
		{
			auto newDataSize = data.size() - clientInfo.DataToSkip;
			data.resize(newDataSize);
		}
		else
		{
			clientInfo.DataToSkip -= data.size();
			return;
		}
	}

	// Step 1 :: Decode header of message
	if (clientInfo.CurrentMessage == nullptr)
	{
		// Calculate how much data for the header we are missing
		size_t missingDataCount = INetMessage::MESSAGE_HEADER_LEN - clientInfo.Buffer.size();

		auto it = data.end();
		if (data.size() >= missingDataCount) {
			it = data.begin() + missingDataCount;
		}

		clientInfo.Buffer.insert(clientInfo.Buffer.end(), data.begin(), it);

		// Wait for next data if needed
		if (clientInfo.Buffer.size() < INetMessage::MESSAGE_HEADER_LEN) {
			return;
		}

		size_t type;
		size_t messageLength;

		std::memcpy(&type, &clientInfo.Buffer[0], sizeof(type));
		std::memcpy(&messageLength, &clientInfo.Buffer[0] + sizeof(type), sizeof(messageLength));

		clientInfo.CurrentMessage = AllocateNewMessage(type);
		if (clientInfo.CurrentMessage == nullptr) {
			// TODO Report error!
			clientInfo.DataToSkip = messageLength;

			if (m_pMessageReceiver)
			{
				UnknownMessage msg;
				m_pMessageReceiver->Process(clientInfo.Client, &msg);
			}
		}
		else{
			clientInfo.MessageLength = messageLength;
		}

		// We no longer need header data, appen any remaining data as it is part of the message body
		clientInfo.Buffer.clear();
		clientInfo.Buffer.insert(clientInfo.Buffer.end(), data.begin() + INetMessage::MESSAGE_HEADER_LEN, data.end());
	}
	else
	{
		// We are reading the message body insert all the data
		clientInfo.Buffer.insert(clientInfo.Buffer.end(), data.begin(), data.end());
	}

	// We are ready to decode the message
	if (clientInfo.CurrentMessage != nullptr &&
		clientInfo.Buffer.size() == clientInfo.MessageLength) {

		clientInfo.CurrentMessage->Decode(clientInfo.Buffer);
		clientInfo.Buffer.clear();

		if (m_pMessageReceiver)
		{
			m_pMessageReceiver->Process(clientInfo.Client, clientInfo.CurrentMessage);
		}

		delete clientInfo.CurrentMessage;
		return;
	}
}

void MessageSocket::ProcessMessageFromServer(std::vector<char>& data)
{
	static ClientInfo s_clientCache;
	ProcessMessage(s_clientCache, data);
}

void MessageSocket::ProcessMessageFromClient(Socket* sourceSocket, std::vector<char>& data) {
	static std::map<const Socket*, ClientInfo> s_clientMap;

	auto it = s_clientMap.find(sourceSocket);
	if (it == s_clientMap.end()) {

		ClientInfo info;
		info.Client = sourceSocket;
		it = s_clientMap.insert(it, std::make_pair(sourceSocket, info));
	}

	ClientInfo& cache = it->second;
	ProcessMessage(cache, data);
}

void MessageSocket::OnReceiveData(Socket* sourceSocket, std::vector<char>& data)
{
	if (IsClient()) {
		ProcessMessageFromServer(data);
		return;
	}

	if (IsServer()) {
		ProcessMessageFromClient(sourceSocket, data);
	}
}

void MessageSocket::OnClientDisconnected(Socket* socket)
{
	AsyncSocket::OnClientDisconnected(socket);
	if (m_pMessageReceiver != nullptr) {
		m_pMessageReceiver->OnClientDisconnected(socket);
	}
}

void MessageSocket::OnClientConnected(Socket* socket)
{
	AsyncSocket::OnClientConnected(socket);
	if (m_pMessageReceiver != nullptr) {
		m_pMessageReceiver->OnClientConnected(socket);
	}
}

INetMessage* MessageSocket::AllocateNewMessage(size_t type)
{
	if (m_pMessageReceiver == nullptr) {
		return nullptr;
	}

	return m_pMessageReceiver->AllocateMessageOfType(type);
}


