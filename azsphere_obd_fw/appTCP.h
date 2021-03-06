#pragma once

#include "config.h"


/// <summary> Starts the socket, waits for an incoming connection and then starts sending and
/// receiving. Threaded, non-blocking. </summary>
void startTCPThreads(void);

/// <summary> Stops the TCP send/receive threads and closes the socket. </summary>
void stopTCPThreads(void);

typedef enum {
	STATUS_UNKNOWN,
	STATUS_STARTING,
	STATUS_RUNNING,
	STATUS_STOPPING,
	STATUS_STOPPED
}TCPSocketThreadStatus;

/// Variable shared between receive and send threads.
TCPSocketThreadStatus TCPThreadStatus;

/// <summary> Reads the content of the input buffer until it finds a /r/n (and removes it). </summary>
/// <param name="_data"> The string in which the data will be put into from the input buffer. </param>
/// <returns> 0 if successful, -1 if the returned command is incomplete. </returns>
int readTCPString(char** _data);

/// <summary> Writes a string to the output buffer and adds a trailing \r\n if necessary. </summary>
/// <param name="_data"> The string that will be put into the output buffer. </param>
/// <returns> 0 if successful, -1 if the buffer was full. </returns>
int writeTCPString(char* _data);

/// <summary> Writes a single byte in the output buffer. </summary>
/// <param name="_data"> The character that will be put into the output buffer. </param>
/// <returns> 0 if successful, -1 if the buffer was full. </returns>
int writeTCPChar(char _data);