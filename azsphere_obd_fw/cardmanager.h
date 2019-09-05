#pragma once

#include <stdbool.h>

#include "lib/fatfs/ff.h"

FATFS SD;
FIL currentFile;
bool SDmounted;

/// <summary> Initializes the connected SD Card. </summary>
void startSDThread(void);

/// <summary> Logs a string to the SD Card. Date, time and formatting are automatically added. </summary>
void logToSD(char* _data);

/// <summary> Stops the SD Card thread. </summary>
void stopSDThread(void);