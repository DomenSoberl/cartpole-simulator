#pragma once
#include "drawingdevice.h"

class Cart
{
public:
	Cart();
	~Cart();

	double x;
	double y;
	double dx;
	double ddx;
	double dtheta;
	double ddtheta;
	double phi;
	double theta;
	bool frozen;

	void reset(
		double x,
		double dx,
		double ddx,
		double theta,
		double dtheta,
		double ddtheta
	);
	double getWidth();
	double getWheelDistance();
	void bounce();
	void dropMomentum();
	bool isTouched(double x, double y);
	void tick(double F, double dt);
	void paint(DrawingDevice* drawingDevice);

private:
	static const double pi;
	static const double negPi;
	static const double doublePi;
};