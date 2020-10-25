#include "../Sockets/Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int res = 0;
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	const char* msg = "pong";
	char msg_recieved[MAX_BUFFER_SIZE];

	printf("Esto es el server UDP!\n");

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	res = bind(s, (const struct sockaddr*) & bindAddr, sizeof(bindAddr));

	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Socket error! Not binded correctly");
	}

	while (true)
	{
		// Recieve message from client
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

		// Send message to client
		res = sendto(s, msg, sizeof(msg), 0, (sockaddr*)(&bindAddr), sizeof(bindAddr));
		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message don't send!");
	}


	closesocket(s);
	system("pause");
	return 0;
}