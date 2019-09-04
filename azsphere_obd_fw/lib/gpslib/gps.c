#include "gps.h"

#include "../../config.h"
#include "minmea.h"
#include "intercorecomm.h"
#include "../../cardmanager.h"

#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include <applibs/log.h>
#include <applibs/rtc.h>

pthread_t GPSThread;

int GPSThreadStatus = 0;

void* GPSThreadMain(void* _param) {

	while (!GPSThreadStatus) {

		// Get a complete string
		char sentence[MINMEA_MAX_LENGTH] = { };

		// Same as while(1) but exits on thread exit request
		while (!GPSThreadStatus) {
			char* rec = malloc(MINMEA_MAX_LENGTH);
			memset(rec, 0, MINMEA_MAX_LENGTH);
			readStringSS(&rec);

			// Add to sentence
			if ((strlen(rec) + strlen(sentence)) < MINMEA_MAX_LENGTH - 1) {
				strcat(sentence, rec);
			}
			else {

				// Reset when full
				memset(sentence, 0, MINMEA_MAX_LENGTH);
			}

			free(rec);

			// Check for CRLF. If found, break.
			char* ending = strrchr(sentence, '\r');
			if (ending && !strcmp(ending, "\r\n")) break;
		}

		// Now we have a CRLF terminated string. Let's decode it.

		switch (minmea_sentence_id(sentence, false)) {
		case MINMEA_SENTENCE_RMC: {
			struct minmea_sentence_rmc frame;
			if (minmea_parse_rmc(&frame, sentence)) {
				/*Log_Debug("$RMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
					frame.latitude.value, frame.latitude.scale,
					frame.longitude.value, frame.longitude.scale,
					frame.speed.value, frame.speed.scale);
				Log_Debug("$RMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
					minmea_rescale(&frame.latitude, 1000),
					minmea_rescale(&frame.longitude, 1000),
					minmea_rescale(&frame.speed, 1000));
				Log_Debug("$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
					minmea_tocoord(&frame.latitude),
					minmea_tocoord(&frame.longitude),
					minmea_tofloat(&frame.speed));*/

				// Time sync management
				struct timespec now;
				clock_gettime(CLOCK_REALTIME, &now);

				struct timespec gpstime;
				minmea_gettime(&gpstime, &frame.date, &frame.time);
				
				// More than 30 seconds out of sync
				if (abs(now.tv_sec - gpstime.tv_sec) > 30) {

					// Last log item with wrong time
					logToSD("RTCOLDTIME\t?");

					clock_settime(CLOCK_REALTIME, &gpstime);
					clock_systohc();
					Log_Debug("GPS: Time was synced to satellite.\n");

					// First log item with correct time
					logToSD("RTCNEWTIME\tGPS");
				}

				// Real-time location and speed data

				realTimeGPSData.lat = minmea_tocoord(&frame.latitude);
				realTimeGPSData.lon = minmea_tocoord(&frame.longitude);
				realTimeGPSData.course = minmea_tofloat(&frame.course);
				realTimeGPSData.speed = minmea_tofloat(&frame.speed) * 1.852f;	// Knots to km/h
				
				// Set to zero when reading in order to avoid duplicates
				realTimeGPSData.newData = 1;

				// Log data to SD card
				char buf[100];

				memset(buf, 0, 100);
				sprintf(buf, "RTLATITUDE\t%f", realTimeGPSData.lat);
				logToSD(buf);

				memset(buf, 0, 100);
				sprintf(buf, "RTLONGITUD\t%f", realTimeGPSData.lon);
				logToSD(buf);

				memset(buf, 0, 100);
				sprintf(buf, "GPSSPEEDKM\t%f", realTimeGPSData.speed);
				logToSD(buf);

				memset(buf, 0, 100);
				sprintf(buf, "GPSVCOURSE\t%f", realTimeGPSData.course);
				logToSD(buf);

			}
		} break;
		}
	}

	Log_Debug("GPS: Exiting from thread.\n");
}

int startGPSThread()
{

	Log_Debug("GPS: Trying to initialize SoftwareSerial and to start GPS thread.\n");

	if (initSSSocket() == 0) {

		// GPS thread
		GPSThreadStatus = 0;
		pthread_create(&GPSThread, NULL, GPSThreadMain, NULL);
	}
	else {
		Log_Debug("GPS: Failed to initialize SoftwareSerial.\n");
	}

	return 0;
}

int stopGPSThread()
{
	GPSThreadStatus = 1;

	return 0;
}
