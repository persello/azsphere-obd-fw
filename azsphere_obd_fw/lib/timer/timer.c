#include <timer.h>

#include <time.h>

#include <sys/time.h>

unsigned long long millis() {
	struct timeval time;
	gettimeofday(&time, NULL);
	unsigned long long val =
		(unsigned long long)(time.tv_sec) * 1000 +
		(unsigned long long)(time.tv_usec) / 1000;
	return val;
}

long period[MAX_TIMERS] = {};
unsigned long long started[MAX_TIMERS] = {};
void Timer_On(int _ms, int _timer) {
	period[_timer] = _ms;
	started[_timer] = millis();
}


inline int Timer_Status(int _timer) {

	// Timer is disabled
	if (period == 0) {
		return 1;
	}
	else {

		// Timer has expired
		if (millis() > started[_timer] + period[_timer]) {
			return 0;
		}
		else {
			return 1;
		}
	}
}

inline void Timer_Off(int _timer) {
	period[_timer] = 0;
}