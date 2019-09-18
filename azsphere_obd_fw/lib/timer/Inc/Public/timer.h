#pragma once

#define MAX_TIMERS 20

/// <summary> Get the milliseconds since midnight UTC. </summary>
/// <returns> The number of milliseconds since midnight UTC. </summary>
unsigned long long millis();

/// <summary> Get the nanoseconds since an undefined time point. </summary>
/// <returns> The number of nanoseconds. </summary>
unsigned long long nanos();

/// <summary> Starts a timer. </summary>
/// <param name="ms"> The interval of the timer. </param>
/// <param name="timer"> The number of the timer. </param>
void Timer_On(int _ms, int _timer);

/// <summary> Gets the status of the specified timer. </param>
/// <param name="timer"> The number of the timer. </param>
/// <returns> 1 if stopped or running, 0 if expired. </param>
inline int Timer_Status(int _timer);

/// <summary> Stops the specified timer. </summary>
/// <param name="timer"> The number of the timer. </param>
inline void Timer_Off(int _timer);