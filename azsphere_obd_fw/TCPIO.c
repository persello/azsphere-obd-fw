#include "TCPIO.h"
#include "buffer.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <errno.h>
#include <malloc.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <applibs/log.h>


char* wlan_addr;							// IP address of the selected interface
struct sockaddr_in ServerIp;				// The address struct
int sock = 0, client_conn = 0;				// File descriptors for the socket and the connection
char* init_data = "AZSPHEREOBDSCANNER $>";	// Initialization data that is sent first

/// <summary> Returns a string containing the current IP address of the interface specified in INTERFACE_NAME in TCPIO.h
/// <param name="addr"> The string in which the address will be written. </param>
/// </summary>
char* getWlanAddr(char* _interfaceName) {

	struct ifaddrs* addrs;
	getifaddrs(&addrs);
	struct ifaddrs* tmp = addrs;

	Log_Debug("TCPIO: The selected interface is %s.\n", INTERFACE_NAME);

	while (tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in* pAddr = (struct sockaddr_in*)tmp->ifa_addr;
			Log_Debug("TCPIO: IP address of interface %s is: %s.\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
			if (strncmp(tmp->ifa_name, _interfaceName, sizeof(tmp->ifa_name) - 1) == 0) {
				freeifaddrs(addrs);
				return inet_ntoa(pAddr->sin_addr);
			}
		}
		tmp = tmp->ifa_next;
	}
	Log_Debug("TCPIO: The specified interface was not found.\n");
	freeifaddrs(addrs);
	return "127.0.0.1";
}

int send_all(int _socket, void* _buffer)
{
	char* ptr = (char*)_buffer;
	size_t length = strlen((char*)_buffer);
	while (length > 0)
	{
		int i = send(_socket, ptr, length, NULL);
		if (i < 1) return -1;
		ptr += i;
		length -= i;
	}
	return 0;
}

int initSocket() {
	sock = socket(AF_INET, SOCK_STREAM, 0);		// Socket creation (AF_INET -> IPv4, SOCK_STREAM -> TCP)

	wlan_addr = getWlanAddr(INTERFACE_NAME);	// Getting wlan0's IP address

	memset(&ServerIp, '0', sizeof(ServerIp));	// Sets ServerIp to 0 (for its entire length)

	ServerIp.sin_family = AF_INET;						// IPv4
	ServerIp.sin_port = htons(PORT_NUMBER);				// Port defined in TCPIO.h
	ServerIp.sin_addr.s_addr = inet_addr(wlan_addr);	// Address is set to wlan0's address

	// Try to bind sock and ServerIp
	if (bind(sock, (struct sockaddr*) & ServerIp, sizeof(ServerIp)) == -1) {
		Log_Debug("TCPIO: Socket binding failed.\n\tERROR: \"%s\".\n", strerror(errno));
		return -1;
	}

	// Socket is set to passive mode (listening)
	if (listen(sock, 20) == -1) {
		Log_Debug("TCPIO: Error. Server not started.\n\tERROR: \"%s\".\n", strerror(errno));
		return -1;
	}
	else {
		Log_Debug("TCPIO: TCP Server started at address %s on port %d.\n", inet_ntoa(ServerIp.sin_addr), ntohs(ServerIp.sin_port));
	}

	// When a connection happens (blocking), we populate the client_conn file descriptor
	client_conn = accept(sock, (struct sockaddr*)NULL, NULL);
	Log_Debug("TCPIO: A request received from client.\n");
	Log_Debug("TCPIO: Answering with the initialization data: %s.\n", init_data);
	return send_all(client_conn, init_data);
}

int receiveOne(char* _data) {
	int result = recv(client_conn, &_data, 1, NULL);
	return result;
}

int endSocket() {
	close(client_conn);
}

int sendData(char* _data) {
	send_all(client_conn, _data);
}

void initCircBuffer(buffer_t* _buf, int _size) {
	_buf->head = 0;
	_buf->tail = 0;
	_buf->size = _size;
	_buf->content = (char*)malloc(_size * sizeof(char));
	_buf->status = BUFFER_EMPTY;
}


///////////// PUBLIC /////////////

int readTCPString(char** data) {}

int writeTCPString(char** data) {}

void* TCPThread(void* _param) {
	// Try starting the socket, exits immediately on fail. Waits until socket is open (blocking).
	int exit = initSocket();

	// Buffers declaration
	buffer_t inputBuffer;
	buffer_t outputBuffer;

	initCircBuffer(&inputBuffer, 1024);
	initCircBuffer(&outputBuffer, 1024);

	while (!exit) {

		char rec;

		int receiveResult = receiveOne(&rec);		// Receive a character
		if (receiveResult == 0) {					// When the connection closes
			exit = 1;
			Log_Debug("TCPIO: Socket closed gracefully. Exiting now.\n");
		}
		else if (receiveResult == -1) {
			exit = 1;
			Log_Debug("TCPIO: Socket error. Exiting now. Details on next log.\n");
		}

	}

	close(client_conn);
	Log_Debug("TCPIO: TCP thread closed. Last error was \"%s\".\n", strerror(errno));
}