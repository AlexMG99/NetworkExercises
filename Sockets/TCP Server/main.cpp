#include "../Sockets/Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int res = 0;
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	const char* msg = "Sasuukeeeeeeee";
	char msg_recieved[MAX_BUFFER_SIZE];

	printf("Esto es el server TCP!\n");

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	res = bind(s, (const struct sockaddr*) & bindAddr, sizeof(bindAddr));

	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Socket error! Not binded correctly");
	}

	res = listen(s, 1);

	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Socket error! Not listened correctly");
	}

	int len = sizeof(bindAddr);
	SOCKET s1 = accept(s, (sockaddr*)&bindAddr, &len);

	while (true)
	{
		// Recieve message from client
		res = recv(s1, msg_recieved, MAX_BUFFER_SIZE, 0);

		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message not recieved!");
		else
		{
			msg_recieved[res] = '\0';
			printf_s("%s\n", msg_recieved);
			Sleep(1000);
		}

		// Send message to client
		res = send(s1, msg, strlen(msg), 0);
		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message don't send!");
	}


	closesocket(s);
	closesocket(s1);
	system("pause");
	return 0;
}