#include "intercorecomm.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <applibs/log.h>
#include <applibs/application.h>

#include "minmea.h"

const char* lowLevelAppId = "005180bc-402f-4cb3-a662-72937dbcde47";
int sockfd = 0;

int initSSSocket() {
	if (!sockfd) {

		sockfd = Application_Socket(lowLevelAppId);

		if (sockfd == -1) {
			Log_Debug("INTERCORECOMM: Error while opening socket to M4 RT application. Error: \"%s\" (%d).\n", strerror(errno), errno);
			return -1;
		}
		else {
			Log_Debug("INTERCORECOMM: M4 RT intercore socket initialized with fd=%d.\n", sockfd);
		}
	}

	return 0;
}

int closeSSSocket() {
	Log_Debug("INTERCORECOMM: Closing M4 core socket.\n");
	int val = close(sockfd);
	sockfd = 0;
	return val;
}

int readStringSS(char** data) {

	// Stop when empty or on \r\n
	char c;
	int result;
	do {

		static char mes[5] = { };

		// Send a char in order to receive one
		strcpy(mes, "-");
		send(sockfd, mes, strlen(mes), 0);

		// Strange behavior: returns -1 but also correct data
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