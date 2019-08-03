#include "commandinterpreter.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <applibs/log.h>


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

	Log_Debug("COMMANDINT: This is the command interpreter thread.\n");

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
		if (!strcmp(currentCommand.header, "PING")) {
			Log_Debug("COMMANDINT: Command decoded as ping request.\n");
			answer = malloc(5 * sizeof(char));
			answer = strdup("PING");
			answered = 1;
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