#pragma once

#include <applibs/pwm.h>

/// <summary> A simple RGB color struct. </summary>
typedef struct color {
	unsigned int r;
	unsigned int g;
	unsigned int b;
} color_t;

void startLEDThread();

void stopLEDThread();

/// <summary> Sets the RGB led color. </summary>
/// <param name="red"> The power applied to the red channel, ranging from 1 to 1000. </param>
/// <param name="green"> The power applied to the green channel, ranging from 1 to 1000. </param>
/// <param name="blue"> The power applied to the blue channel, ranging from 1 to 1000. </param>
void setColorLED(color_t color);

/// <summary> Sets the APP led intensity. </summary>
/// <param name="power"> The power applied to the LED, ranging from 1 to 1000. </summary>
void setAppLED(int power);

/// <summary> Sets the WLAN led intensity. </summary>
/// <param name="power"> The power applied to the LED, ranging from 1 to 1000. </summary>
void setWlanLED(int power);

/// <summary> Initializes the PWM controllers for driving the LEDs. </summary>
int initializeLEDControllers(PWM_ControllerId statusLEDsController, PWM_ControllerId colorLEDController, PWM_ChannelId wifiChannel, PWM_ChannelId appChannel, PWM_ChannelId redChannel, PWM_ChannelId greenChannel, PWM_ChannelId blueChannel);
