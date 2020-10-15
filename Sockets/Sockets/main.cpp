#include "Sockets.h"

int main(int argc, char** argv)
{
	InitSockets();
	int port = 8000;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);

	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	const char* remoteAddrStr = "192.168.0.1";
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	closesocket(s);
	system("pause");
	return 0;
}



