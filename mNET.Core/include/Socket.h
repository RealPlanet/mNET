#pragma once

#ifndef _H_SOCKET_
#define _H_SOCKET_

#include "mnet.h"

#include <vector>
#include <map>
#include <string>

namespace mnet {

	struct SocketImpl;

	enum class SocketProtocol
	{
		Unknown,
		TCP,
		UDP,
	};

	enum class AddressFamily
	{
		Unknown,
		IPV4,
		IPV6,
	};

	struct KeepAliveConfig
	{
		int IdleTime;
		int ProbeInterval;
		int ProbeCount;
	};

	struct SocketOptions
	{
		AddressFamily Family;
		SocketProtocol Protocol;
		std::string Address;
		int Port;
		bool EnableKeepAlive;
		KeepAliveConfig KeepAliveCfg;
	};

	class Socket
	{
	private:
		Socket(SocketImpl* source);

	public:
		DLL_API Socket(SocketOptions options);
		DLL_API virtual ~Socket();

		DLL_API int OpenServer();
		DLL_API int OpenClient();

		bool IsValid() const;
		bool IsServer() const { return m_bIsServer; }
		bool IsClient() const { return m_bIsClient; }

		DLL_API Socket* Accept();
		DLL_API bool DisconnectClient(Socket* sock);
		DLL_API void Disconnect();
		DLL_API Socket* GetClient(int handle);
		DLL_API void Send(std::vector<char> data);
		DLL_API void Send(const std::string& data);
		DLL_API void SendToClient(std::vector<char> data, Socket* client = nullptr);
		DLL_API std::vector<char> Receive(size_t bufferLen = 512);
		DLL_API int GetPort();

		DLL_API virtual void OnClientConnected(Socket* sock) { (void*)sock; }
		DLL_API virtual void OnClientDisconnected(Socket* sock) { (void*)sock; }

		DLL_API long GetInternalSocketHandle();
	protected:
		DLL_API SocketImpl* GetInternalSocket() { return m_pSocket; }

		DLL_API void InvokeClientConnected(Socket* sock);
		DLL_API void InvokeClientDisconnected(Socket* sock);

	private:
		SocketImpl* m_pSocket;
		std::map<unsigned long, Socket*> m_mClients{};
		bool m_bIsClient{ false };
		bool m_bIsServer{ false };
	};
}

#endif // !_H_SOCKET_

