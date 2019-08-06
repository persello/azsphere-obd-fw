#include "commandinterpreter.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define WIFICONFIG_STRUCTS_VERSION 1

#include <applibs/log.h>
#include <applibs/wificonfig.h>


typedef struct {
	char header[5];
	char arguments[101];
} command_t;


int (*receive)(char**);
int (*send)(char*);
int halt = 0;


int isInOOBE;
int waitingForBtnPress;

void* commandInterpreterThread(void* _param) {

	Log_Debug("COMMANDINT: Command interpreter thread started.\n");

	while (!halt) {

		command_t currentCommand;
		memset(currentCommand.header, 0, 5);
		memset(currentCommand.arguments, 0, 101);

		// The result from the receive function (-1 is incomplete, 0 is complete).
		int result = -1;

		// 1024 because could be and encrypted or non-standard command.
		char buf[1024];

		// buf is now an empty string.
		memset(buf, 0, 1024);

		Log_Debug("COMMANDINT: Ready to receive a new command.\n");

		long i;

		// First we receive an entire command. Blocks until a command is received.
		do {

			// Set to a fixed size because of segfaults when using free()
			char recv[1024];
			memset(recv, 0, 1024);

			result = (*receive)(&recv);

			if (strlen(recv) > 0) {

				// Concatenate the last data with the previous part.
				strcat(buf, recv);

				Log_Debug("COMMANDINT: New data received (%s), now the buffer is \"%s\".\n", recv, buf);
			}

			// Loop until command is complete.
		} while (result == -1);

		Log_Debug("COMMANDINT: The command is complete. The content of the buffer is \"%s\".\n", buf);

		// TODO: Decrypt encrypted commands (start with *).

		// TODO: Filter and process special commands.

		Log_Debug("COMMANDINT: Processing command into a command_t.\n");

		// TODO: strlen > 104

		// Then we decode standard, non-encrypted commands (4HEAD + 100ARG max.).
		for (int i = 0; i < strlen(buf); i++) {
			if (i < 4) {
				currentCommand.header[i] = buf[i];
			}
			else {
				currentCommand.arguments[i - 4] = buf[i];
			}
		}

		Log_Debug("COMMANDINT: Decoding last command. Header is \"%s\", arguments are \"%s\".\n", currentCommand.header, currentCommand.arguments);

		char* answer;
		int answered = 0;

		// Process the current command before the answer.

		// Ping request
		if (!strcmp(currentCommand.header, "PING")) {
			Log_Debug("COMMANDINT: Command decoded as ping request.\n");
			answer = malloc(5 * sizeof(char));
			memset(answer, 0, 5 * sizeof(char));
			strcat(answer, "PING");
			answered = 1;

			// TODO: Add ping timer control

		}
		// OOBE Request
		else if (!strcmp(currentCommand.header, "OOBE")) {
			Log_Debug("COMMANDINT: Command decoded as OOBE request.\n");

			// Test variable
			char OOBEAllowed = 1;
			if (OOBEAllowed) {
				Log_Debug("COMMANDINT: OOBE allowed, entering configuration mode.\n");
				isInOOBE = 1;
				answer = malloc(5 * sizeof(char));
				memset(answer, 0, 5 * sizeof(char));
				strcat(answer, "OOBE");
				answered = 1;
			}
			else {
				Log_Debug("COMMANDINT: OOBE denied. Device is already configured.\n");
				answer = malloc(5 * sizeof(char));
				memset(answer, 0, 5 * sizeof(char));
				strcat(answer, "ODEN");
				answered = 1;
			}
		}
		// Get Wi-Fi scanned network list
		else if (!strcmp(currentCommand.header, "WISC")) {

			Log_Debug("COMMANDINT: Command decoded as Wi-Fi scan request.\n");
			int result;

			// We get the number of networks.
			result = WifiConfig_TriggerScanAndGetScannedNetworkCount();

			// If there is an error...
			if (result == -1) {
				Log_Debug("COMMANDINT: Wi-Fi scan error: \"%s\".\n", strerror(errno));
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcat(answer, "WISCE");
				answered = 1;
			}
			// If there are no networks...
			else if (result == 0) {
				Log_Debug("COMMANDINT: Wi-Fi scan found no networks.\n");
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcat(answer, "WISCN");
				answered = 1;
			}
			// If there are networks...
			else {

				// Array of available networks
				WifiConfig_ScannedNetwork networks[result];
				WifiConfig_GetScannedNetworks(&networks, result);
				
				// 6 = WISCF + \n, result - 1 for separators (#)
				int length = 6 + (result - 1);

				// Then we add to length the length of every SSID
				for (int i = 0; i < result; i++) {
					length += networks[i].ssidLength;
				}

				// Allocating space for the answer
				answer = (char*) malloc(length * sizeof(char));
				memset(answer, 0, length * sizeof(char));
				strcat(answer, "WISCF");

				// Adds the scanned SSIDs to the answer
				for (int i = 0; i < result; i++) {
					strncat(answer, networks[i].ssid, networks[i].ssidLength);
					answer[strlen(answer)] = '\0';

					// Except the last time, we use a # as a separator
					if (i != result - 1) {
						strcat(answer, "#");
					}
				}

				answered = 1;
			}
		}

		// Answer with the requested data.
		// TODO: check results

		// This filters errors when receiving invalid commands.
		if (answered) {
			Log_Debug("COMMANDINT: Sending answer. Content is \"%s\".\n", answer);
			(*send)(answer);
			free(answer);
		}
		else {
			Log_Debug("COMMANDINT: Invalid command. No answer.\n");
		}
	}

	// Thread is stopping
	Log_Debug("COMMANDINT: The command interpreter thread is stopping %s.\n", halt ? "gracefully" : "because of an internal error");

	// If it exited because of a break condition, print the possible cause.
	if (!halt) {
		Log_Debug("COMMANDINT: This could be because of the following error: \"%s\". Check the previous log items for other kinds of errors.\n", strerror(errno));
	}

	// Thread is stopped. This allows the stop function to know it is really stopped.
	halt = 2;
}


pthread_t ciThread;

int startCommandInterpreter(int (*_receive)(char**), int (*_send)(char*)) {

	// Allows the thread to continue.
	halt = 0;

	Log_Debug("COMMANDINT: Starting command interpreter thread.\n");

	// Associate read and write functions (generic in order to allow different connections, not only TCP).
	receive = _receive;
	send = _send;

	// Start the thread.
	pthread_create(&ciThread, NULL, commandInterpreterThread, NULL);
}


int stopCommandInterpreter()
{
	Log_Debug("COMMANDINT: Stopping command interpreter thread.\n");

	// Exits the main loop.
	halt = 1;

	// 2 ms
	struct timespec ts = { 0, 2000000 };
	nanosleep(&ts, NULL);

	// Check if it is really stopped.
	if (halt == 2) {
		return 0;
	}

	// If not, return -1.
	return -1;
}