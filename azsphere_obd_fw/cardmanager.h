#pragma once

#include <stdbool.h>

#include "lib/fatfs/ff.h"

FATFS SD;
FIL currentFile;

int initializeSD(void);

int logToSD(char* data);

int stopSD(void);