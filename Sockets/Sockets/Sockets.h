#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define MAX_BUFFER_SIZE 1024

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

void printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	system("pause");
	exit(-1);
}

void InitSockets()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printWSErrorAndExit("Initializated not correctly");	
}

void CleanUp()
{
	int iResult = WSACleanup();
	system("pause");
}