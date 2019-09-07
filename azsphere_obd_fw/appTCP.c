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
#include "lib/circularbuffer/buffer.h"
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

	Log_Debug("TCPIO: Getting IP address.\n");

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
	return "";
}

int send_all(int _socket, char* _buffer)
{
	size_t length = strlen(_buffer);
	int i = write(_socket, _buffer, length);
	return i;
}

int initSocket(void) {

	Log_Debug("TCPIO: TCP socket initialization started.\n");

	sock = socket(AF_INET, SOCK_STREAM, 0);		// Socket creation (AF_INET -> IPv4, SOCK_STREAM -> TCP)

	wlan_addr = getWlanAddr(INTERFACE_NAME);	// Getting wlan0's IP address

	// When interface is not available, fail.
	if (!strlen(wlan_addr)) {
		return -1;
	}

	memset(&ServerIp, '0', sizeof(ServerIp));	// Sets ServerIp to 0 (for its entire length)

	ServerIp.sin_family = AF_INET;						// IPv4
	ServerIp.sin_port = htons(PORT_NUMBER);				// Port defined in TCPIO.h
	ServerIp.sin_addr.s_addr = inet_addr(wlan_addr);	// Address is set to wlan0's address

	// Reuse address (because it might still be used by the previous instance if the app crashed).
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int));

	// Try to bind sock and ServerIp
	if (bind(sock, (struct sockaddr*) & ServerIp, sizeof(ServerIp)) == -1) {
		Log_Debug("TCPIO: Socket binding failed. ERROR: \"%s\".\n", strerror(errno));
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
	int result = recv(client_conn, _data, 1, 0);
	return result;
}

buffer_t inputBuffer;
buffer_t outputBuffer;

///////////// PUBLIC /////////////

int readTCPString(char* _data[]) {

	// A temporary buffer, _data will have the correct size.
	char* workingbuf = malloc(1024 * sizeof(char));

	char c;
	int i = 0;
	int result = 0;

	do {
		if (getCharBuffer(&inputBuffer, &c) == -1) {
			// If the returned command is incomplete exit from the loop, but let the caller know.
			result = -1;
			break;
		}

		if (!(NULL == c || c == '\r' || c == '\n')) {
			workingbuf[i] = c;
			i++;
		}

		// When we end with a \n the command is complete, so we return 0.
	} while (c != '\n');

	// Add a null terminator for strcpy
	workingbuf[i] = '\0';

	// Allocate only what needed
	strcpy(_data, workingbuf);
	free(workingbuf);
	return result;
}

// For file transmission only
int writeTCPChar(char _data) {
	if (_data == '\r') {
		putCharBuffer(&outputBuffer, _data);
		_data = '\n';
	}

	return (putCharBuffer(&outputBuffer, _data));
}

int writeTCPString(char* _data) {

	size_t length = strlen(_data);

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

	Log_Debug("TCPIO: TCP send thread started.\n");

	while (TCPThreadStatus == STATUS_RUNNING) {

		// Skip if empty
		if (outputBuffer.status != BUFFER_EMPTY) {

			// Buffer for calculating minimum size.
			char buf[outputBuffer.size];
			int i = 0;

			memset(buf, 0, outputBuffer.size);

			// Load all the contents of the buffer into the output string.
			while (outputBuffer.status != BUFFER_EMPTY && i < outputBuffer.size) {
				getCharBuffer(&outputBuffer, &buf[i++]);
			}

			char snd[strlen(buf) + 1];
			snd[0] = '\0';
			strcpy(snd, buf);
			// Send everything (blocks until everything is sent).
			send_all(client_conn, snd);
		}
	}

	shutdown(sock, 2);
	close(client_conn);
	close(sock);
	pthread_cancel(receiveThread);
	sleep(1);
	Log_Debug("TCPIO: TCP send thread closed. Last error was \"%s\".\n", strerror(errno));
	TCPThreadStatus = STATUS_STOPPED;

	return NULL;
}

void* TCPReceiveThread(void* _param) {

	// Allow the thread to be halted while blocking
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	Log_Debug("TCPIO: TCP receive thread started.\n");

	// Try starting the socket, exits immediately on fail. Waits until socket is open (blocking).
	if (initSocket() == -1) {
		TCPThreadStatus = STATUS_STOPPING;
	}
	else {
		// Allow to run even if previously stopped. Setting this to 0 means the thread is running.
		TCPThreadStatus = STATUS_RUNNING;
	}

	// When (and if) the socket is initialized, we also start sending data.
	if (TCPThreadStatus == STATUS_RUNNING) {
		pthread_create(&sendThread, NULL, TCPSendThread, NULL);
	}

	while (TCPThreadStatus == STATUS_RUNNING) {

		// Receive
		char rec;
		int receiveResult = receiveOne(&rec);		// Receive a character (blocking)
		if (receiveResult == 0) {					// When the connection closes
			TCPThreadStatus = STATUS_STOPPING;
			Log_Debug("TCPIO: Socket closed gracefully. Exiting now.\n");
		}
		else if (receiveResult == -1) {				// When there is an error
			TCPThreadStatus = STATUS_STOPPING;
			Log_Debug("TCPIO: Socket error. Exiting now. Details: \"%s\"\n", strerror(errno));
		}
		else {										// When correctly received
			if (putCharBuffer(&inputBuffer, rec) == -1) {	// Push to buffer, if full...
				TCPThreadStatus = STATUS_STOPPING;
				Log_Debug("TCPIO: Input buffer was full. Exiting now.");
			}
		}
	}

	shutdown(sock, 2);
	close(client_conn);
	close(sock);
	Log_Debug("TCPIO: TCP receive thread closed. Last error was \"%s\".\n", strerror(errno));
	TCPThreadStatus = STATUS_STOPPED;
	return NULL;
}

void startTCPThreads() {

	Log_Debug("TCPIO: Starting threads.\n");

	TCPThreadStatus = STATUS_STARTING;

	// Buffer initialization.
	if (!inputBuffer.size)
		initCircBuffer(&inputBuffer, 1024);

	if (!outputBuffer.size)
		initCircBuffer(&outputBuffer, 20000);	// Big for fast file output

	// We load some initialization data into the output buffer (FW version and other).
	writeTCPString(init_data);

	/* We start the receive thread, it will start sending buffer data when the socket
	is ready. Since listening to a new connection is a blocking action (and can't be
	made non-blocking on this platform due to the unavailability of fcntl), we choose
	to let the receiveThread manage the socket's opening and the send thread's creation. */
	pthread_create(&receiveThread, NULL, TCPReceiveThread, NULL);
}

void stopTCPThreads() {
	TCPThreadStatus = STATUS_STOPPING;
}