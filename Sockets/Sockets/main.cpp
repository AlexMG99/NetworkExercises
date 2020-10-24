#include "Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	const char* msg = "ping";
	char msg_recieved[MAX_BUFFER_SIZE];
	int res = 0;

	printf("Esto es el cliente UDP!\n");

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);

	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	while (true)
	{
		// Send message to server
		res = sendto(s, msg, sizeof(msg), 0, (sockaddr*)(&remoteAddr), sizeof(remoteAddr));

		// Recieve message from server
		int size = sizeof(bindAddr);
		res = recvfrom(s, msg_recieved, MAX_BUFFER_SIZE, 0, (sockaddr*)&bindAddr, &size);

		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message not recieved!");
		else
		{
			msg_recieved[res] = '\0';
			printf_s("%s\n", msg_recieved);
			Sleep(1000);
		}
	}

	closesocket(s);
	system("pause");
	return 0;
}



