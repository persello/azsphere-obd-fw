#pragma once

#include "softwareserial.h"

SoftwareSerial GPSSerial;

int startGPSThreads();

int stopGPSThreads();