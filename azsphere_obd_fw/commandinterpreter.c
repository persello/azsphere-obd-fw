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

#include "obdserial.h"
#include "cardmanager.h"
#include "lib/gpslib/gps.h"
#include "lib/timer/Inc/Public/timer.h"


typedef struct {
	char header[5];
	char arguments[101];
} command_t;


int (*cirecv)(char**);
int (*cisend)(char*);
int (*sendChar)(char);
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

		// 1024 because could be a non-standard command.
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

			result = (*cirecv)(&recv);

			if (strlen(recv) > 0) {

				// Concatenate the last data with the previous part.
				strcat(buf, recv);

				// Log_Debug("COMMANDINT: New data received (%s), now the buffer is \"%s\".\n", recv, buf);
			}

			// Loop until command is complete.
		} while (result == -1);

		Log_Debug("COMMANDINT: The command is complete. The content of the buffer is \"%s\".\n", buf);

		// TODO: Decrypt encrypted commands (start with *).

		// TODO: Filter and process special commands.

		// Log_Debug("COMMANDINT: Processing command into a command_t.\n");

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
			strcpy(answer, "PING");
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
				strcpy(answer, "OOBE");
				answered = 1;
			}
			else {
				Log_Debug("COMMANDINT: OOBE denied. Device is already configured.\n");
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcpy(answer, "OOBEE");
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
				strcpy(answer, "WISCE");
				answered = 1;
			}
			// If there are no networks...
			else if (result == 0) {
				Log_Debug("COMMANDINT: Wi-Fi scan found no networks.\n");
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcpy(answer, "WISCN");
				answered = 1;
			}
			// If there are networks...
			else {

				Log_Debug("COMMANDINT: Wi-Fi scan found %d networks.\n", result);

				// Array of available networks
				WifiConfig_ScannedNetwork networks[result];
				WifiConfig_GetScannedNetworks(&networks, result);

				// 6 = WISCF + \n, (result - 1) for separators (#), result for WPA indicators (*), (result * 5) for RSSI (-100) and separator (^)
				int length = 6 + (result - 1) + result + (result * 5);

				// Then we add to length the length of every SSID
				for (int i = 0; i < result; i++) {
					length += networks[i].ssidLength;
				}

				// Allocating space for the answer
				answer = (char*)malloc(length * sizeof(char));
				memset(answer, 0, length * sizeof(char));
				strcpy(answer, "WISCF");

				// Adds the scanned SSIDs to the answer
				for (int i = 0; i < result; i++) {

					if (networks[i].security == WifiConfig_Security_Wpa2_Psk) {
						strcat(answer, "*");
					}

					strncat(answer, networks[i].ssid, networks[i].ssidLength);

					// Adding RSSI information
					char rssi[5];
					sprintf(rssi, "^%d", networks[i].signalRssi);
					strcat(answer, rssi);


					answer[strlen(answer)] = '\0';

					// Except the last time, we use a # as a separator
					if (i != result - 1) {
						strcat(answer, "#");
					}
				}

				answered = 1;
			}
		}
		// Get saved Wi-Fi networks list
		else if (!strcmp(currentCommand.header, "WISA")) {

			Log_Debug("COMMANDINT: Command decoded as Wi-Fi saved networks request.\n");
			int result;

			// We get the number of networks saved.
			result = WifiConfig_GetStoredNetworkCount();

			// If there is an error...
			if (result == -1) {
				Log_Debug("COMMANDINT: Wi-Fi saved request error: \"%s\".\n", strerror(errno));
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcpy(answer, "WISAE");
				answered = 1;
			}
			// If there are no networks...
			else if (result == 0) {
				Log_Debug("COMMANDINT: No saved networks found.\n");
				answer = malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcpy(answer, "WISAN");
				answered = 1;
			}
			// If there are networks...
			else {

				Log_Debug("COMMANDINT: There are %d saved Wi-Fi networks.\n", result);

				// Array of available networks
				WifiConfig_StoredNetwork networks[result];
				WifiConfig_GetStoredNetworks(&networks, result);

				// 6 = WISAF + \n + eventual connected and protection markers (^, *), result - 1 for separators (#)
				int length = 7 + (2 * result) - 1;

				// Then we add to length the length of every SSID
				for (int i = 0; i < result; i++) {
					length += networks[i].ssidLength;
				}

				// Allocating space for the answer
				answer = (char*)malloc(length * sizeof(char));
				memset(answer, 0, length * sizeof(char));
				strcpy(answer, "WISAF");

				// Adds the scanned SSIDs to the answer
				for (int i = 0; i < result; i++) {

					// Mark the connected one
					if (networks[i].isConnected) {
						strcat(answer, "^");
					}

					if (networks[i].security == WifiConfig_Security_Wpa2_Psk) {
						strcat(answer, "*");
					}

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
		// Forget a Wi-Fi network
		else if (!strcmp(currentCommand.header, "WIRM")) {

			Log_Debug("COMMANDINT: Command decoded as Wi-Fi forget request.\n");
			int result;

			WifiConfig_StoredNetwork foundNetwork;

			// We get the number of networks saved.
			result = WifiConfig_GetStoredNetworkCount();

			if (result > 0) {
				// Then all the saved networks
				WifiConfig_StoredNetwork networks[result];
				WifiConfig_GetStoredNetworks(&networks, result);

				// Search for a network with same SSID
				for (int i = 0; i < result; i++) {

					// If we have a match we save it. SSID property is not null-terminated.
					if (!strncmp(currentCommand.arguments, networks[i].ssid, networks[i].ssidLength)) {
						Log_Debug("COMMANDINT: Network to be forgotten was found.\n");
						foundNetwork = networks[i];
					}
				}

				// Try to remove
				if (WifiConfig_ForgetNetwork(&foundNetwork) != -1) {

					Log_Debug("COMMANDINT: Network forgotten successfully.\n");

					// Removed
					answer = (char*)malloc(5 * sizeof(char));
					memset(answer, 0, 5 * sizeof(char));
					strcpy(answer, "WIRM");
					answered = 1;
				}
				else {

					Log_Debug("COMMANDINT: Error while forgetting network. Error is \"%s\".\n", strerror(errno));

					// Error
					answer = (char*)malloc(6 * sizeof(char));
					memset(answer, 0, 6 * sizeof(char));
					strcpy(answer, "WIRME");
					answered = 1;
				}
			}
			else {

				Log_Debug("COMMANDINT: No saved networks.\n");

				// No networks saved
				answer = (char*)malloc(6 * sizeof(char));
				memset(answer, 0, 6 * sizeof(char));
				strcpy(answer, "WIRME");
				answered = 1;
			}
		}
		// Add a Wi-Fi network
		else if (!strcmp(currentCommand.header, "WIAD")) {

			Log_Debug("COMMANDINT: Command decoded as Wi-Fi add request.\n");
			int result;

			if (strchr(currentCommand.arguments, '#') == NULL) {

				Log_Debug("COMMANDINT: Adding open network \"%s\".\n", currentCommand.arguments);

				// Open network
				if (WifiConfig_StoreOpenNetwork(currentCommand.arguments, strlen(currentCommand.arguments)) == -1) {
					Log_Debug("COMMANDINT: Error while adding open network. Error is \"%s\".\n", strerror(errno));
					answer = (char*)malloc(6 * sizeof(char));
					memset(answer, 0, 6 * sizeof(char));
					strcpy(answer, "WIADE");
					answered = 1;
				}
				else {
					Log_Debug("COMMANDINT: Open network added successfully.\n");
					answer = (char*)malloc(5 * sizeof(char));
					memset(answer, 0, 5 * sizeof(char));
					strcpy(answer, "WIAD");
					answered = 1;
				}

			}
			else {

				// Protected network, # separates SSID and PSK
				int tokenpos = strchr(currentCommand.arguments, '#') - currentCommand.arguments;

				char* ssid = (char*)malloc((tokenpos + 1) * sizeof(char));
				memset(ssid, 0, (tokenpos + 1) * sizeof(char));

				char* password = (char*)malloc((strlen(currentCommand.arguments) - tokenpos) * sizeof(char));
				memset(password, 0, (strlen(currentCommand.arguments) - tokenpos) * sizeof(char));

				// Splitting
				strcpy(ssid, strtok(currentCommand.arguments, "#"));
				strcpy(password, strtok(NULL, "#"));

				Log_Debug("COMMANDINT: Adding protected network with SSID \"%s\" (%d) and password \"%s\" (%d).\n", ssid, strlen(ssid), password, strlen(password));

				if (WifiConfig_StoreWpa2Network(ssid, strlen(ssid), password, strlen(password)) == -1) {
					Log_Debug("COMMANDINT: Error while adding protected network. Error is \"%s\".\n", strerror(errno));
					answer = (char*)malloc(6 * sizeof(char));
					memset(answer, 0, 6 * sizeof(char));
					strcpy(answer, "WIADE");
					answered = 1;
				}
				else {
					Log_Debug("COMMANDINT: Protected network added successfully.\n");
					answer = (char*)malloc(5 * sizeof(char));
					memset(answer, 0, 5 * sizeof(char));
					strcpy(answer, "WIAD");
					answered = 1;
				}
			}
		}
		// Is OBD module connected?
		else if (!strcmp(currentCommand.header, "OBDC")) {
			Log_Debug("COMMANDINT: Command decoded as OBD module connected request.\n");
			answer = (char*)malloc(6 * sizeof(char));
			memset(answer, 0, 6 * sizeof(char));
			strcpy(answer, "OBDC");

			// 1: Connected, 0: Disconnected
			strcat(answer, OBD.connected ? "1" : "0");
			answered = 1;
		}
		// Is car connected?
		else if (!strcmp(currentCommand.header, "CARC")) {
			Log_Debug("COMMANDINT: Command decoded as car connected request.\n");
			answer = (char*)malloc(6 * sizeof(char));
			memset(answer, 0, 6 * sizeof(char));
			strcpy(answer, "CARC");

			// 1: Connected, 0: Disconnected
			strcat(answer, car.initialized ? "1" : "0");
			answered = 1;
		}
		// Is SD card mounted?
		else if (!strcmp(currentCommand.header, "SDMN")) {
			Log_Debug("COMMANDINT: Command decoded as SD mounted request.\n");
			answer = (char*)malloc(6 * sizeof(char));
			memset(answer, 0, 6 * sizeof(char));
			strcpy(answer, "SDMN");

			// 1: Mounted, 0: Unmounted
			strcat(answer, SDmounted ? "1" : "0");
			answered = 1;
		}
		// SD card size
		else if (!strcmp(currentCommand.header, "SDSZ")) {
			Log_Debug("COMMANDINT: Command decoded as SD size request.\n");
			answer = (char*)malloc(25 * sizeof(char));
			memset(answer, 0, 25 * sizeof(char));


			DWORD tot_sect;

			/* Get total sectors and free sectors */
			tot_sect = (SD.n_fatent - 2) * SD.csize;

			/* Print the free space in KiB (assuming 512 bytes/sector) */
			sprintf(answer, "SDSZ%lu", tot_sect / 2);

			answered = 1;
		}
		// Get last file name
		else if (!strcmp(currentCommand.header, "LFNM")) {
			Log_Debug("COMMANDINT: Command decoded as current file name request.\n");

			SDThreadLock = 1;

			// Wait for confirmation
			while (SDThreadLock != 2);

			Log_Debug("COMMANDINT: SD thread locked.\n");

			answer = (char*)malloc(25 * sizeof(char));
			memset(answer, 0, 25 * sizeof(char));

			char* name = malloc(16 * sizeof(char));
			int i = 0;
			FRESULT res;

			// Search for files with incremental names.
			do {
				memset(name, 0, sizeof(name));
				sprintf(name, "%d.log", i);
				res = f_stat(name, NULL);
				i++;
				
				// 1ms delay
				struct timespec t;
				t.tv_nsec = 1000000UL;
				nanosleep(&t, NULL);

			} while (res == FR_OK);

			// O = OK
			if (res == FR_NO_FILE && i > 1) {

				sprintf(name, "%d.log", i - 2);
				sprintf(answer, "LFNMO%s", name);
			}
			// Error (E)
			else {
				sprintf(answer, "LFNME");
			}

			free(name);

			SDThreadLock = 0;

			answered = 1;
		}
		// Get file size
		else if (!strcmp(currentCommand.header, "GFSZ")) {
			Log_Debug("COMMANDINT: Command decoded as file size request.\n");

			SDThreadLock = 1;

			// Wait for confirmation
			while (SDThreadLock != 2);

			Log_Debug("COMMANDINT: SD thread locked.\n");

			answer = (char*)malloc(35 * sizeof(char));
			memset(answer, 0, 35 * sizeof(char));

			FILINFO fno;

			// File exists, O = ok
			if (f_stat(currentCommand.arguments, &fno) == FR_OK) {
				sprintf(answer, "GFSZO%s#%d", fno.fname, (int)fno.fsize);
			}
			// File doesn't exist
			else {
				sprintf(answer, "GFSZE%s", currentCommand.arguments);
			}

			SDThreadLock = 0;

			answered = 1;
		}
		// Get file content (SPECIAL ANSWER)
		else if (!strcmp(currentCommand.header, "GFIL")) {
			Log_Debug("COMMANDINT: Command decoded as file content request.\n");


			// Save SDThreadLock before?
			// Safe R/W
			SDThreadLock = 1;

			// Wait for confirmation
			while (SDThreadLock != 2);

			Log_Debug("COMMANDINT: SD thread locked.\n");

			// Performance
			stopGPSThread();
			// stopOBDThread();

			// Check whether we're trying to read the current file
			char* name = malloc(16 * sizeof(char));
			int i = 0;
			FRESULT res;

			bool isLastFile = false;

			do {
				memset(name, 0, sizeof(name));
				sprintf(name, "%d.log", i);
				res = f_stat(name, NULL);
				i++;
			} while (res == FR_OK);

			sprintf(name, "%d.log", i - 2);
			if (!strcmp(name, currentCommand.arguments))
				isLastFile = true;

			answer = (char*)malloc(35 * sizeof(char));
			memset(answer, 0, 35 * sizeof(char));

			FIL file;

			const unsigned int smallBlockSize = 1024;
			const unsigned int largeBlockSize = 8192;

			// Not allowed to read last file, and in case of error break now.
			if (isLastFile || res != FR_NO_FILE) {
				sprintf(answer, "GFILE%s", currentCommand.arguments);
			}
			// Not last
			else {

				// Esists
				if (f_open(&file, currentCommand.arguments, FA_READ) == FR_OK) {


					// Start reading
					unsigned long long transferstart = nanos();

					// Send header (S = start)
					sprintf(answer, "GFILS%s", currentCommand.arguments);
					(*cisend)(answer);

					// Start sending data
					int readBytes;
					int totalReadBytes;
					char* buf = (char*)malloc((largeBlockSize + 1) * sizeof(char));

					do {

						totalReadBytes = 0;

						// Clean temp buffer
						memset(buf, 0, (largeBlockSize + 1) * sizeof(char));

						// Read large block
						do {
							int bytesToRead = ((largeBlockSize - totalReadBytes) < smallBlockSize) ? (largeBlockSize - totalReadBytes) : smallBlockSize;
							f_read(&file, buf + totalReadBytes, bytesToRead, &readBytes);
							totalReadBytes += readBytes;

							// Read until big chunk is ready or EOF
						} while (totalReadBytes < largeBlockSize && readBytes == smallBlockSize);


						// Send buffer
						for (int i = 0; i < totalReadBytes; i++) {

							char c = buf[i];

							// Loop until buffer has free space
							while ((*sendChar)(c) == -1) { ; }
						}

						// Loop until EOF
					} while (totalReadBytes == largeBlockSize);

					free(buf);
					f_close(&file);

					unsigned long long transferend = nanos();

					unsigned long long elapsed = transferend - transferstart;

					// Send footer (O = ok)
					memset(answer, 0, 35 * sizeof(char));
					sprintf(answer, "GFILO%s", currentCommand.arguments);

				}
				// File doesn't exist
				else {
					sprintf(answer, "GFILE%s", currentCommand.arguments);
				}
			}

			SDThreadLock = 0;

			startGPSThread();
			// startOBDThread();

			answered = 1;
		}



		// Answer with the requested data.
		// TODO: check results

		// This filters errors when receiving invalid commands.
		if (answered) {
			Log_Debug("COMMANDINT: Sending answer. Content is \"%s\".\n", answer);
			(*cisend)(answer);
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

int startCommandInterpreter(int (*_receive)(char**), int (*_send)(char*), int (*_sendChar)(char)) {

	// Allows the thread to continue.
	halt = 0;

	Log_Debug("COMMANDINT: Starting command interpreter thread.\n");

	// Associate read and write functions (generic in order to allow different connections, not only TCP).
	cirecv = _receive;
	cisend = _send;
	sendChar = _sendChar;

	// Start the thread.
	pthread_create(&ciThread, NULL, commandInterpreterThread, NULL);
	// TODO: pthread_setschedparam(ciThread, ..., ...);
}


int stopCommandInterpreter()
{
	Log_Debug("COMMANDINT: Stopping command interpreter thread.\n");

	// Exits the main loop.
	halt = 1;

	// 2 ms wait
	struct timespec ts = { 0, 2000000 };
	nanosleep(&ts, NULL);

	// Check if it is really stopped.
	if (halt == 2) {
		return 0;
	}

	// If not, return -1.
	return -1;
}