#include "cpuusage.h"

int CPUUsage::frequency = 0;
double CPUUsage::usage = 0;
double CPUUsage::sum = 0;
int CPUUsage::count = 0;

void CPUUsage::setFrequency(int frequency)
{
	CPUUsage::frequency = frequency;
}

void CPUUsage::clear()
{
	CPUUsage::usage = 0;
	CPUUsage::sum = 0;
	CPUUsage::count = 0;
}

void CPUUsage::reportUsage(double usage)
{
	CPUUsage::sum += usage;
	CPUUsage::count++;

	if (CPUUsage::count >= CPUUsage::frequency / 2) {
		CPUUsage::usage = CPUUsage::sum / (double)CPUUsage::count;
		CPUUsage::sum = 0;
		CPUUsage::count = 0;
	}
}

double CPUUsage::getUsage()
{
	return 100.0 * CPUUsage::usage;
}