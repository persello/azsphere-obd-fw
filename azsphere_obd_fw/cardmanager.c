#include "cardmanager.h"

#include "lib/circularbuffer/buffer.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/rtc.h>

// ulibSD: SPI <-> block
// FATFS: block <-> file system

bool mounted = false;
bool fileopened = false;
pthread_t SDThread;

buffer_t writeBuffer;
buffer_t readBuffer;

int threadStatus = 0;

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
		fileopened = true;
	}
	else {
		Log_Debug("CARDMANAGER: Error during file creation (%d).\n", res);
		fileopened = false;
		return -1;
	}

	return 0;
}

int mountSD() {

	Log_Debug("CARDMANAGER: Mounting SD card.\n");

	// Mount SD card. 1 = Mount now.
	FRESULT res = f_mount(&SD, "", 1);

	DWORD vsn;
	char label[100];

	f_getlabel("", label, &vsn);

	if (res == FR_OK) {
		Log_Debug("CARDMANAGER: SD \"%s\" (%u) mounted successfully.\n", label, vsn);
		mounted = true;
		return 0;
	}
	else {
		Log_Debug("CARDMANAGER: SD card failed to mount. Error code is %d.\n", res);
		mounted = false;
		return -1;
	}

}

int unmountSD() {
	Log_Debug("CARDMANAGER: Unmounting SD card.\n");

	// Close the file, if opened, before unmounting.
	if (&currentFile) {

		Log_Debug("CARDMANAGER: A file was opened. Closing now.\n");

		FRESULT r = f_close(&currentFile);
		fileopened = false;

		memset(&currentFile, 0, sizeof(currentFile));

		if (r == FR_OK) {
			Log_Debug("CARDMANAGER: File closed successfully.\n");
		}
		else {
			Log_Debug("CARDMANAGER: Cannot close file (%d).\n", r);
		}
	}

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

int logToSD(char* _data) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	struct tm* time = gmtime(&ts.tv_sec);

	// | DATE & TIME | \t | DATA | \r\n | NULL TERMINATOR |
	// Date and time: YYYY-MM-DD HH:mm:ss.ttt (maximum is 23 chars)
	int length = 27;												// Time, \t, \r\n and terminator
	length += strlen(_data);

	// Create a buffer and erase it
	char* buf = malloc(length * sizeof(char));
	memset(buf, 0, sizeof(buf));

	sprintf(buf, "%d-%d-%d %d:%d:%d.%d\t%s\r\n",
		time->tm_year + 1900,
		time->tm_mon + 1,
		time->tm_mday,
		time->tm_hour,
		time->tm_min,
		time->tm_sec + ((ts.tv_nsec / 1000000) > 999),
		(ts.tv_nsec / 1000000) % 1000,
		_data);


	// TODO: Implement a simple thread lock if necessary
	for (int i = 0; i < strlen(buf); i++) {
		if (putCharBuffer(&writeBuffer, buf[i]) == -1) {
			Log_Debug("CARDMANAGER: Write buffer is full! Data will be lost!\n");
			break;
		}
	}
}

void* SDThreadMain(void* _param) {

	Log_Debug("CARDMANAGER: SD thread started.\n");

	while (!threadStatus) {

		if (!mounted) {

			// We try to mount the file system continuously.
			mountSD();
		}
		else {

			if (fileopened) {

				// When the file is opened...
				// Write from the circular buffer
				char c;
				while (getCharBuffer(&writeBuffer, &c) != -1) {
					f_putc(c, &currentFile);
				}

				// Sync the changes to the SD card for safety reasons.
				// Also serves as SD insertion check (unmounts on fail).
				if (f_sync(&currentFile) != FR_OK) {
					unmountSD();
				}
			}
			else {

				// Create a new session.

				if (newSession() != 0) {

					// Unmount device on fail.
					unmountSD();
				}
			}
		}
	}

	// Unmount before exiting

	unmountSD();

	Log_Debug("CARDMANAGER: SD thread stopped.\n");
}

int startSDThread() {

	Log_Debug("CARDMANAGER: Starting SD thread.\n");

	// Allow it to loop.
	threadStatus = 0;

	// Initialize the buffers
	initCircBuffer(&writeBuffer, 4096);
	initCircBuffer(&readBuffer, 4096);

	// Start a thread.
	pthread_create(&SDThread, NULL, SDThreadMain, NULL);
}

int stopSDThread() {

	Log_Debug("CARDMANAGER: Stopping SD thread.\n");

	// Request to exit the loop
	threadStatus = 1;
}