#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


#include "appTCP.h"
#include "obdserial.h"
#include "config.h"
#include "commandinterpreter.h"

int main(void)
{
    // This minimal Azure Sphere app repeatedly toggles GPIO 9, which is the green channel of RGB
    // LED 1 on the MT3620 RDB.
    // Use this app to test that device and SDK installation succeeded that you can build,
    // deploy, and debug an app with Visual Studio, and that you can deploy an app over the air,
    // per the instructions here: https://docs.microsoft.com/azure-sphere/quickstarts/qs-overview
    //
    // It is NOT recommended to use this as a starting point for developing apps; instead use
    // the extensible samples here: https://github.com/Azure/azure-sphere-samples

    int fd = GPIO_OpenAsOutput(PIN_LED_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (fd < 0) {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return -1;
    }

	startTCPThreads();

	// Connect the command interpreter to the TCP buffers.
	startCommandInterpreter(readTCPString, writeTCPString);
	initStandardOBDModule();

	struct timespec ts = {0, 200000000};
    while (1) {
		GPIO_SetValue(fd, GPIO_Value_Low);
		nanosleep(&ts, NULL);
		GPIO_SetValue(fd, GPIO_Value_High);
		sleep(3);
    }
}