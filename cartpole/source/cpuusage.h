#pragma once

class CPUUsage
{
public:
	static void setFrequency(int frequency);
	static void clear();
	static void reportUsage(double usage);
	static double getUsage();

protected:
	static int frequency;
	static double usage;
	static double sum;
	static int count;
};