#include "cardmanager.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/rtc.h>

// ulibSD: SPI <-> block
// FATFS: block <-> file system

bool mounted = false;

int newSession() {

	Log_Debug("CARDMANAGER: Creating new session.\n");

	FILINFO fno;
	FRESULT res;
	
	char* name = malloc(16 * sizeof(char));
	int i = 0;

	// Search for files with incremental names.
	do {
		memset(name, 0, sizeof(name));
		sprintf(name, "%d.log", i);
		res = f_stat(name, &fno);
		i++;
	} while (res == FR_OK);

	Log_Debug("CARDMANAGER: Creating file \"%s\".\n", name);

	res = -1;

	// When we reached a number that previously didn't exist, we create that file.
	res = f_open(&currentFile, name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);

	if (res == FR_OK) {
		Log_Debug("CARDMANAGER: File created successfully.\n");
	}
	else {
		Log_Debug("CARDMANAGER: Error during file creation (%d).\n", res);
		return -1;
	}

	f_close(&currentFile);
	stopSD();

	return 0;
}

int initializeSD() {

	Log_Debug("CARDMANAGER: Mounting SD card.\n");

	// Mount SD card. 1 = Mount now.
	FRESULT res = f_mount(&SD, "", 1);

	DWORD vsn;
	char label[12];

	f_getlabel("", label, &vsn);

	if (res == FR_OK) {
		Log_Debug("CARDMANAGER: SD \"%s\" (%u) mounted successfully.\n", label, vsn);
		mounted = true;
		newSession();
		return 0;
	}
	else {
		Log_Debug("CARDMANAGER: SD \"%s\" (%u) failed to mount. Error code is %d.\n", label, vsn, res);
		mounted = false;
		return -1;
	}

}

int logToSD(char* data) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	struct tm* time = gmtime(&ts.tv_sec);
	
	// | DATE & TIME | \t | DATA | \r\n | NULL TERMINATOR |
	// Date and time: YYYY-MM-DD HH:mm:ss (maximum is 19 chars)
	int length = 23;												// Time, \t, \r\n and terminator
	length += strlen(data);

	// Create a buffer and erase it
	char* buf = malloc(length * sizeof(char));
	memset(buf, 0, sizeof(buf));

	sprintf(buf, "%d-%d-%d %d:%d:%d\t%s\r\n",
		time->tm_year + 1900,
		time->tm_mon + 1,
		time->tm_mday,
		time->tm_hour,
		time->tm_min,
		time->tm_sec,
		data);
}

int stopSD() {
	Log_Debug("CARDMANAGER: Unmounting SD card.\n");

	// Unmounts the default drive (SD card)
	FRESULT res = f_mount(0, "", 0);

	int s = 0;

	if (res == FR_OK) {
		Log_Debug("CARDMANAGER: SD unmounted successfully.\n");
	}
	else {
		Log_Debug("CARDMANAGER: SD unmount error. Error code is %d.\n", res);
		s = -1;
	}

	mounted = false;

	return s;
}