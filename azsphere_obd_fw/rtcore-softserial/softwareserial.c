#include "softwareserial.h"

#include "mt3620-gpio.h"
#include "mt3620-timer-poll.h"

#include <errno.h>
#include <string.h>

// Debug pin
const int pulse = 0;
const int CLK_CORRECTION = 18; // [us] Measured at 9600


int initializeSS(SoftwareSerial* s, int tx, int rx)
{

	// Save pin numbers
	s->tx = tx;
	s->rx = rx;

	// Init GPIOs

	static const GpioBlock pwm0 = {
		.baseAddr = 0x38010000,.type = GpioBlock_PWM,.firstPin = 0,.pinCount = 4 };

	static const GpioBlock adc0 = {
		.baseAddr = 0x38110000,.type = GpioBlock_ADC,.firstPin = 40,.pinCount = 4 };

	Mt3620_Gpio_AddBlock(&pwm0);
	Mt3620_Gpio_AddBlock(&adc0);

	// TX
	Mt3620_Gpio_ConfigurePinForOutput(tx);
	Mt3620_Gpio_Write(tx, 1);

	// RX
	Mt3620_Gpio_ConfigurePinForInput(rx);

	// Pulse
	Mt3620_Gpio_ConfigurePinForOutput(pulse);
	Mt3620_Gpio_Write(pulse, 0);

	// Set the default baudrate
	s->br = 9600;

	// Initialize other variables
	s->currentBitRead = 0;
	s->currentBitWrite = 0;
	s->lastCharRead = 0;
	s->lastCharReadProcessed = 0;
	s->nextCharWrite = 0;
	s->nextCharWriteProcessed = 0;

	// Start timer with period = 1000000us/baudrate
	// Gpt3_StartUs(1000000 / s->br);

	return 0;
}

int setSSBaudRate(SoftwareSerial* s, int br)
{

	s->br = br;
	return 0;
}

int updateSS(SoftwareSerial* s)
{

	// Get GPT3 timer status
	int timerStatus = Gpt3_Check();

	// Restart timer if expired
	if (!timerStatus) {
		Gpt3_StartUs((1000000 / s->br) - CLK_CORRECTION);
	}

	// Read (no return, otherwise it won't write)

	// Read RX line
	int reading = 0;
	if (Mt3620_Gpio_Read(s->rx, &reading) < 0) {

		// Fatal error. No need to write anything.
		return -1;
	}

	// Waiting for start bit and one delay bit (due to timer's first clock)
	if (s->currentBitRead == 0) {

		// Falling edge
		if (reading == 0 && s->lastGPIOStatus == 1) {
			// For two times
			s->currentBitRead++;

			// Sync
			Gpt3_StartUs(1000000 / s->br);
		}

		s->lastGPIOStatus = reading;

	}
	// After the start bit, start reading data
	else {

		// Time to read?
		if (!timerStatus) {

			// Debug sampling rate
			Mt3620_Gpio_Write(pulse, 1);

			// Push data to current character
			s->currentCharRead |= (reading << (s->currentBitRead - 1));

			// Debug sampling rate
			Mt3620_Gpio_Write(pulse, 0);

			s->currentBitRead++;

			// Roll back to zero, ignore stop, save character
			if (s->currentBitRead > 8) {
				// Output
				s->lastCharRead = s->currentCharRead;
				s->lastCharReadProcessed = 0;
				s->currentCharRead = 0x00;
				s->currentBitRead = 0;
			}
		}
	}


	// Write

	// Time to write?
	//if (!timerStatus) {
	//	int value;

	//	// Start bit
	//	if (s->currentBitWrite == 0) {

	//		// Loads a character, if the buffer is empty, stop.
	//		if (s->nextCharWriteProcessed) {
	//			s->currentCharWrite = s->nextCharWrite;
	//			s->nextCharWriteProcessed = 0;
	//		}
	//		else {
	//			return -1;
	//		}

	//		value = 0;
	//	}
	//	// Stop bits (2 for safety)
	//	else if (s->currentBitWrite == 9 | s->currentBitWrite == 10) {
	//		value = 1;
	//	}
	//	// Data
	//	else {
	//		value = (s->currentCharWrite & (1 << (s->currentBitWrite - 1))) > 0;
	//	}

	//	// Write current bit on pin
	//	Mt3620_Gpio_Write(s->tx, value);

	//	// Increment bit index (0 to 10)
	//	s->currentBitWrite++;
	//	if (s->currentBitWrite == 11) s->currentBitWrite = 0;

	//}

	return 0;
}
