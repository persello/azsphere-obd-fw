#include "intercorecomm.h"

#include <errno.h>
#include <string.h>
#include "minmea.h"

#include <sys/socket.h>

#include <applibs/log.h>
#include <applibs/application.h>

const char* lowLevelAppId = "005180bc-402f-4cb3-a662-72937dbcde47";
int sockfd = 0;

int initSSSocket() {
	sockfd = Application_Socket(lowLevelAppId);

	if (sockfd == -1) {
		Log_Debug("INTERCORECOMM: Error while opening socket to M4 RT application. Error: \"%s\" (%d).\n", strerror(errno), errno);
		return -1;
	}

	return 0;
}

int readStringSS(char** data) {

	// Stop when empty or on \r\n
	char c;
	int result;
	do {
		result = recv(sockfd, &c, 1, MSG_DONTWAIT) > 0;

		// We received a character
		if (result)
			strncat(*data, &c, 1);

		// Break on \r\n
		char* ending = strrchr(*data, '\r');
		if (ending && !strcmp(ending, "\r\n")) break;
	} while (result > 0 && strlen(*data) < MINMEA_MAX_LENGTH);

	if (strlen(*data)) {
		// Log_Debug("INTERCORECOMM: New data (%s).\n", *data);
		return 0;
	}
	else {
		return -1;
	}
}