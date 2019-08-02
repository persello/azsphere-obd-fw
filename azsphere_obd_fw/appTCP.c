#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <applibs/log.h>

#include "appTCP.h"
#include "buffer.h"
#include "config.h"



char* wlan_addr;							// IP address of the selected interface
struct sockaddr_in ServerIp;				// The address struct
int sock = 0, client_conn = 0;				// File descriptors for the socket and the connection
char* init_data = "Azure Sphere OBD Scanner. FW v" FW_VER ", built " __DATE__ " " __TIME__ ".";

pthread_t sendThread;
pthread_t receiveThread;

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

	return 0;
}

int receiveOne(char* _data) {
	int result = recv(client_conn, _data, 1, NULL);
	return result;
}

int endSocket() {
	close(client_conn);
}

buffer_t inputBuffer;
buffer_t outputBuffer;

int TCPexit;		// Variable shared between receive and send threads, in case closing the socket is not enough.

///////////// PUBLIC /////////////

int readTCPString(char** _data) {
	char workingbuf[1024];

	char c;
	int i = 0;
	int result = 0;

	do {
		if (getCharBuffer(&inputBuffer, &c) == -1) {
			// If the returned command is incomplete exit from the loop, but let the caller know.
			result = -1;
			break;
		}

		if (!(c == NULL || c == '\r' || c == '\n')) {
			workingbuf[i] = c;
			i++;
		}

		// When we end with a \n the command is complete, so we return 0.
	} while (c != '\n');

	// Add a null terminator for strcpy
	workingbuf[i] = '\0';

	// Allocate only what needed
	*_data = malloc(i + 1);
	strcpy(*_data, workingbuf);
	return result;
}

int writeTCPString(char* _data) {

	int length = strlen(_data);

	// This way we don't even put a \r\n on empty strings
	if (length) {

		// In case we need to add \r, \n and null terminator
		char s[length + 3];
		if (strlen(_data) >= 2 && !strcmp(_data + strlen(_data) - 2, "\r\n")) {
			// Already ok
			strcpy(s, _data);
		}
		else {
			// Need to add new line
			strcpy(s, _data);
			strcat(s, "\r\n");
		}

		// Pushes all the characters to the output buffer
		for (int i = 0; i < strlen(s); i++) {

			// Exit on buffer full
			if (putCharBuffer(&outputBuffer, s[i]) == -1) {
				return -1;
			}
		}
	}

	return 0;
}

void* TCPSendThread(void* _param) {

	while (!TCPexit) {

		// Skip if empty
		if (outputBuffer.status != BUFFER_EMPTY) {

			// String for output
			char snd[1024];
			int i;

			// Load all the contents of the buffer into the output string
			while (outputBuffer.status != BUFFER_EMPTY) {
				getCharBuffer(&outputBuffer, &snd[i++]);
			}

			// Send everything (blocks until everything is sent)
			send_all(client_conn, &snd);
		}
	}
}

void* TCPReceiveThread(void* _param) {

	// Try starting the socket, exits immediately on fail. Waits until socket is open (blocking).
	TCPexit = initSocket();

	// When (and if) the socket is initialized, we also start sending data.
	if (!TCPexit) {
		pthread_create(&sendThread, NULL, TCPSendThread, NULL);
	}

	while (!TCPexit) {

		// Receive
		char rec;
		int receiveResult = receiveOne(&rec);		// Receive a character (blocking)
		if (receiveResult == 0) {					// When the connection closes
			TCPexit = 1;
			Log_Debug("TCPIO: Socket closed gracefully. Exiting now.\n");
		}
		else if (receiveResult == -1) {				// When there is an error
			TCPexit = 1;
			Log_Debug("TCPIO: Socket error. Exiting now. Details: \"%s\"\n", strerror(errno));
		}
		else {										// When correctly received
			if (putCharBuffer(&inputBuffer, rec) == -1) {	// Push to buffer, if full...
				TCPexit = 1;
				Log_Debug("TCPIO: Input buffer was full. Exiting now.");
			}
		}
	}

	close(client_conn);
	Log_Debug("TCPIO: TCP thread closed. Last error was \"%s\".\n", strerror(errno));
}

void startTCPThreads() {

	// Buffer initialization
	initCircBuffer(&inputBuffer, 1024);
	initCircBuffer(&outputBuffer, 1024);

	// We load some initialization data into the output buffer (FW version and other)
	writeTCPString(init_data);

	/* We start the receive thread, it will start the send one if the socket is ready.
	Since listening to a new connection is a blocking action (and can't be made
	non-blocking on this platform due to the unavailability of fcntl), we choose to
	let the receiveThread manage the socket's opening and the send thread's creation. */
	pthread_create(&receiveThread, NULL, TCPReceiveThread, NULL);
}