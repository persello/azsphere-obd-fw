#pragma once

#include <stdbool.h>

#include "lib/fatfs/ff.h"

FATFS SD;
FIL currentFile;

/// <summary> Initializes the connected SD Card. </summary>
int startSDThread(void);

/// <summary> Logs a string to the SD Card. Date, time and formatting are automatically added. </summary>
int logToSD(char* data);

/// <summary> Stops the SD Card thread. </summary>
int stopSDThread(void);