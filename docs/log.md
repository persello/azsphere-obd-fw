# SD logs

Logs are files in which all the data from connected sensors is stored. Each line of a log file contains three parts:

- Date and time,
- Name of the parameter,
- Measured value.
  
This is an example of a log item:

`2000-01-01 00:00:06.757    ENGINETEMP  54`

As you can see, the three parts are divided by tabulation characters (`\t`). The general structure of a log item is:

`YYYY-MM-DD HH:mm:ss.ttt    XXXXXXXXXX  VAL`

Every part except the value (`VAL`) has fixed length.
Date and time are divided by a single space character.

## Identifiers

This is a list of every identifier that this version of the firmware supports.

|  Identifier  |                Description                |  Unit  | Example value |
|:------------:|:-----------------------------------------:|:------:|:-------------:|
|  ASPHEREOBD  | Log header, specifies firmware version.   |   --   |    `V0.2.1`   |
|  CARECUINIT  | Car ECU initialization/deinitialization.  |  bool  |   `0` or `1`  |
|  BATVOLTAGE  | Car battery voltage.                      |  Volt  |    `12.46`    |
|  ENGINETEMP  | Engine coolant temperature (integer).     |   °C   |     `54`      |
|  VEHICLERPM  | Engine RPM count.                         |  RPM   |  `917.000000` |
|  VEHICSPEED  | Vehicle speed (wheel sensors, integer).   |  km/h  |     `41`      |
|  ENGAIRFLOW  | Intake manifold mass air flow rate.       |   g/s  |  `11.520000`  |
|  ENTHROTTLE  | Relative throttle position.               |   %    |   `3.529412`  |
|  RTCOLDTIME  | Last log item before time sync.           |   --   | always `???`  |
|  RTCNEWTIME  | First log item after sync, time source.   | source |     `GPS`     |
|  RTLATITUDE  | Real-time position: latitude.             |   °    |  `46.006125`  |
|  RTLONGITUD  | Real-time position: longitude.            |   °    |  `12.983617`  |
|  GPSSPEEDKM  | Vehicle speed (from GPS data).            |  km/h  |  `0.066672`   |
|  GPSVCOURSE  | Vehicle course (from GPS data).           |   °    | `nan`, `12.452312` |