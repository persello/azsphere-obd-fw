#pragma once

typedef enum {
	Gasoline = 0x01,
	Methanol,
	Ethanol,
	Diesel,
	LPG,
	CNG,
	Propane,
	Electric,
	Bifuel_Gasoline,
	Bifuel_Methanol,
	Bifuel_Ethanol,
	Bifuel_LPG,
	Bifuel_CNG,
	Bifuel_Propane,
	Bifuel_Electric,
	Bifuel_Gas_Electric,
	Hybrid_Gasoline,
	Hybrid_Ethanol,
	Hybrid_Diesel,
	Hybrid_Electric,
	Hybrid_Mixed_Fuel,
	Hybrid_Regenerative
} FuelType;

typedef struct {
	char* VIN;
	char supportedMode1PIDs[0x61];
	FuelType fuelType;
	char (*DTCs)[5];
	int initialized;

	int lastSpeed;
	double lastRPM;
	int lastEngineCoolantTemp;
	double lastAirFlow;
	double lastThrottlePosition;
} VehicleProperties;