#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <iostream>
#include "NiceException.h"

namespace nc
{
	void errorCheckSock(int err);
	void initWsa(WORD version);
	void cleanupWsa();
	std::string resolveDNS(const std::string& hostName);

	struct IPEndPoint
	{
		std::string ip;
		unsigned short port;
	};

	class NiceSocket
	{
	public:
		NiceSocket();
		NiceSocket(WORD version);
		~NiceSocket();
		void nConnect(const IPEndPoint& endPoint) const;
		void nAccept(unsigned short port);

		int nSend(const char* bufffer, int size) const;  //Sends the data. Returns the amount of bytes sent.
		int nSendS(const char* buffer, int size) const;  //Sends the data with the first 8 bytes containing the size of the rest of the message to be sent. Returns the amount of bytes read. Used in conjunction with nRecvS.

		int nRecv(char* buffer, int size) const;
		std::vector<char> nRecvS() const;  //Recieves the data with the first 8 bytes containing the size of the rest of the message to be sent. Used in conjunction with nSendS.

		void block();
		void unblock();

		void nDisconnect();

		SOCKET getInternal();
		SOCKET getInternalListening();

	private:
		void nListen(unsigned short port) const;
		void nBind(unsigned short) const;
		SOCKET nSocket;
		SOCKET nlisteningSocket;
	};
}
