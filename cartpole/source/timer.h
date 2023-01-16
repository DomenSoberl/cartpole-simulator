#pragma once

class Timer
{
public:
	Timer();
	~Timer();

	void start();
	double stop();
	void reset();
	bool isActive() { return active; }
	void setInterval(double seconds);
	double getInterval() const { return interval; }
	double timeFromDeadline();
	bool deadline();
	void nextInterval();
	void catchUp();

protected:
	bool active;
	long long startTicks;
	long long intervalTicks;
	double interval;

private:
	long long systemFrequency;

	long long querySystemTicks();
};