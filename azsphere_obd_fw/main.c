﻿#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


#include "appTCP.h"
#include "cardmanager.h"
#include "commandinterpreter.h"
#include "config.h"
#include "obdserial.h"



static volatile sig_atomic_t terminationRequested = 0;

void TerminationHandler(void) {
	Log_Debug("MAIN: SIGTERM caught! Halting all the threads before exiting!\n");
	terminationRequested = 1;
}

int main(void)
{

	Log_Debug("MAIN: Application started. Registering SIGTERM handler.\n");

	// Register a SIGTERM handler for termination requests
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

	Log_Debug("MAIN: SIGTERM handler registered, initializing SD card.\n");

	// FAT File Sytem Initialization
	startSDThread();

	Log_Debug("MAIN: Initializing GPIOs.\n");

	// GPIO initialization
	int btnafd = GPIO_OpenAsInput(PIN_BTN_A);
	if (btnafd < 0) {
		Log_Debug(
			"MAIN: Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}
	int btnbfd = GPIO_OpenAsInput(PIN_BTN_B);
	if (btnbfd < 0) {
		Log_Debug(
			"MAIN: Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}


	Log_Debug("MAIN: Starting all the threads.\n");

	// The TCP threads are initially stopped.
	TCPThreadStatus = STATUS_UNKNOWN;

	// Starts TCP I/O threads.
	startTCPThreads();

	// Connect the command interpreter to the TCP buffers.
	startCommandInterpreter(readTCPString, writeTCPString);

	// Starts the serial with parameters specified in config.h.
	startOBDThread();

	Log_Debug("MAIN: Initialization finished.\n");

	GPIO_Value_Type btnares;
	GPIO_Value_Type btnaprev;
	GPIO_Value_Type btnbres;
	GPIO_Value_Type btnbprev;


	while (!terminationRequested) {

		GPIO_GetValue(btnafd, &btnares);
		if (btnares == GPIO_Value_High && btnaprev == GPIO_Value_Low) {
			writeTCPString("BTNA");
			Log_Debug("MAIN: Button A pressed, adding message to output buffer.\n");
		}
		btnaprev = btnares;

		/*GPIO_GetValue(btnbfd, &btnbres);
		if (btnbres == GPIO_Value_High && btnbprev == GPIO_Value_Low && TCPThreadStatus == STATUS_RUNNING) {
			stopTCPThreads();
			Log_Debug("MAIN: Button B pressed, resetting TCP threads.\n");
		}
		btnbprev = btnbres;*/

		if (TCPThreadStatus == STATUS_STOPPED) {
			Log_Debug("MAIN: TCP threads are stopped. Restarting...\n");
			startTCPThreads();
		}
	}

	stopCommandInterpreter();
	stopTCPThreads();
	stopSDThread();

	Log_Debug("MAIN: Services stopped. Exiting from application.\n");

	Log_Debug("MAIN: Application halted.\n");
}