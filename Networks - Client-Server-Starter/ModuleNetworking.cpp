#include "Networks.h"
#include "ModuleNetworking.h"
#include <list>


static uint8 NumModulesUsingWinsock = 0;

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).

	// New socket set
	fd_set readfds;
	FD_ZERO(&readfds);

	// Fill the set
	for (auto s : sockets) {
		FD_SET(s, &readfds);
	}

	// Timeout (return immediately)
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Select (check for readability)
	int res = select(0, &readfds, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR) {
		reportError("select 4 read");
	}

	// Fill this array with disconnected sockets
	std::list<SOCKET> disconnectedSockets;

	// Read selected sockets
	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds)) {
			if (isListenSocket(s)) { // Is the server socket
				sockaddr_in bindAddr;
				//bindAddr.sin_family = AF_INET;
				//bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;
				
				// Accept stuff
				int len = sizeof(bindAddr);
				SOCKET s1 = accept(s, (sockaddr*)&bindAddr, &len);
				if (s1 != SOCKET_ERROR)
				{
					onSocketConnected(s1, bindAddr);

					// Send message of connecting
					OutputMemoryStream message;
					message << ClientMessage::Welcome;
					message << "*****************************\n WELCOME TO THE CHAT\n Please type /help to see the aviable commands.\n *****************************";
					message << MessageType::Info;

					sendPacket(message, s1);

					addSocket(s1);
				}
				else
					reportError("Server Socket not accepted");
			}
			else { // Is a client socket
				// Recv stuff / Accept stuff
				InputMemoryStream packet;
				int bytesRead = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0);
				if (bytesRead > 0)
				{
					packet.SetSize((uint32)bytesRead);
					onSocketReceivedData(s, packet);
				}
				else
				{
					onSocketDisconnected(s);
					disconnectedSockets.push_back(s);
					reportError("Client Socket not accepted");
				}

				/*int result = recv(s, (char*)incomingDataBuffer, incomingDataBufferSize, 0);
				if (result == SOCKET_ERROR || result == 0)
				{
					
				}
				else
					onSocketReceivedData(s, incomingDataBuffer);*/
				
			}
		}
	}

	for (auto it_sock = disconnectedSockets.begin(); it_sock != disconnectedSockets.end(); ++it_sock)
	{
		auto it_erase = std::find(sockets.begin(), sockets.end(), (*it_sock));
		if (it_erase != sockets.end())
		{
			sockets.erase(it_erase);
			break;
		}
	}


	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}


void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR)
	{
		reportError("Error sending package");
		return false;
	}
	return true;
}
