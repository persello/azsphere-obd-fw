#pragma once

/// <summary> Initializes the socket that allows to communicate with the M4 core running the SoftwareSerial reader. </summary>
/// <returns> 0 if successful, -1 on fail. </returns>
int initSSSocket(void);

/// <summary> Closes the socket that allows to communicate with the M4 core running the SoftwareSerial reader. </summary>
/// <returns> 0 if successful, -1 on fail. </returns>
int closeSSSocket(void);

/// <summary> Reads a string from the intercore buffer until empty or \n. </summary>
/// <param name="data"> Where to put received data. </param>
/// <returns> The number of bytes received or -1 on fail. </returns>
int readStringSS(char** data);