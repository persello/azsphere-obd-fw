#pragma once

#include <applibs/pwm.h>

/// <summary> Sets the RGB led color. </summary>
/// <param name="red"> The power applied to the red channel, ranging from 1 to 1000. </summary>
/// <param name="green"> The power applied to the green channel, ranging from 1 to 1000. </summary>
/// <param name="blue"> The power applied to the blue channel, ranging from 1 to 1000. </summary>
void setColorLED(int red, int green, int blue);

/// <summary> Sets the APP led intensity. </summary>
/// <param name="power"> The power applied to the LED, ranging from 1 to 1000. </summary>
void setAppLED(int power);

/// <summary> Sets the WLAN led intensity. </summary>
/// <param name="power"> The power applied to the LED, ranging from 1 to 1000. </summary>
void setWlanLED(int power);

/// <summary> Initializes the PWM channels for driving the LEDs. </summary>
int initializeLEDChannels(PWM_ControllerId red, PWM_ControllerId green, PWM_ControllerId);