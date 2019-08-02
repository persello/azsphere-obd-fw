#pragma once

// MODE 1

// #define OBD_SUPPORTED_01_20				{0x01, 0x00, 0x04}
// #define OBD_STATUS_SINCE_DTC_CLEAR		{0x01, 0x01, 0x04}
// #define OBD_FREEZE_DTC					{0x01, 0x02, 0x08}
// #define OBD_FUEL_SYSTEM_STATUS			{0x01, 0x03, 0x02}
// #define OBD_CALCULATED_ENGINE_LOAD		{0x01, 0x04, 0x01}
// #define OBD_ENGINE_COOLANT_TEMPERATURE	{0x01, 0x05, 0x01}
// #define OBD_SHORT_TERM_FUEL_PERC_TRIM_1	{0x01, 0x06, 0x01}
// #define OBD_LONG_TERM_FUEL_PERC_TRIM_1	{0x01, 0x07, 0x01}
// #define OBD_SHORT_TERM_FUEL_PERC_TRIM_2 {0x01, 0x08, 0x01}
// #define OBD_LONG_TERM_FUEL_PERC_TRIM_2	{0x01, 0x09, 0x01}
// #define OBD_FUEL_PRESSURE				{0x01, 0x0A, 0x01}
// #define OBD_INTAKE_MANIFOLD_PRESSURE	{0x01, 0x0B, 0x01}
// #define OBD_ENGINE_RPM					{0x01, 0x0C, 0x02}
// #define OBD_VEHICLE_SPEED				{0x01, 0x0D, 0x01}
// #define OBD_TIMING_ADVANCE				{0x01, 0x0E, 0x01}
// #define OBD_INTAKE_AIR_TEMPERATURE		{0x01, 0x0F, 0x01}
// #define OBD_MAF_AIR_FLOW_RATE			{0x01, 0x10, 0x02}
// #define OBD_THROTTLE_POSITION			{0x01, 0x11, 0x01}
// #define OBD_COMM_SECONDARY_AIR_STATUS	{0x01, 0x12, 0x01}
// #define OBD_OXYGEN_SENSORS_PRESENT_1	{0x01, 0x13, 0x01}
// #define OBD_B1S1_OXY_AND_S_FUEL_TRIM	{0x01, 0x14, 0x02}
// #define OBD_B1S2_OXY_AND_S_FUEL_TRIM	{0x01, 0x15, 0x02}
// #define OBD_B1S3_OXY_AND_S_FUEL_TRIM	{0x01, 0x16, 0x02}
// #define OBD_B1S4_OXY_AND_S_FUEL_TRIM	{0x01, 0x17, 0x02}
// #define OBD_B2S1_OXY_AND_S_FUEL_TRIM	{0x01, 0x18, 0x02}
// #define OBD_B2S2_OXY_AND_S_FUEL_TRIM	{0x01, 0x19, 0x02}
// #define OBD_B2S3_OXY_AND_S_FUEL_TRIM	{0x01, 0x1A, 0x02}
// #define OBD_B2S4_OXY_AND_S_FUEL_TRIM	{0x01, 0x1B, 0x02}
// #define OBD_STANDARD_CONFORM			{0x01, 0x1C, 0x01}
// #define OBD_OXYGEN_SENSORS_PRESENT_2	{0x01, 0x1D, 0x01}
// #define OBD_AUXILIARY_INPUT_STATUS		{0x01, 0x1E, 0x01}
// #define OBD_RUN_TIME_SINCE_ENG_START	{0x01, 0x1F, 0x02}
// #define OBD_SUPPORTED_21_40				{0x01, 0x20, 0x04}
// #define OBD_DISTANCE_TRAVELED_MIL_ON	{0x01, 0x21, 0x02}
// #define OBD_FUEL_RAIL_PRESSURE_REL		{0x01, 0x22, 0x02}
// #define OBD_FUEL_RAIL_PRESSURE_DIESEL	{0x01, 0x23, 0x02}
// #define OBD_O2S1_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x24, 0x04}
// #define OBD_O2S2_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x25, 0x04}
// #define OBD_O2S3_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x26, 0x04}
// #define OBD_O2S4_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x27, 0x04}
// #define OBD_O2S5_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x28, 0x04}
// #define OBD_O2S6_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x29, 0x04}
// #define OBD_O2S7_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x2A, 0x04}
// #define OBD_O2S8_WR_LAMBDA_EQ_RATIO_V	{0x01, 0x2B, 0x04}
// #define OBD_COMMANDED_EGR				{0x01, 0x2C, 0x01}
// #define OBD_EGR_ERROR					{0x01, 0x2D, 0x01}
// #define OBD_COMMANDED_EVAP_PURGE		{0x01, 0x2E, 0x01}	
// #define OBD_FUEL_LEVEL_INPUT			{0x01, 0x2F, 0x01}
// #define OBD_WARMUPS_SINCE_CODES_CLEAR	{0x01, 0x30, 0x01}
// #define OBD_DISTANCE_SINCE_CODES_CLEAR	{0x01, 0x31, 0x02}
// #define OBD_EVAP_SYS_VAPOR_PRESSURE		{0x01, 0x32, 0x02}
// #define OBD_BAROMETRIC_PRESSURE			{0x01, 0x33, 0x01}
// #define OBD_O2S1_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x34, 0x04}
// #define OBD_O2S2_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x35, 0x04}
// #define OBD_O2S3_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x36, 0x04}
// #define OBD_O2S4_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x37, 0x04}
// #define OBD_O2S5_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x38, 0x04}
// #define OBD_O2S6_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x39, 0x04}
// #define OBD_O2S7_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x3A, 0x04}
// #define OBD_O2S8_WR_LAMBDA_EQ_RATIO_I	{0x01, 0x3B, 0x04}
// #define OBD_CATALYST_TEMP_1_1			{0x01, 0x3C, 0x02}
// #define OBD_CATALYST_TEMP_2_1			{0x01, 0x3D, 0x02}
// #define OBD_CATALYST_TEMP_1_2			{0x01, 0x3E, 0x02}
// #define OBD_CATALYST_TEMP_2_2			{0x01, 0x3F, 0x02}
// #define OBD_SUPPORTED_41_60				{0x01, 0x40, 0x04}
// #define OBD_MON_STATUS_DRIVE_CYCLE		{0x01, 0x41, 0x04}
// #define OBD_CONTROL_MODULE_VOLTAGE		{0x01, 0x42, 0x02}
// #define OBD_ABSOLUTE_LOAD_VALUE			{0x01, 0x43, 0x02}
// #define OBD_COMMAND_EQUIV_RATIO			{0x01, 0x44, 0x02}
// #define OBD_RELATIVE_THROTTLE_POSITION	{0x01, 0x45, 0x01}
// #define OBD_AMBIENT_AIR_TEMPERATURE		{0x01, 0x46, 0x01}
// #define OBD_ABS_THROTTLE_POSITION_B		{0x01, 0x47, 0x01}
// #define OBD_ABS_THROTTLE_POSITION_C		{0x01, 0x48, 0x01}
// #define OBD_ABS_THROTTLE_POSITION_D		{0x01, 0x49, 0x01}
// #define OBD_ABS_THROTTLE_POSITION_E		{0x01, 0x4A, 0x01}
// #define OBD_ABS_THROTTLE_POSITION_F		{0x01, 0x4B, 0x01}
// #define OBD_COMMANDED_THROTTLE_ACTUATOR	{0x01, 0x4C, 0x01}
// #define OBD_TIME_RUN_MIL_ON				{0x01, 0x4D, 0x02}
// #define OBD_TIME_TROUBLE_CODES_CLEARED	{0x01, 0x4E, 0x02}
// #define OBD_FUEL_TYPE					{0x01, 0x51, 0x01}
// #define OBD_ETHANOL_FUEL_PERCENTAGE		{0x01, 0x52, 0x01}
// #define OBD_ABS_EVAP_SYS_VAPOR_PRESSURE	{0x01, 0x53, 0x02}


// MODE 2

// #define OBD_LAST_DTC_SUPPORTED_01_20			{0x02, 0x00, 0x04}
// #define OBD_LAST_DTC_STATUS_SINCE_DTC_CLEAR		{0x02, 0x01, 0x04}
// #define OBD_FREEZE_FRAME_TROUBLE_CODE			{0x02, 0x02, 0x02}
// #define OBD_LAST_DTC_FUEL_SYSTEM_STATUS			{0x02, 0x03, 0x02}
// #define OBD_LAST_DTC_CALCULATED_ENGINE_LOAD		{0x02, 0x04, 0x01}
// #define OBD_LAST_DTC_ENGINE_COOLANT_TEMPERATURE	{0x02, 0x05, 0x01}
// #define OBD_LAST_DTC_SHORT_TERM_FUEL_PERC_TRIM_1{0x02, 0x06, 0x01}
// #define OBD_LAST_DTC_LONG_TERM_FUEL_PERC_TRIM_1	{0x02, 0x07, 0x01}
// #define OBD_LAST_DTC_SHORT_TERM_FUEL_PERC_TRIM_2{0x02, 0x08, 0x01}
// #define OBD_LAST_DTC_LONG_TERM_FUEL_PERC_TRIM_2	{0x02, 0x09, 0x01}
// #define OBD_LAST_DTC_FUEL_PRESSURE				{0x02, 0x0A, 0x01}
// #define OBD_LAST_DTC_INTAKE_MANIFOLD_PRESSURE	{0x02, 0x0B, 0x01}
// #define OBD_LAST_DTC_ENGINE_RPM					{0x02, 0x0C, 0x02}
// #define OBD_LAST_DTC_VEHICLE_SPEED				{0x02, 0x0D, 0x01}
// #define OBD_LAST_DTC_TIMING_ADVANCE				{0x02, 0x0E, 0x01}
// #define OBD_LAST_DTC_INTAKE_AIR_TEMPERATURE		{0x02, 0x0F, 0x01}
// #define OBD_LAST_DTC_MAF_AIR_FLOW_RATE			{0x02, 0x10, 0x02}
// #define OBD_LAST_DTC_THROTTLE_POSITION			{0x02, 0x11, 0x01}
// #define OBD_LAST_DTC_COMM_SECONDARY_AIR_STATUS	{0x02, 0x12, 0x01}
// #define OBD_LAST_DTC_OXYGEN_SENSORS_PRESENT_1	{0x02, 0x13, 0x01}
// #define OBD_LAST_DTC_B1S1_OXY_AND_S_FUEL_TRIM	{0x02, 0x14, 0x02}
// #define OBD_LAST_DTC_B1S2_OXY_AND_S_FUEL_TRIM	{0x02, 0x15, 0x02}
// #define OBD_LAST_DTC_B1S3_OXY_AND_S_FUEL_TRIM	{0x02, 0x16, 0x02}
// #define OBD_LAST_DTC_B1S4_OXY_AND_S_FUEL_TRIM	{0x02, 0x17, 0x02}
// #define OBD_LAST_DTC_B2S1_OXY_AND_S_FUEL_TRIM	{0x02, 0x18, 0x02}
// #define OBD_LAST_DTC_B2S2_OXY_AND_S_FUEL_TRIM	{0x02, 0x19, 0x02}
// #define OBD_LAST_DTC_B2S3_OXY_AND_S_FUEL_TRIM	{0x02, 0x1A, 0x02}
// #define OBD_LAST_DTC_B2S4_OXY_AND_S_FUEL_TRIM	{0x02, 0x1B, 0x02}
// #define OBD_LAST_DTC_STANDARD_CONFORM			{0x02, 0x1C, 0x01}
// #define OBD_LAST_DTC_OXYGEN_SENSORS_PRESENT_2	{0x02, 0x1D, 0x01}
// #define OBD_LAST_DTC_AUXILIARY_INPUT_STATUS		{0x02, 0x1E, 0x01}
// #define OBD_LAST_DTC_RUN_TIME_SINCE_ENG_START	{0x02, 0x1F, 0x02}
// #define OBD_LAST_DTC_SUPPORTED_21_40			{0x02, 0x20, 0x04}
// #define OBD_LAST_DTC_DISTANCE_TRAVELED_MIL_ON	{0x02, 0x21, 0x02}
// #define OBD_LAST_DTC_FUEL_RAIL_PRESSURE_REL		{0x02, 0x22, 0x02}
// #define OBD_LAST_DTC_FUEL_RAIL_PRESSURE_DIESEL	{0x02, 0x23, 0x02}
// #define OBD_LAST_DTC_O2S1_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x24, 0x04}
// #define OBD_LAST_DTC_O2S2_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x25, 0x04}
// #define OBD_LAST_DTC_O2S3_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x26, 0x04}
// #define OBD_LAST_DTC_O2S4_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x27, 0x04}
// #define OBD_LAST_DTC_O2S5_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x28, 0x04}
// #define OBD_LAST_DTC_O2S6_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x29, 0x04}
// #define OBD_LAST_DTC_O2S7_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x2A, 0x04}
// #define OBD_LAST_DTC_O2S8_WR_LAMBDA_EQ_RATIO_V	{0x02, 0x2B, 0x04}
// #define OBD_LAST_DTC_COMMANDED_EGR				{0x02, 0x2C, 0x01}
// #define OBD_LAST_DTC_EGR_ERROR					{0x02, 0x2D, 0x01}
// #define OBD_LAST_DTC_COMMANDED_EVAP_PURGE		{0x02, 0x2E, 0x01}	
// #define OBD_LAST_DTC_FUEL_LEVEL_INPUT			{0x02, 0x2F, 0x01}
// #define OBD_LAST_DTC_WARMUPS_SINCE_CODES_CLEAR	{0x02, 0x30, 0x01}
// #define OBD_LAST_DTC_DISTANCE_SINCE_CODES_CLEAR	{0x02, 0x31, 0x02}
// #define OBD_LAST_DTC_EVAP_SYS_VAPOR_PRESSURE	{0x02, 0x32, 0x02}
// #define OBD_LAST_DTC_BAROMETRIC_PRESSURE		{0x02, 0x33, 0x01}
// #define OBD_LAST_DTC_O2S1_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x34, 0x04}
// #define OBD_LAST_DTC_O2S2_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x35, 0x04}
// #define OBD_LAST_DTC_O2S3_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x36, 0x04}
// #define OBD_LAST_DTC_O2S4_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x37, 0x04}
// #define OBD_LAST_DTC_O2S5_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x38, 0x04}
// #define OBD_LAST_DTC_O2S6_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x39, 0x04}
// #define OBD_LAST_DTC_O2S7_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x3A, 0x04}
// #define OBD_LAST_DTC_O2S8_WR_LAMBDA_EQ_RATIO_I	{0x02, 0x3B, 0x04}
// #define OBD_LAST_DTC_CATALYST_TEMP_1_1			{0x02, 0x3C, 0x02}
// #define OBD_LAST_DTC_CATALYST_TEMP_2_1			{0x02, 0x3D, 0x02}
// #define OBD_LAST_DTC_CATALYST_TEMP_1_2			{0x02, 0x3E, 0x02}
// #define OBD_LAST_DTC_CATALYST_TEMP_2_2			{0x02, 0x3F, 0x02}
// #define OBD_LAST_DTC_SUPPORTED_41_60			{0x02, 0x40, 0x04}
// #define OBD_LAST_DTC_MON_STATUS_DRIVE_CYCLE		{0x02, 0x41, 0x04}
// #define OBD_LAST_DTC_CONTROL_MODULE_VOLTAGE		{0x02, 0x42, 0x02}
// #define OBD_LAST_DTC_ABSOLUTE_LOAD_VALUE		{0x02, 0x43, 0x02}
// #define OBD_LAST_DTC_COMMAND_EQUIV_RATIO		{0x02, 0x44, 0x02}
// #define OBD_LAST_DTC_RELATIVE_THROTTLE_POSITION	{0x02, 0x45, 0x01}
// #define OBD_LAST_DTC_AMBIENT_AIR_TEMPERATURE	{0x02, 0x46, 0x01}
// #define OBD_LAST_DTC_ABS_THROTTLE_POSITION_B	{0x02, 0x47, 0x01}
// #define OBD_LAST_DTC_ABS_THROTTLE_POSITION_C	{0x02, 0x48, 0x01}
// #define OBD_LAST_DTC_ABS_THROTTLE_POSITION_D	{0x02, 0x49, 0x01}
// #define OBD_LAST_DTC_ABS_THROTTLE_POSITION_E	{0x02, 0x4A, 0x01}
// #define OBD_LAST_DTC_ABS_THROTTLE_POSITION_F	{0x02, 0x4B, 0x01}
// #define OBD_LAST_DTC_COMMANDED_THROTTLE_ACTUATOR{0x02, 0x4C, 0x01}
// #define OBD_LAST_DTC_TIME_RUN_MIL_ON			{0x02, 0x4D, 0x02}
// #define OBD_LAST_DTC_TIME_TROUBLE_CODES_CLEARED	{0x02, 0x4E, 0x02}
// #define OBD_LAST_DTC_FUEL_TYPE					{0x02, 0x51, 0x01}
// #define OBD_LAST_DTC_ETHANOL_FUEL_PERCENTAGE	{0x02, 0x52, 0x01}
// #define OBD_LAST_DTC_ABS_EVAP_SYS_VAPOR_PRESSURE{0x02, 0x53, 0x02}


// MODE 3 (NULL lengths are variable, not standard or null, null PIDs not to be sent)

// #define OBD_REQUEST_TROUBLE_CODES		{0x03, NULL, NULL}


// MODE 4

// #define OBD_CLEAR_CODES_MIL_CHECK_LIGHT	{0x04, NULL, NULL}


// MODE 5


// MODE 9

// #define OBD_MODE_9_SUPPORTED_01_20		{0x09, 0x00, 0x04}
// #define OBD_VIN							{0x09, 0x02, NULL}
// #define OBD_CALIBRATION_ID				{0x09, 0x04, NULL}