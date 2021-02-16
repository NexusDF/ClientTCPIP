// ClientTCP.cpp : Defines the entry point for the console application.
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET Connection;
int result;

char* CreateDynamicString(int size) {
	char* str = new char[size + 1];
	str[size] = '\0';
	return str;
}

void ClientHandler() {
	int msg_size;
	while (true) {
		result = recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		if (result <= 0) break;

		char* msg = CreateDynamicString(msg_size);
		result = recv(Connection, msg, msg_size, NULL);
		if (result <= 0) break;

		std::cout << msg << std::endl;
		delete[] msg;
	}
	printf("Lost connection\n");
	shutdown(Connection, SD_SEND);
}

int __cdecl main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	char msg[256];
	std::string name;
	int msg_size;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Attempt to connect to an address until one succeeds
		Connection = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connect(Connection, ptr->ai_addr, (int)ptr->ai_addrlen) != 0) {
			printf("Error: failed connect to server.\n");
			return 1;
		}
		recv(Connection, msg, sizeof(msg), NULL);
		printf(msg);
		std::getline(std::cin, name);
		int name_size = name.size();
		send(Connection, (char*)&name_size, sizeof(int), NULL);
		send(Connection, name.c_str(), name_size, NULL);
		

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

		std::string msg;
		while (true) {
			std::getline(std::cin, msg);
			int msg_size = msg.size();
			send(Connection, (char*)&msg_size, sizeof(int), NULL);
			send(Connection, msg.c_str(), msg_size, NULL);
			Sleep(10);
		}
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

