#include "../Sockets/Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);

	closesocket(s);
	system("pause");
	return 0;
}