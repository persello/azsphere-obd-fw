#pragma once

typedef struct {
	float lat;
	float lon;
	float speed;
	float height;
	float course;

	int newData;
} GPSData;

GPSData realTimeGPSData;

int startGPSThread();

int stopGPSThread();