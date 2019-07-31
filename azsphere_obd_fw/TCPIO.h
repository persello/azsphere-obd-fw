#pragma once

#define INTERFACE_NAME	"wlan0"
#define PORT_NUMBER		2400


/// <summary> The thread that manages the data I/O on the socket. Communication happens by using the two buffers. </summary>
void* TCPThread(void* _param);

/// <summary> Reads a string received from the socket until a \r\n (or \n) is found. </summary>
int readTCPString(char** _data);

/// <summary> Writes a string and adds a trailing \r\n if necessary. </summary>
int writeTCPString(char** _data);