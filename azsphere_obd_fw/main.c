#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


#include "appTCP.h"
#include "obdserial.h"
#include "config.h"
#include "commandinterpreter.h"


void TerminationHandler() {
	Log_Debug("MAIN: SIGTERM caught! Halting all the threads before exiting!\n");
	stopCommandInterpreter();
	stopTCPThreads();
	Log_Debug("MAIN: Threads stopped. Exiting from application.\n");
}

int main(void)
{

	Log_Debug("MAIN: Application started. Registering SIGTERM handler.\n");

	// Register a SIGTERM handler for termination requests
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

	Log_Debug("MAIN: SIGTERM handler registered, initializing GPIOs.\n");

	// GPIO initialization
	int fd = GPIO_OpenAsOutput(PIN_LED_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (fd < 0) {
		Log_Debug(
			"MAIN: Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}

	Log_Debug("MAIN: Starting all the threads.\n");

	// Starts TCP I/O threads.
	startTCPThreads();

	// Connect the command interpreter to the TCP buffers.
	startCommandInterpreter(readTCPString, writeTCPString);

	// Starts the serial with parameters specified in config.h.
	initStandardOBDModule();

	Log_Debug("MAIN: Initialization finished.\n");

	struct timespec ts = { 0, 200000000 };
	while (1) {
		GPIO_SetValue(fd, GPIO_Value_Low);
		nanosleep(&ts, NULL);
		GPIO_SetValue(fd, GPIO_Value_High);
		sleep(3);
	}
}