#include "Socket.h"

using namespace mnet;

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

struct mnet::SocketImpl
{
	sockaddr_in SockAddr = {};
	SOCKET Handle = SOCKET_ERROR;

	unsigned long GetInternalHandle() const { return (unsigned long)Handle; }
};

static bool EnableKeepAlive(SOCKET sock, const KeepAliveConfig& options) {
	bool bKeepAlive = TRUE;
	auto err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));
	if (err != 0)
	{
		std::fprintf(stderr, "Error while enabling keep alive for socket: %d", err);
		return false;
	}

	err = setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&options.IdleTime, sizeof options.IdleTime);
	if (err != 0)
	{
		std::fprintf(stderr, "Error while enabling keep alive for socket: %d", err);
		return false;
	}

	err = setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&options.ProbeInterval, sizeof options.ProbeInterval);
	if (err != 0)
	{
		std::fprintf(stderr, "Error while enabling keep alive for socket: %d", err);
		return false;
	}

	err = setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (char*)&options.ProbeCount, sizeof options.ProbeCount);
	if (err != 0)
	{
		std::fprintf(stderr, "Error while enabling keep alive for socket: %d", err);
		return false;
	}

	return true;
}

static SocketImpl* OS_CreateSocket(SocketOptions options) {

	IPPROTO winProtocol;
	int sockType;
	switch (options.Protocol)
	{
	case SocketProtocol::TCP:
	{
		winProtocol = IPPROTO_TCP;
		sockType = SOCK_STREAM;
		break;
	}
	case SocketProtocol::UDP:
	{
		winProtocol = IPPROTO_UDP;
		sockType = SOCK_DGRAM;
		break;
	}
	default:
		return nullptr;
	}

	int addrFamily;
	switch (options.Family)
	{
	case AddressFamily::IPV4:
	{
		addrFamily = AF_INET;
		break;
	}
	case AddressFamily::IPV6:
	{
		addrFamily = AF_INET6;
		break;
	}
	default:
		return nullptr;
	}

	SOCKET sock = socket(addrFamily, sockType, winProtocol);
	if (sock == INVALID_SOCKET)
	{
		std::fprintf(stderr, "Error while creating socket %d\n", WSAGetLastError());
		return nullptr;
	}

	if (options.Protocol == SocketProtocol::TCP &&
		options.EnableKeepAlive)
	{
		if (!EnableKeepAlive(sock, options.KeepAliveCfg))
		{
			closesocket(sock);
			return nullptr;
		}
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = addrFamily;
	serverAddr.sin_port = htons(options.Port);
	inet_pton(AF_INET, options.Address.c_str(), &serverAddr.sin_addr);

	SocketImpl* socket = new SocketImpl();
	socket->Handle = sock;
	socket->SockAddr = serverAddr;
	return socket;
}

static void OS_CloseSocket(SocketImpl* socket) {
	if (socket == nullptr) {
		return;
	}

	closesocket(socket->Handle);
	socket->Handle = INVALID_SOCKET;
}

static int OS_ConnectAsClient(SocketImpl* socket) {
	if (socket == nullptr) {
		return -1;
	}

	return connect(socket->Handle, (sockaddr*)&(socket->SockAddr), sizeof(socket->SockAddr));
}

static int OS_BindAsServer(SocketImpl* socket) {
	if (socket == nullptr) {
		return -1;
	}

	int bindResult = bind(socket->Handle, (sockaddr*)&(socket->SockAddr), sizeof(socket->SockAddr));
	if (bindResult != 0) {
		std::fprintf(stderr, "Server bind failed: %d\n", WSAGetLastError());
		return bindResult;
	}

	return listen(socket->Handle, SOMAXCONN);
}

static int OS_SendData(SocketImpl* socket, const char* data, size_t len) {
	if (socket == nullptr) {
		return -1;
	}

	return send(socket->Handle, data, (int)len, 0);
}

static int OS_ReceiveData(SocketImpl* socket, char* buffer, size_t bufferLen) {
	if (socket == nullptr) {
		return -1;
	}

	return recv(socket->Handle, buffer, (int)bufferLen, 0);
}

static SocketImpl* OS_AcceptConnection(SocketImpl* socket) {
	SOCKET newSocket = accept(socket->Handle, nullptr, nullptr);
	if (newSocket == INVALID_SOCKET) {
		std::fprintf(stderr, "Accept socket is invalid: %d", WSAGetLastError());
		return nullptr;
	}

	SocketImpl* clientSock = new SocketImpl;
	clientSock->Handle = newSocket;
	return clientSock;
}

#elif UNIX

#else
#error NO_ARCHITECTURE_SET
#endif // _WIN32

Socket::Socket(SocketImpl* source)
	: m_pSocket{ source }
{
}

Socket::Socket(SocketOptions options)
{
	m_pSocket = OS_CreateSocket(options);
}

Socket::~Socket()
{
	for (auto kvp : m_mClients) {
		delete kvp.second;
	}

	OS_CloseSocket(m_pSocket);
	delete m_pSocket;
	m_pSocket = nullptr;
}

Socket* Socket::Accept()
{
	SocketImpl* newSock = OS_AcceptConnection(m_pSocket);
	if (newSock == nullptr) {
		return nullptr;
	}

	Socket* w = new Socket(newSock);
	return w;
}

void Socket::Disconnect() {
	OS_CloseSocket(this->m_pSocket);
	InvokeClientDisconnected(this);
}

bool Socket::DisconnectClient(Socket* sock)
{
	if (!IsServer() || sock == nullptr) {
		return false;
	}

	// We use the handle as reference in this call back
	// First invoke the disconnection then close the socket for good
	InvokeClientDisconnected(sock);
	OS_CloseSocket(sock->m_pSocket);
	delete sock;
	return true;
}

Socket* Socket::GetClient(int handle)
{
	auto it = m_mClients.find(handle);
	if (it == m_mClients.end()) {
		return nullptr;
	}

	return it->second;
}

void Socket::Send(std::vector<char> data)
{
	OS_SendData(m_pSocket, &data[0], data.size());
}

void Socket::Send(const std::string& data)
{
	OS_SendData(m_pSocket, &data[0], data.size());
}

void Socket::SendToClient(std::vector<char> data, Socket* client)
{
	if (!IsServer()) {
		return;
	}

	if (client == nullptr)
	{
		for (auto sock : m_mClients)
		{
			OS_SendData(sock.second->GetInternalSocket(), &data[0], data.size());
		}
	}
	else
	{
		OS_SendData(client->GetInternalSocket(), &data[0], data.size());
	}
}

std::vector<char> Socket::Receive(size_t bufferLen)
{
	std::vector<char> buffer;
	buffer.resize(bufferLen);
	size_t bytesRead = OS_ReceiveData(m_pSocket, &buffer[0], bufferLen);
	if (bytesRead == (size_t)-1) {
		return {};
	}

	buffer.resize(bytesRead);
	return buffer;
}

int mnet::Socket::GetPort()
{
	sockaddr_in address{};

#ifdef _WIN32
	int addressLength = sizeof(address);
#else
	socklen_t addressLength = sizeof(address);
#endif

	if (getsockname(m_pSocket->Handle, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0)
	{
		return 0;
	}

	return ntohs(address.sin_port);
}

long Socket::GetInternalSocketHandle()
{
	if (!IsValid()) {
		return -1;
	}

	return (long)m_pSocket->GetInternalHandle();
}

void Socket::InvokeClientConnected(Socket* sock)
{
	SocketImpl* internalSocket = sock->m_pSocket;
	auto handle = internalSocket->GetInternalHandle();
	m_mClients.insert(std::make_pair(handle, sock));
	OnClientConnected(sock);
}

void Socket::InvokeClientDisconnected(Socket* sock)
{
	SocketImpl* internalSocket = sock->m_pSocket;
	auto handle = internalSocket->GetInternalHandle();
	m_mClients.erase(handle);

	OnClientDisconnected(sock);
}

int Socket::OpenServer()
{
	m_bIsClient = false;
	m_bIsServer = true;
	return OS_BindAsServer(m_pSocket);
}

int Socket::OpenClient()
{
	m_bIsClient = true;
	m_bIsServer = false;
	return OS_ConnectAsClient(m_pSocket);
}

bool Socket::IsValid() const
{
	if (m_pSocket == nullptr)
	{
		return false;
	}

#ifdef WIN32
	return m_pSocket->Handle != INVALID_SOCKET;
#else
#error Unsupported platform
#endif // WIN32
}
