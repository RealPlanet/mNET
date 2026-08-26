#include "AsyncSocket.h"

using namespace mnet;

using ReceiveDataCallback = void(AsyncSocket::*)(Socket*, std::vector<char>);

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

struct mnet::AsyncPollInfo
{
	std::vector<WSAPOLLFD> FileDescriptors;
};

static void OS_InitPolling(AsyncPollInfo* info) {

}

static void OS_StopPolling(AsyncPollInfo* info) {

}

static void OS_AddSocketToPolling(AsyncPollInfo* pollingInfo, int socketHandle) {
	WSAPOLLFD pfd{};
	pfd.fd = socketHandle;
	pfd.events = POLLRDNORM;
	pfd.revents = POLLERR | POLLHUP | POLLNVAL;
	pollingInfo->FileDescriptors.push_back(pfd);
}

static void OS_RemoveSocketFromPolling(AsyncPollInfo* pollingInfo, int socketHandle) {
	auto fds = pollingInfo->FileDescriptors;
	for (auto it = fds.begin(); it != fds.end(); ++it) {
		if (it->fd == socketHandle) {
			fds.erase(it);
			break;
		}
	}
}


static int OS_PollData(AsyncSocket* server, AsyncPollInfo* info) {
	if (server == nullptr || info == nullptr) {
		return -1;
	}

	return WSAPoll(info->FileDescriptors.data(), (ULONG)info->FileDescriptors.size(), 1000);
}

#elif defined(UNIX)

#include <sys/epoll.h>
#include <unistd.h>

struct mnet::AsyncPollInfo
{
	int EPollFD;
	epoll_event Events[64];
	int PollCount;
};

void OS_InitPolling(AsyncPollInfo* info) {
	info->EPollFD = epoll_create1(0);
}

void OS_StopPolling(AsyncPollInfo* info) {
	close(info->EPollFD);
}

int OS_PollData(AsyncSocket* server, AsyncPollInfo* info) {
	if (server == nullptr || info == nullptr) {
		return -1;
	}

	info->PollCount = epoll_wait(info->EPollFD, info->Events, 64, 1000);
}

void OS_ProcessSocketData(AsyncSocket* server, AsyncPollInfo* info) {
	if (server == nullptr || info == nullptr) {
		return;
	}

	for (int i{ 0 }; i < info->PollCount; i++) {
		int fd = info->Events[i].data.fd;
		Socket* internalSocket = server->GetClient(fd);


	}
}

#else
#error NO_ARCHITECTURE_SET
#endif // _WIN32

AsyncSocket::AsyncSocket(SocketOptions options)
	: Socket(options)
{
	m_pPollingInfo = new AsyncPollInfo();
	OS_InitPolling(m_pPollingInfo);
}

AsyncSocket::~AsyncSocket()
{
	OS_StopPolling(m_pPollingInfo);
	delete m_pPollingInfo;
	m_internalThread.join();
}

void AsyncSocket::BeginListening()
{
	if (IsClient()) {
		m_internalThread = std::thread(&AsyncSocket::ListenThreadClient, this);
	}
	else if (IsServer()) {
		m_internalThread = std::thread(&AsyncSocket::ListenThreadServer, this);
	}
}

void AsyncSocket::OnClientConnected(Socket* sock)
{
	if (IsServer())
	{
		OS_AddSocketToPolling(m_pPollingInfo, sock->GetInternalSocketHandle());
	}
}

void AsyncSocket::OnClientDisconnected(Socket* sock)
{
	if (IsServer())
	{
		OS_RemoveSocketFromPolling(m_pPollingInfo, sock->GetInternalSocketHandle());
	}
}

void AsyncSocket::ListenThreadClient()
{
	// Client socket can receive from a single server
	while (IsValid()) {
		std::vector<char> data = Receive(RECEIVE_BUFFER_LEN);
		OnReceiveData(nullptr, data);
	}
}

void AsyncSocket::ListenThreadServer()
{
	std::vector<Socket*> socketsToDisconnect;
	std::vector<Socket*> socketsToConnect;

	// Add self for polling
	OS_AddSocketToPolling(m_pPollingInfo, GetInternalSocketHandle());
	while (IsValid()) {
		int result = OS_PollData(this, m_pPollingInfo);
		if (result <= 0)
		{
			// Nothing to do
			continue;
		}

		socketsToDisconnect.clear();
		socketsToConnect.clear();

#ifdef _WIN32
		for (auto& pfd : m_pPollingInfo->FileDescriptors) {
			if (pfd.revents & (POLLHUP | POLLERR)) {
				if (pfd.fd == GetInternalSocketHandle()) {
					// TODO :: Our socket is in error ? What to do?
					continue;
				}

				Socket* errSock = GetClient((int)pfd.fd);
				socketsToDisconnect.push_back(errSock);
				continue;
			}

			if (!(pfd.revents & POLLRDNORM)) {
				continue;
			}

			// SERVER SOCKET → accept
			if (pfd.fd == GetInternalSocketHandle()) {
				Socket* client = Accept();
				if (client != nullptr) {
					socketsToConnect.push_back(client);
				}
				continue;
			}

			Socket* sock = GetClient((int)pfd.fd);
			if (!sock)
			{
				continue;
			}


			auto bytes = sock->Receive();
			if (bytes.size() <= 0) {
				socketsToDisconnect.push_back(sock);
				continue;
			}

			OnReceiveData(sock, bytes);
		}
#endif // _WIN32


		for (auto sock : socketsToDisconnect) {
			DisconnectClient(sock);
		}

		for (auto sock : socketsToConnect) {
			InvokeClientConnected(sock);
		}
	}
}
