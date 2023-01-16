#include <math.h>
#include "cart.h"
#include "engine.h"

const double Cart::pi = acos(-1);
const double Cart::negPi = -1 * acos(-1);
const double Cart::doublePi = 2 * acos(-1);

Cart::Cart() :
	x(0),
	y(0),
	dx(0),
	ddx(0),
	dtheta(0),
	ddtheta(0),
	phi(0),
	theta(0),
	frozen(false)
{
}

Cart::~Cart()
{
}

void Cart::reset(
	double x,
	double dx,
	double ddx,
	double theta,
	double dtheta,
	double ddtheta
)
{
	this->x = x;
	y = 0;
	phi = 0;
	this->theta = theta;
	frozen = false;
	this->dx = dx;
	this->ddx = ddx;
	this->dtheta = dtheta;
	this->ddtheta = ddtheta;
}

double Cart::getWidth()
{
	return Engine::simulatorParameters.cart.size;
}

double Cart::getWheelDistance()
{
	return 3 * Engine::simulatorParameters.cart.size / 5;
}

void Cart::bounce()
{
	dx = -dx * 0.5;
	ddx = 0;
}

void Cart::dropMomentum()
{
	dx = 0;
	ddx = 0;
	dtheta = 0;
	ddtheta = 0;
}

bool Cart::isTouched(double x, double y)
{
	double width = Engine::simulatorParameters.cart.size;
	double height = width / 6;
	double wheel = width / 10;
	double sinP = sin(phi);
	double cosP = cos(phi);
	
	/* Compute the central point. */
	double l = wheel + height / 2;
	double cx = this->x - l * sinP;
	double cy = this->y + l * cosP;

	/* Move / rotate the point relative to the body. */
	double vx = x - cx;
	double vy = y - cy;
	double rx = vx * cosP + vy * sinP;
	double ry = vy * cosP - vx * sinP;

	/* Check if within the body. */
	if (rx < - width / 2 || rx > width / 2) return false;
	if (ry < - height / 2 || ry > height / 2) return false;
	return true;
}

void Cart::tick(double F, double dt)
{
	if (frozen) return;

	double cartMass = Engine::simulatorParameters.cart.mass;
	double cartDamping = Engine::simulatorParameters.cart.damping;
	double poleMass = Engine::simulatorParameters.pole.mass;
	double poleLength = Engine::simulatorParameters.pole.size;
	double poleDamping = Engine::simulatorParameters.pole.damping;
	double g = Engine::simulatorParameters.gravity;
	
	/* Compute accelerations */
	double mass = cartMass + poleMass;
	double sinT = sin(theta);
	double cosT = cos(theta);
	double sinP = sin(phi);
	double ml = poleMass * poleLength;
	double dtheta2 = dtheta * dtheta;
	
	ddtheta = mass * g * sin(theta - phi) - cosT * (F + ml * dtheta2 * sinT - mass * g * sinP);
	ddtheta /= mass * poleLength - ml * cosT * cosT;
	ddtheta -= poleDamping * dtheta;

	ddx = F + ml * (dtheta2 * sinT - ddtheta * cosT);
	ddx /= mass;
	ddx -= g * sinP;
	ddx -= cartDamping * dx;

	/* Integrate time */
	dx += ddx * dt;
	dtheta += ddtheta * dt;
	double dist = dx * dt;
	theta += dtheta * dt;

	while (theta < negPi) theta += doublePi;
	while (theta >= pi) theta -= doublePi;

	/* Compute cart position */
	x += dist * cos(phi);
	y += dist * sin(phi);
}

void Cart::paint(DrawingDevice* drawingDevice)
{
	double width = Engine::simulatorParameters.cart.size;
	double halfWidth = width / 2;
	double height = width / 6;
	double wheel = width / 10;
	double poleLength = Engine::simulatorParameters.pole.size;
	double poleHalfWidth = width / 40;
	double wheelDistance = getWheelDistance();

	double sinh = sin(phi);
	double cosh = cos(phi);
	double sinv = sin(phi + pi / 2);
	double cosv = cos(phi + pi / 2);
	double sinph = sin(phi - theta);
	double cosph = cos(phi - theta);
	double sinpv = sin(phi - theta + pi / 2);
	double cospv = cos(phi - theta + pi / 2);

	double rx0 = wheel * cosv;           // Base of the body
	double ry0 = wheel * sinv;
	double rx1 = halfWidth * cosh;       // Front body point
	double ry1 = halfWidth * sinh;
	double rx2 = height * cosv;          // Top body point
	double ry2 = height * sinv;
	double rx3 = poleHalfWidth * cosph;  // Front pole point
	double ry3 = poleHalfWidth * sinph;
	double rx4 = poleLength * cospv;     // Top pole point
	double ry4 = poleLength * sinpv;

	/* Pole */
	DrawingDevice::Point pole[] = {
		DrawingDevice::Point(x + rx0 + rx2 - rx3, y + ry0 + ry2 - ry3),
		DrawingDevice::Point(x + rx0 + rx2 + rx3, y + ry0 + ry2 + ry3),
		DrawingDevice::Point(x + rx0 + rx2 + rx3 + rx4, y + ry0 + ry2 + ry3 + ry4),
		DrawingDevice::Point(x + rx0 + rx2 - rx3 + rx4, y + ry0 + ry2 - ry3 + ry4),
	};
	drawingDevice->polygon(pole, 4, drawingDevice->brushPole);

	DrawingDevice::Point hinge(x + rx0 + rx2, y + ry0 + ry2 - poleHalfWidth);
	drawingDevice->circle(hinge, 2.4 * poleHalfWidth, drawingDevice->brushCart);

	DrawingDevice::Point ball(x + rx0 + rx2 + rx4, y + ry0 + ry2 + ry4);
	drawingDevice->circle(ball, 3 * poleHalfWidth, drawingDevice->brushPoleBall);

	/* Body */
	DrawingDevice::Point body[] = {
		DrawingDevice::Point(x + rx0 - rx1, y + ry0 - ry1),
		DrawingDevice::Point(x + rx0 + rx1, y + ry0 + ry1),
		DrawingDevice::Point(x + rx0 + rx1 + rx2, y + ry0 + ry1 + ry2),
		DrawingDevice::Point(x + rx0 - rx1 + rx2, y + ry0 - ry1 + ry2)
	};
	drawingDevice->polygon(body, 4, drawingDevice->brushCart);
	
	/* Front wheel */
	rx1 = (wheelDistance / 2) * cosh; // Wheel distance
	ry1 = (wheelDistance / 2) * sinh;
	DrawingDevice::Point frontWheel(x + rx0 + rx1, y + ry0 + ry1);
	drawingDevice->circle(frontWheel, wheel, drawingDevice->brushTire);
	drawingDevice->circle(frontWheel, wheel / 2, drawingDevice->brushWheel);

	/* Rare wheel */
	DrawingDevice::Point rareWheel(x + rx0 - rx1, y + ry0 - ry1);
	drawingDevice->circle(rareWheel, wheel, drawingDevice->brushTire);
	drawingDevice->circle(rareWheel, wheel / 2, drawingDevice->brushWheel);
}