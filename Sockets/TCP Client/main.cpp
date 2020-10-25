#include "../Sockets/Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	const char* msg = "Naruutoooooooo";
	char msg_recieved[MAX_BUFFER_SIZE];
	int res = 0;

	printf("Esto es el cliente TCP!\n");

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);

	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	int len = sizeof(bindAddr);
	res = connect(s, (sockaddr*)&remoteAddr, len);

	while (true)
	{
		// Send message to server
		res = send(s, msg, sizeof(msg), 0);

		// Recieve message from server
		res = recv(s, msg_recieved, MAX_BUFFER_SIZE, 0);

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