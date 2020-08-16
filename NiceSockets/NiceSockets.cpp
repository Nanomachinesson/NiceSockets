#include "NiceSockets.h"

namespace nc
{
	void errorCheckSock(int err)
	{
		if (err == SOCKET_ERROR) {
			std::stringstream msg;
			int extErr = WSAGetLastError();
			msg << "SOCKET_ERROR, WSAGetLastError: " << extErr;
			throw NiceException(msg);
		}
	}

	void errorCheckSock(SOCKET s)
	{
		if (s == INVALID_SOCKET) {
			std::stringstream msg;
			int extErr = WSAGetLastError();
			msg << "INVALID_SOCKET, WSAGetLastError: " << extErr;
			throw NiceException(msg);
		}
	}

	void initWsa(WORD version)
	{
		WSAData wsaData;
		std::stringstream msg;
		int err = WSAStartup(version, &wsaData);

		if (err != 0) {
			msg << "WSAStartup: " << err;
			throw NiceException(msg);
		}
	}

	void cleanupWsa()
	{
		std::stringstream msg;
		int err = WSACleanup();

		if (err != 0) {
			msg << "WSACleanup: " << err;
			throw NiceException(msg);
		}
	}

	std::string resolveDNS(const std::string& hostName)
	{
		initWsa(MAKEWORD(2, 2));

		addrinfo info{};
		addrinfo* result{};

		info.ai_family = AF_INET;
		info.ai_socktype = SOCK_STREAM;
		info.ai_protocol = IPPROTO_TCP;

		getaddrinfo(hostName.c_str(), NULL, &info, &result);

		char buf[1024];
		for (; result != NULL; result = result->ai_next) {  //epic for loop
			getnameinfo(result->ai_addr, result->ai_addrlen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
		}

		freeaddrinfo(result);
		return std::string(buf);  //just return the last one lmao

		cleanupWsa();
	}

	NiceSocket::NiceSocket() :
		nSocket(NULL),
		nlisteningSocket(NULL)
	{
		initWsa(MAKEWORD(2, 2));
		nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	NiceSocket::NiceSocket(WORD version) :
		nSocket(NULL),
		nlisteningSocket(NULL)
	{
		initWsa(version);
		nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	NiceSocket::~NiceSocket()
	{
		try {
			WSACleanup();
		}
		catch (NiceException ex) {
			std::cerr << "NiceSocket destructor failed WSACleanup" << std::endl;
		}
	}

	void NiceSocket::nConnect(const IPEndPoint& endPoint) const
	{
		sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET;
		InetPtonA(AF_INET, endPoint.ip.c_str(), &sockaddr.sin_addr.s_addr);
		sockaddr.sin_port = htons(endPoint.port);
		errorCheckSock(connect(nSocket, reinterpret_cast<SOCKADDR*>(&sockaddr), sizeof(sockaddr)));
	}

	void NiceSocket::nAccept(unsigned short port)
	{
		nListen(port);
		SOCKET acceptSocket = accept(nSocket, NULL, NULL);
		errorCheckSock(acceptSocket);
		closesocket(nSocket);
		nlisteningSocket = acceptSocket;
		nSocket = acceptSocket;
	}

	int NiceSocket::nSend(const char* buffer, int size) const
	{
		int offset = 0;

		do {
			int ret = send(nSocket, buffer + offset, size, 0);
			
			errorCheckSock(ret);

			offset += ret;

			if (offset == size || ret == 0) {
				break;
			}
		} while (true);

		return offset;
	}

	int NiceSocket::nRecv(char* buffer, int size) const
	{
		int offset = 0;

		do {
			int ret = recv(nSocket, buffer + offset, size, 0);  //MSG_WAITALL?
			
			errorCheckSock(ret);

			offset += ret;

			if (offset == size || ret == 0) {
				break;
			}
		} while (true);

		return offset;
	}

	SOCKET NiceSocket::getInternal()
	{
		return nSocket;
	}

	SOCKET NiceSocket::getInternalListening()
	{
		return nlisteningSocket;
	}

	int NiceSocket::nSendS(const char* buffer, int size) const
	{
		std::int32_t actualSize = static_cast<int32_t>(size);  //muh int not guaranteed to be 4 bytes
		nSend(reinterpret_cast<char*>(&actualSize), sizeof(actualSize));  //first 4 bytes = package size
		int sent = nSend(buffer, size);
		return sent;
	}

	std::vector<char> NiceSocket::nRecvS() const
	{
		std::int32_t size;
		nRecv(reinterpret_cast<char*>(&size), sizeof(size));  //first 4 bytes = package size

		std::vector<char> buf(size);
		nRecv(buf.data(), size);

		return buf;
	}

	void NiceSocket::block()
	{
		u_long arg = 0;
		errorCheckSock(ioctlsocket(nSocket, FIONBIO, &arg));
	}

	void NiceSocket::unblock()
	{
		u_long arg = 1;
		errorCheckSock(ioctlsocket(nSocket, FIONBIO, &arg));
	}

	void NiceSocket::nDisconnect()
	{
		errorCheckSock(closesocket(nSocket));
	}

	void NiceSocket::nListen(unsigned short port) const
	{
		nBind(port);
		errorCheckSock(listen(nSocket, SOMAXCONN));
	}

	void NiceSocket::nBind(unsigned short port) const
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		InetPtonA(AF_INET, "127.0.0.1", &service.sin_addr.s_addr);
		service.sin_port = htons(port);

		errorCheckSock(bind(nSocket, reinterpret_cast<sockaddr*>(&service), sizeof(service)));
	}
}
