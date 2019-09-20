#include "obdserial.h"

#include "cardmanager.h"
#include "config.h"
#include "obdpids.h"

#include "lib/circularbuffer/buffer.h"
#include "lib/timer/Inc/Public/timer.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <applibs/log.h>

buffer_t RXBuffer;
buffer_t TXBuffer;

pthread_t OBDThread;

int OBDThreadTerminationRequest;

void OBDModule_Init(OBDModule* _module) {
	_module->baudRate = -1;
	_module->connected = 0;
	_module->initialized = 1;
	// _module->uartfd = 0; NO

	// It is requested to initialize an UART_Config struct
	UART_InitConfig(&_module->uartConfig);
}

// Sends data from TX buffer
int UART_Send() {
	char r;
	int count = 0;

	// Read from buffer and send to UART until TX buffer is empty
	while (getCharBuffer(&TXBuffer, &r) != -1) {
		int result = write(OBD.uartfd, &r, 1);
		if (result == -1) {
			Log_Debug("OBDSERIAL: Error while sending data to OBD module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			return -1;
		}
		else {
			count += result;
		}
	}
	return count;
}

// Receives data and puts into RX buffer
int UART_Receive() {
	char r;
	int count = 0;
	int fdresult;

	Timer_On(TIMER_OBD_UART_DURATION, TIMER_OBD_UART);

	// Read from UART and send to RX buffer until UART buffer is empty
	do {

		// Read from UART
		fdresult = read(OBD.uartfd, &r, 1);

		// Check result
		if (fdresult == -1) {
			Log_Debug("OBDSERIAL: Error while reading data from OBD module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			return -1;

			// There is data
		}
		else if (fdresult > 0) {

			// Put in buffer
			int bufresult = putCharBuffer(&RXBuffer, r);

			// Check result
			if (bufresult == -1) {
				Log_Debug("OBDSERIAL: Error while reading data from OBD module. Buffer is full.\n");
				return -1;
			}
			else {
				count++;
			}
		}

		// Exit when UART RX is empty and last char is '>', when timer expires or a lock is requested
	} while ((fdresult > 0 || r != '>') && Timer_Status(TIMER_OBD_UART) && !OBDThreadLock);

	Timer_Off(TIMER_OBD_UART);

	if (r != '>') {
		Log_Debug("OBDSERIAL: Error while reading data. The module did not answer with '>' within %d milliseconds.\n", TIMER_OBD_UART_DURATION);

		// Try to reconnect to the module
		OBD.connected = 0;

		car.initialized = 0;
		logToSD("CARECUINIT\t0");

		return -1;
	}

	return count;
}

int getLastReceivedMessage(char** _output) {

	int length = 0;

	// Create a temporary buffer
	char* buf = malloc(1024 * sizeof(char));
	memset(buf, 0, 1024);

	char r;
	char previous = 0x00;

	while (getCharBuffer(&RXBuffer, &r) != -1) {

		// Discard everything except last message
		if (previous == '>') {
			memset(buf, 0, 1024);
			length = 0;
		}

		buf[length] = r;
		length++;
		previous = r;
	}

	// Length has to be at least 1 for a valid message
	if (length > 0) {

		// Allocate the needed size (length + terminator ---but no '>'---)
		*_output = malloc((length) * sizeof(char));
		memset(*_output, 0, length);

		// Copy the result to the final buffer
		strncpy(*_output, buf, length - 1);

	}

	// Big temporary buffer not needed anymore
	free(buf);

	return length;
}

int sendOBDRequest(OBDRequest _request) {

	// Output to device (2x (1 byte in ASCII (2 bytes)) + \r + NUL)
	char* OBDout = malloc(6 * sizeof(char));
	memset(OBDout, 0, 6 * sizeof(char));

	// Input (8 + 3x length)
	// 8 = 5 are echoed response (mode (2), space (1), command (2)), 2 are ending space, \r and NUL
	// 3x length is 1 byte in ASCII (2 bytes) for each byte in the answer and a space
	int anslen = (8 + (3 * _request.length)) * sizeof(char);
	char* OBDin = malloc(anslen);
	memset(OBDin, 0, anslen);

	// Sends request mode and PID in hexadecimal (4 bytes), followed by a \r
	sprintf(OBDout, "%02x%02x\r", _request.mode, _request.pid);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(OBDout); i++) {
		if (putCharBuffer(&TXBuffer, OBDout[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put OBD request in the TX buffer: buffer is full.\n");
			free(OBDout);
			free(OBDin);
			return -1;
		}
	}

	free(OBDout);
	free(OBDin);

	if (UART_Send() == -1) return -1;

	return UART_Receive();
}

int sendATCommand(char* _command) {

	// Output to device (AT + command + \r + NUL)
	char* ATout = malloc((4 + strlen(_command)) * sizeof(char));
	memset(ATout, 0, (4 + strlen(_command)) * sizeof(char));

	// Creating the AT message
	sprintf(ATout, "AT%s\r", _command);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(ATout); i++) {
		if (putCharBuffer(&TXBuffer, ATout[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put AT command in the TX buffer: buffer is full.\n");
			// free(ATout);
			// return -1; // Otherwise, how could the buffer get emptied?
		}
	}

	free(ATout);

	if (UART_Send() == -1) return -1;
	return UART_Receive();
}

int sendSTCommand(char* _command) {

	// Output to device (ST + command + \r + NUL)
	char* STout = malloc((4 + strlen(_command)) * sizeof(char));
	memset(STout, 0, (4 + strlen(_command)) * sizeof(char));

	// Creating the ST message
	sprintf(STout, "ST%s\r", _command);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(STout); i++) {
		if (putCharBuffer(&TXBuffer, STout[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put AT command in the TX buffer: buffer is full.\n");
			free(STout);
			return -1;
		}
	}

	free(STout);

	if (UART_Send() == -1) return -1;
	return UART_Receive();
}

int initOBDComm(UART_Id _id, OBDModule* _module, long _initialBaudRate) {

	// Initialize the OBDModule struct and its UART_Config if not already done
	if (!_module->initialized)
		OBDModule_Init(_module);

	// Set all the parameters
	_module->uartConfig.baudRate = _initialBaudRate;
	_module->uartConfig.blockingMode = UART_BlockingMode_NonBlocking;
	_module->uartConfig.dataBits = UART_DataBits_Eight;
	_module->uartConfig.flowControl = UART_FlowControl_None;
	_module->uartConfig.parity = UART_Parity_None;
	_module->uartConfig.stopBits = UART_StopBits_One;

	// Create UART file descriptor only the first time
	if (_module->uartfd == 0) {
		_module->uartfd = UART_Open(_id, &_module->uartConfig);

		if (_module->uartfd == -1) {
			Log_Debug("OBDSERIAL: UART %d initialization failed. Details: \"%s\".\n", _id, strerror(errno));
			return -1;
		}
		else {
			Log_Debug("OBDSERIAL: UART %d initialized at %d bauds with fd=%d.\n", _id, _module->uartConfig.baudRate, _module->uartfd);
		}
	}

	// Initialize buffers
	if (!RXBuffer.size)
		initCircBuffer(&RXBuffer, 512);

	if(!TXBuffer.size)
		initCircBuffer(&TXBuffer, 512);

	// Reset the device
	if (sendATCommand("Z") == -1) {
		return -1;
	}

	// Echo off
	if (sendATCommand("E0") == -1) {
		return -1;
	}

	// Automatic protocol detection
	if (sendATCommand("SP A0") == -1) {
		return -1;
	}

	// TODO: Raise speed?
	// sendSTCommand("BR2000000"); // Maximum MT3620 speed

	// Finally, confirm the connection
	_module->connected = 1;
	_module->baudRate = _initialBaudRate;

	Log_Debug("OBDSERIAL: Module initialized.\n");

	return 0;
}

int initializeVehicleProperties(VehicleProperties* _car) {

	// 17 VIN + terminator
	_car->VIN = malloc(18);
	memset(_car->VIN, 0, 18);

	_car->initialized = 0;

	// -40 to 215 celsius, integer
	_car->lastEngineCoolantTemp = 0;

	// 0 to 16383.75, double precision
	_car->lastRPM = 0.00;

	// 0 to 255 km/h, integer
	_car->lastSpeed = 0;

	// 0 to 655.35 g/s, double precision
	_car->lastAirFlow = 0.00;

	// 0 to 100%, double precision
	_car->lastThrottlePosition = 0.00;
}

void* OBDThreadMain(void* _param) {
	while (!OBDThreadTerminationRequest) {
		if (!OBD.connected && !OBDThreadLock) {

			// Disconnected
			initOBDComm(OBD_SERIAL, &OBD, OBD_INITIAL_BR);
		}
		else if (!OBDThreadLock) {

			// Car not initialized
			if (!car.initialized) {

				// Initialize the ECU and get basic info

				// Initialize the struct
				initializeVehicleProperties(&car);

				// Get the 01-20 supported PIDs
				OBDRequest r = OBD_SUPPORTED_01_20;

				if (sendOBDRequest(r) == -1) {
					Log_Debug("OBDSERIAL: Error while asking 01-20 supported PIDs.\n");

					// Let's reset the module
					OBD.connected = false;
				}
				else {

					// Some cars (VW Golf Mk. 3/4) won't answer to the first message if continuously pinged.
					sleep(3);

					char* received;
					getLastReceivedMessage(&received);

					// Receives n bytes of answer and 2 of confirmation (written in ASCII), spacing between each one and trailing \n\n
					if (strlen(received) == ((2 + r.length) * 3 + 2)) {

						// Length is correct, check header
						char* expectedHeader = malloc(6);
						sprintf(expectedHeader, "%02X %02X ", r.mode + 0x40, r.pid);

						// Check if we received the correct answer
						if (strncmp(received, expectedHeader, 6) == 0) {
							Log_Debug("OBDSERIAL: 01-20 supported PIDs received.\n");

							char param[6] = {};
							char* pointer;

							param[0] = strtol(received, &pointer, 16);
							param[1] = strtol(pointer, &pointer, 16);
							param[2] = strtol(pointer, &pointer, 16);
							param[3] = strtol(pointer, &pointer, 16);
							param[4] = strtol(pointer, &pointer, 16);
							param[5] = strtol(pointer, &pointer, 16);

							int currentPID = 1;
							for (int i = 2; i < 6; i++) {
								for (int j = 0; j < 8; j++) {
									car.supportedMode1PIDs[currentPID] = ((1 << (7 - j)) & param[i]) > 0;
									if (car.supportedMode1PIDs[currentPID]) {
										Log_Debug("OBDSERIAL: Car supports PID number %02x.\n", currentPID);
									}
									currentPID++;
								}
							}

							car.initialized = 1;

							// Logs the initialization
							logToSD("CARECUINIT\t1");

							Log_Debug("OBDSERIAL: Car initialized.\n");

						}
						// Wrong header
						else {
							// Let's reset the module
							// OBD.connected = 0;
						}
					}
					// Wrong length
					else {

						// Let's reset the module
						// OBD.connected = 0;
					}

					free(received);

				}
				// ECU initialized, poll the parameters
			}
			else {

				// Poll battery voltage
				sendATCommand("RV");
				char* voltagestr = malloc(20);
				getLastReceivedMessage(&voltagestr);

				// We received back voltage information

				// TODO: Fix this. ATRV does not echo ATRV back.
				if (/*strncmp(&voltagestr, "ATRV\r", 5) == 0*/ 1) {

					// Convert from string

					// TODO: This should be set to voltagestr + 5 when you receive ATRV\r before the voltage reading
					float voltage = strtof(voltagestr, NULL);
					Log_Debug("OBDSERIAL: Battery voltage is %f volts.\n", voltage);

					char* result = malloc(30);

					// Log to SD card
					sprintf(result, "BATVOLTAGE\t%f", voltage);
					logToSD(result);

					free(result);
				}
				else {
					Log_Debug("OBDSERIAL: Could not get battery voltage information. Answer was \"%s\".\n", voltagestr);
				}

				free(voltagestr);


				// Poll all the interesting parameters
				for (int i = 0; i < PARAMETER_POLL_COUNT; i++) {

					// Is its PID supported by this car?
					if (carParametersToPoll[i].mode == 1 && car.supportedMode1PIDs[carParametersToPoll[i].pid]) {

						// Supported: Send message
						if (sendOBDRequest(carParametersToPoll[i]) == -1) {

							// Send failed
							Log_Debug("OBDSERIAL: Failed to get parameter %02x from the car's ECU.\n", carParametersToPoll[i].pid);

							// Reinitialize the ECU next time
							car.initialized = 0;

							// Logs the deinitialization
							logToSD("CARECUINIT\t0");

							break;
						}

						// Read the answer next (reads all the messages and processes the current one, if found)
						char* received;
						while (getLastReceivedMessage(&received)) {

							// Receives n bytes of answer and 2 of confirmation (written in ASCII), spacing between each one and trailing \n\n
							if (strlen(received) == ((2 + carParametersToPoll[i].length) * 3 + 2)) {


								// Length is correct, check header against last sent message's parameters
								char* expectedHeader = malloc(6);
								sprintf(expectedHeader, "%02X %02X ", carParametersToPoll[i].mode + 0x40, carParametersToPoll[i].pid);

								// Check if we received the correct answer
								if (strncmp(received, expectedHeader, 6) == 0) {
									Log_Debug("OBDSERIAL: PID %02x received.\n", carParametersToPoll[i].pid);

									char param[2 + carParametersToPoll[i].length];
									char* pointer;

									// Load the received bytes into an array
									param[0] = strtol(received, &pointer, 16);
									for (int j = 1; j < (2 + carParametersToPoll[i].length); j++) {
										param[j] = strtol(pointer, &pointer, 16);
									}

									char* result = malloc(30);

									// Interpreting received data
									switch (param[1]) {

										// param[1] is the PID of the current received message

										// Engine coolant temperature
									case 0x05:

										Log_Debug("OBDSERIAL: Decoding engine coolant temperature.\n");

										// Celsius degrees
										int temperature = param[2] - 40;
										car.lastEngineCoolantTemp = temperature;

										// Write to SD
										sprintf(result, "ENGINETEMP\t%d", temperature);
										logToSD(result);

										Log_Debug("OBDSERIAL: Engine coolant temperature is %d C.\n", temperature);

										break;

										// Engine RPM
									case 0x0C:

										Log_Debug("OBDSERIAL: Decoding engine RPM.\n");

										double rpm = ((256 * param[2]) + param[3]) / 4.00;
										car.lastRPM = rpm;

										// Write to SD
										sprintf(result, "VEHICLERPM\t%f", rpm);
										logToSD(result);

										Log_Debug("OBDSERIAL: RPM count is %f RPM.\n", rpm);

										break;

										// Vehicle speed
									case 0x0D:

										Log_Debug("OBDSERIAL: Decoding vehicle speed.\n");

										int speed = param[2];
										car.lastSpeed = speed;

										// Write to SD
										sprintf(result, "VEHICSPEED\t%d", speed);
										logToSD(result);

										Log_Debug("OBDSERIAL: Speed is %d km/h.\n", speed);

										break;

										// Intake air flow rate
									case 0x10:

										Log_Debug("OBDSERIAL: Decoding intake air flow.\n");

										double maf = ((256 * param[2]) + param[3]) / 100.00;
										car.lastAirFlow = maf;

										// Write to SD
										sprintf(result, "ENGAIRFLOW\t%f", maf);
										logToSD(result);

										Log_Debug("OBDSERIAL: Intake air flow is %f g/s.\n", maf);

										break;

										// Throttle position
									case 0x11:

										Log_Debug("OBDSERIAL: Decoding throttle position.\n");

										double throttle = param[2] * 100 / 255.0;
										car.lastThrottlePosition = throttle;

										// Write to SD
										sprintf(result, "ENTHROTTLE\t%f", throttle);
										logToSD(result);

										Log_Debug("OBDSERIAL: Throttle position is %f.\n", throttle);

										break;
									}

									free(result);

								}
								// Incorrect header: need to reinitialize car
								else {
									car.initialized = 0;

									// Protocol close
									sendATCommand("PC");

									// Reinitialize module
									OBD.connected = 0;

									// Logs the deinitialization
									logToSD("CARECUINIT\t1");
								}
							}
							// Incorrect length: need to reinitialize car (module resets when car initialization fails)
							else {
								car.initialized = 0;

								// Protocol close
								sendATCommand("PC");

								// Reinitialize module
								OBD.connected = 0;

								// Logs the deinitialization
								logToSD("CARECUINIT\t1");
							}
						}

						free(received);

					}
				}
			}
		}
	}


	car.initialized = 0;
	logToSD("CARECUINIT\t0");

	OBD.connected = 0;

	Log_Debug("OBDSERIAL: OBD thread stopped.\n");
}


void startOBDThread() {

	// Starts the thread
	Log_Debug("OBDSERIAL: Starting OBD thread.\n");

	// Let it run
	OBDThreadTerminationRequest = 0;
	pthread_create(&OBDThread, NULL, OBDThreadMain, NULL);
}

void stopOBDThread() {

	Log_Debug("OBDSERIAL: Stopping OBD thread.\n");

	// Exit from loop
	OBDThreadTerminationRequest = 1;
}