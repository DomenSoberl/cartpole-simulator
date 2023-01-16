#include <Windows.h>
#include "timer.h"

Timer::Timer()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	systemFrequency = frequency.QuadPart;
	active = false;
	startTicks = 0;
	intervalTicks = 0;
}

Timer::~Timer()
{
}

void Timer::start()
{
	startTicks = querySystemTicks();
	active = true;
}

double Timer::stop()
{
	active = false;
	return static_cast<double>(querySystemTicks() - startTicks) / static_cast<double>(systemFrequency);
}

void Timer::reset()
{
	startTicks = querySystemTicks();
}

void Timer::setInterval(double seconds)
{
	interval = seconds;
	long long microseconds = static_cast<long long>(seconds * 1000000);
	intervalTicks = (microseconds * systemFrequency) / 1000000;
}

double Timer::timeFromDeadline()
{
	if (!active || intervalTicks == 0)
		return 0;

	long long elapsedTicks = querySystemTicks() - startTicks;
	return (double)elapsedTicks / systemFrequency;
}

bool Timer::deadline()
{
	if (!active || intervalTicks == 0)
		return false;

	long long elapsedTicks = querySystemTicks() - startTicks;
	if (elapsedTicks >= intervalTicks) {
		return true;
	}
	return false;
}

void Timer::nextInterval()
{
	startTicks += intervalTicks;
}

void Timer::catchUp()
{
	startTicks = querySystemTicks() - intervalTicks;
}

long long Timer::querySystemTicks()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}