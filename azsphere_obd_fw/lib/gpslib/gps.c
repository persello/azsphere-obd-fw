#include "gps.h"

#include "../../config.h"
#include "minmea.h"
#include "intercorecomm.h"

#include <pthread.h>
#include <string.h>

#include <applibs/log.h>

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
				Log_Debug("$RMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
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
					minmea_tofloat(&frame.speed));
			}
		} break;

		case MINMEA_SENTENCE_GGA: {
			struct minmea_sentence_gga frame;
			if (minmea_parse_gga(&frame, sentence)) {
				Log_Debug("$GGA: fix quality: %d\n", frame.fix_quality);
			}
		} break;

		case MINMEA_SENTENCE_GSV: {
			struct minmea_sentence_gsv frame;
			if (minmea_parse_gsv(&frame, sentence)) {
				Log_Debug("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
				Log_Debug("$GSV: sattelites in view: %d\n", frame.total_sats);
				for (int i = 0; i < 4; i++)
					Log_Debug("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
						frame.sats[i].nr,
						frame.sats[i].elevation,
						frame.sats[i].azimuth,
						frame.sats[i].snr);
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
