#include "statusleds.h"
#include "obdserial.h"
#include "cardmanager.h"
#include "config.h"

#include "lib/timer/Inc/Public/timer.h"

#include <applibs/log.h>

#define _XOPEN_SOURCE

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <string.h>


#define COLOR_RED		{ 1000, 231, 188 }
#define COLOR_ORANGE	{ 1000, 584, 0 }
#define COLOR_YELLOW	{ 1000, 800, 0 }
#define COLOR_GREEN		{ 204, 780, 349 }
#define COLOR_BLUE		{ 0, 478, 1000 }
#define COLOR_WHITE		{ 1000, 1000, 1000 }

#define BLINK_PERIOD	5000

int statusLEDsfd = 0, colorLEDfd = 0;
int statusLEDThreadStatus = 0;

color_t currentLEDColor = { 0, 0, 0 };
int isBlinking = 0;
int currentAppLEDStatus = 0;
int currentWlanLEDStatus = 0;

PWM_ChannelId wlanLEDChannel, appLEDChannel, redLEDChannel, blueLEDChannel, greenLEDchannel;

void* statusLEDThread(void* _param) {

	Log_Debug("STATUSLEDS: Thread is started.\n");

	initializeLEDControllers(LED_CONTROLLER, RGB_CONTROLLER,
		CH_LED_WLAN, CH_LED_APP,
		CH_LED_RED, CH_LED_GREEN, CH_LED_BLUE);

	while (statusLEDThreadStatus == 0) {
		// Update target color

		// 1: Blinking red: Very low car battery voltage
		if (car.lastBatteryVoltage > 5.0 && car.lastBatteryVoltage < 11.6) {
			currentLEDColor = (color_t)COLOR_RED;
			isBlinking = true;
		}
		// 2: Blinking yellow: Low car battery voltage
		else if (car.lastBatteryVoltage > 5.0 && car.lastBatteryVoltage < 12.3) {
			currentLEDColor = (color_t)COLOR_ORANGE;
			isBlinking = true;
		}
		// 3: Blinking blue: OBD Module not found
		else if (!OBD.connected) {
			currentLEDColor = (color_t)COLOR_BLUE;
			isBlinking = true;
		}
		// 4: Blinking yellow: SD Card not mounted
		else if (!SDmounted) {
			currentLEDColor = (color_t)COLOR_YELLOW;
			isBlinking = true;
		}
		// 5: Solid yellow: Power but ignition off
		else if (car.lastBatteryVoltage > 5.0 && car.initialized == false) {
			currentLEDColor = (color_t)COLOR_YELLOW;
			isBlinking = false;
		}
		// 6: Blinking white: Normal operation
		else {
			currentLEDColor = (color_t)COLOR_WHITE;
			isBlinking = true;
		}

		// Update blink
		color_t calculatedLEDColor;
		if (isBlinking) {
			// Calculate current power
			double currentPhase = ((millis() % BLINK_PERIOD) / (double)BLINK_PERIOD) * 2 * M_PI;
			double currentPower = (exp(sin(currentPhase)) - (1 / M_E)) * (1 / (M_E - (1 / M_E)));

			// Calculate new color
			calculatedLEDColor.r = (unsigned int)(currentLEDColor.r * currentPower);
			calculatedLEDColor.g = (unsigned int)(currentLEDColor.g * currentPower);
			calculatedLEDColor.b = (unsigned int)(currentLEDColor.b * currentPower);
		}
		else {
			calculatedLEDColor = currentLEDColor;
		}

		// Update LEDs
		setColorLED(calculatedLEDColor);		// ALSO MANAGED EXTERNALLY
		// setAppLED(currentAppLEDStatus);		(MANAGED EXTERNALLY)
		// setWlanLED(currentWlanLEDStatus);	NOT SUPPORTED
	}

	Log_Debug("STATUSLEDS: Thread is stopped.\n");
}

pthread_t LEDThread;

void startLEDThread() {
	Log_Debug("STATUSLEDS: Starting LED thread.\n");

	statusLEDThreadStatus = 0;

	pthread_create(&LEDThread, NULL, statusLEDThread, NULL);

	return;
}

void stopLEDThread() {
	Log_Debug("STATUSLEDS: Stopping LED thread.\n");

	statusLEDThreadStatus = 1;

	return;
}

void setPWMFromValue(int value, PwmState* s) {
	if (value >= 0) {
		s->enabled = true;
		s->period_nsec = 10000;
		s->dutyCycle_nsec = value > 1000 ? 10000 : 10 * value;
		s->polarity = PWM_Polarity_Inversed;
	}
}

void setColorLED(color_t color) {

	PwmState redPWM;
	PwmState greenPWM;
	PwmState bluePWM;

	setPWMFromValue(color.r, &redPWM);
	setPWMFromValue(color.g, &greenPWM);
	setPWMFromValue(color.b, &bluePWM);

	if (colorLEDfd > 0) {
		PWM_Apply(colorLEDfd, redLEDChannel, &redPWM);
		PWM_Apply(colorLEDfd, greenLEDchannel, &greenPWM);
		PWM_Apply(colorLEDfd, blueLEDChannel, &bluePWM);
	}

}

void setAppLED(int power) {

	PwmState s;
	setPWMFromValue(power, &s);

	if (statusLEDsfd > 0)
		PWM_Apply(statusLEDsfd, appLEDChannel, &s);
}

void setWlanLED(int power) {

	PwmState s;
	setPWMFromValue(power, &s);

	if (statusLEDsfd > 0)
		PWM_Apply(statusLEDsfd, wlanLEDChannel, &s);
}

int initializeLEDControllers(PWM_ControllerId statusLEDsController, PWM_ControllerId colorLEDController, PWM_ChannelId wifiChannel, PWM_ChannelId appChannel, PWM_ChannelId redChannel, PWM_ChannelId greenChannel, PWM_ChannelId blueChannel) {
	if (statusLEDsfd <= 0) {
		// Try to initialize the status LEDs controller
		if ((statusLEDsfd = PWM_Open(statusLEDsController)) == -1) {
			Log_Debug("STATUSLEDS: Unable to initialize status LEDs controller (%d) due to the following error: \"%s\".\n", statusLEDsController, strerror(errno));
			return -1;
		}
	}

	if (colorLEDfd <= 0) {
		// Try to initialize the color LED controller
		if ((colorLEDfd = PWM_Open(colorLEDController)) == -1) {
			Log_Debug("STATUSLEDS: Unable to initialize color LED controller (%d) due to the following error: \"%s\".\n", statusLEDsController, strerror(errno));
			return -1;
		}
	}

	// Save channel IDs
	wlanLEDChannel = wifiChannel;
	appLEDChannel = appChannel;
	redLEDChannel = redChannel;
	greenLEDchannel = greenChannel;
	blueLEDChannel = blueChannel;

	return 0;
}
